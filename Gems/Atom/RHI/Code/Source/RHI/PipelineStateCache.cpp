/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RHI/PipelineStateCache.h>
#include <Atom/RHI/Factory.h>

#include <AzCore/Debug/Profiler.h>
#include <AzCore/std/sort.h>
#include <AzCore/std/parallel/exponential_backoff.h>

namespace AZ::RHI
{
    Ptr<PipelineStateCache> PipelineStateCache::Create(MultiDevice::DeviceMask deviceMask)
    {
        return aznew PipelineStateCache(deviceMask);
    }

    PipelineStateCache::PipelineStateCache(MultiDevice::DeviceMask deviceMask)
        : m_deviceMask{ deviceMask }
    {
    }

    void PipelineStateCache::ValidateCacheIntegrity() const
    {
#if defined(AZ_ENABLE_TRACING)
        for (size_t i = 0; i < m_globalLibrarySet.size(); ++i)
        {
            const GlobalLibraryEntry& globalLibraryEntry = m_globalLibrarySet[i];
            const PipelineStateSet& readOnlyCache = globalLibraryEntry.m_readOnlyCache;
            AZ_Assert(globalLibraryEntry.m_pendingCompileCount == 0, "Compiles are pending for pipeline library");
            AZ_Assert(globalLibraryEntry.m_pendingCache.empty(), "Pending cache is not empty.");

            if (!m_globalLibraryActiveBits[i])
            {
                AZ_Assert(readOnlyCache.empty(), "Inactive library has pipeline states in its global entry.");
            }

#if defined(AZ_DEBUG_BUILD)
            // the PipelineStateSet is expensive to duplicate, only do this in debug.
            PipelineStateSet readOnlyCacheCopy = readOnlyCache;
            AZ_Assert(AZStd::unique(readOnlyCacheCopy.begin(), readOnlyCacheCopy.end()) == readOnlyCacheCopy.end(),
                "'%d' Duplicates existed in the read-only cache!", readOnlyCache.size() - readOnlyCacheCopy.size());
#endif
        }

        m_threadLibrarySet.ForEach([this](const ThreadLibrarySet& threadLibrarySet)
        {
            const size_t libraryCount = m_globalLibrarySet.size();

            for (size_t i = 0; i < libraryCount; ++i)
            {
                const ThreadLibraryEntry& threadLibraryEntry = threadLibrarySet[i];

                if (!m_globalLibraryActiveBits[i])
                {
                    AZ_Assert(!threadLibraryEntry.m_library, "Inactive library has a valid RHI::MultiDevicePipelineLibrary instance.");
                }

                AZ_Assert(threadLibraryEntry.m_threadLocalCache.empty(), "Thread library should not have any items in its local cache.");
            }
        });
#endif
    }

    void PipelineStateCache::Reset()
    {
        AZStd::unique_lock<AZStd::shared_mutex> lock(m_mutex);

        for (size_t i = 0; i < m_globalLibrarySet.size(); ++i)
        {
            if (m_globalLibraryActiveBits[i])
            {
                ResetLibraryImpl(MultiDevicePipelineLibraryHandle(i));
            }
        }
    }

    MultiDevicePipelineLibraryHandle PipelineStateCache::CreateLibrary(
        const AZStd::unordered_map<int, ConstPtr<RHI::PipelineLibraryData>>& serializedData, const AZStd::unordered_map<int, AZStd::string>& filePaths)
    {
        AZStd::unique_lock<AZStd::shared_mutex> lock(m_mutex);

        MultiDevicePipelineLibraryHandle handle;
        if (!m_libraryFreeList.empty())
        {
            handle = m_libraryFreeList.back();
            m_libraryFreeList.pop_back();
        }
        else
        {
            if (m_globalLibrarySet.size() == LibraryCountMax)
            {
                AZ_Error(
                    "PipelineStateCache",
                    false,
                    "Exceeded maximum number of allowed pipeline libraries in "
                    "cache. You must update LibraryCountMax to add more.");
                return {};
            }

            handle = MultiDevicePipelineLibraryHandle(m_globalLibrarySet.size());
            m_globalLibrarySet.emplace_back();
        }

        AZ_Assert(m_globalLibraryActiveBits[handle.GetIndex()] == false, "Attempted to allocate active library entry!");
        m_globalLibraryActiveBits[handle.GetIndex()] = true;

        GlobalLibraryEntry& libraryEntry = m_globalLibrarySet[handle.GetIndex()];
        libraryEntry.m_pipelineLibraryDescriptor.Init(m_deviceMask, serializedData, filePaths);
        AZ_Assert(libraryEntry.m_readOnlyCache.empty() && libraryEntry.m_pendingCache.empty(), "Library entry has entries in its caches!");

        return handle;
    }

    void PipelineStateCache::ReleaseLibrary(MultiDevicePipelineLibraryHandle handle)
    {
        if (handle.IsValid())
        {
            AZStd::unique_lock<AZStd::shared_mutex> lock(m_mutex);
            AZ_Assert(m_globalLibraryActiveBits[handle.GetIndex()], "Releasing a library that is no longer valid.");

            ResetLibraryImpl(handle);

            GlobalLibraryEntry& libraryEntry = m_globalLibrarySet[handle.GetIndex()];
            libraryEntry.m_readOnlyCache.clear();
            libraryEntry.m_pipelineLibraryDescriptor.Init(m_deviceMask, {}, {});

            m_globalLibraryActiveBits[handle.GetIndex()] = false;
            m_libraryFreeList.push_back(handle);
        }
    }

    void PipelineStateCache::ResetLibrary(MultiDevicePipelineLibraryHandle handle)
    {
        if (handle.IsValid())
        {
            AZStd::unique_lock<AZStd::shared_mutex> lock(m_mutex);
            ResetLibraryImpl(handle);
        }
    }

    void PipelineStateCache::ResetLibraryImpl(MultiDevicePipelineLibraryHandle handle)
    {
        m_threadLibrarySet.ForEach(
            [handle](ThreadLibrarySet& librarySet)
            {
                ThreadLibraryEntry& libraryEntry = librarySet[handle.GetIndex()];
                libraryEntry.m_library = nullptr;
                libraryEntry.m_threadLocalCache.clear();
            });

        GlobalLibraryEntry& libraryEntry = m_globalLibrarySet[handle.GetIndex()];

        AZ_Assert(libraryEntry.m_pendingCompileCount == 0, "Reseting library while compiles are still pending!");
        libraryEntry.m_readOnlyCache.clear();
        libraryEntry.m_pendingCacheMutex.lock();
        libraryEntry.m_pendingCache.clear();
        libraryEntry.m_pendingCacheMutex.unlock();
    }

    Ptr<MultiDevicePipelineLibrary> PipelineStateCache::GetMergedLibrary(MultiDevicePipelineLibraryHandle handle) const
    {
        if (handle.IsNull())
        {
            return nullptr;
        }

        AZStd::unique_lock<AZStd::shared_mutex> lock(m_mutex);
        const GlobalLibraryEntry& entry = m_globalLibrarySet[handle.GetIndex()];

        //! Each thread has its own MultiDevicePipelineLibrary instance. To produce the final serialized data, we
        //! coalesce data from each individual library by merging the thread-local ones into a single
        //! global (temporary) library. The data is then extracted from this global library and returned.
        //! This operation is designed to happen once at application shutdown; certainly not every frame.
        AZStd::vector<const MultiDevicePipelineLibrary*> threadLibraries;
        m_threadLibrarySet.ForEach(
            [handle, &threadLibraries](const ThreadLibrarySet& threadLibrarySet)
            {
                const ThreadLibraryEntry& threadLibraryEntry = threadLibrarySet[handle.GetIndex()];

                // Skip libraries that failed to initialize.
                if (threadLibraryEntry.m_library && threadLibraryEntry.m_library->IsInitialized())
                {
                    threadLibraries.push_back(threadLibraryEntry.m_library.get());
                }
            });

        bool doesPSODataExist{ false };

        for (auto& [deviceIndex, devicePipelineLibraryDescriptor] : entry.m_pipelineLibraryDescriptor.m_devicePipelineLibraryDescriptors)
        {
            doesPSODataExist |= devicePipelineLibraryDescriptor.m_serializedData.get() != nullptr;
        }

        for (const RHI::MultiDevicePipelineLibrary* libraryBase : threadLibraries)
        {
            const MultiDevicePipelineLibrary* library = static_cast<const MultiDevicePipelineLibrary*>(libraryBase);
            doesPSODataExist |= library->IsMergeRequired();
        }

        if (doesPSODataExist)
        {
            Ptr<MultiDevicePipelineLibrary> pipelineLibrary = aznew MultiDevicePipelineLibrary;
            ResultCode resultCode = pipelineLibrary->Init(m_deviceMask, entry.m_pipelineLibraryDescriptor);

            if (resultCode == ResultCode::Success)
            {
                resultCode = pipelineLibrary->MergeInto(threadLibraries);

                if (resultCode == ResultCode::Success)
                {
                    return pipelineLibrary;
                }
            }
        }

        return nullptr;
    }

    void PipelineStateCache::Compact()
    {
        AZ_PROFILE_SCOPE(RHI, "PipelineStateCache: Compact");
        AZStd::unique_lock<AZStd::shared_mutex> lock(m_mutex);

        // Merge the pending cache into the read-only cache.
        bool hasCompiledPipelineStates = false;
        for (size_t i = 0; i < m_globalLibrarySet.size(); ++i)
        {
            GlobalLibraryEntry& globalLibraryEntry = m_globalLibrarySet[i];

            // Skip inactive libraries and ones that didn't compile anything this cycle.
            if (m_globalLibraryActiveBits[i] && !globalLibraryEntry.m_pendingCache.empty())
            {
                hasCompiledPipelineStates = true;

                // Allocate a temporary staging set, perform the merge, and then move it back into the read-only cache.
                PipelineStateSet mergeResult;
                mergeResult.reserve(globalLibraryEntry.m_readOnlyCache.size() + globalLibraryEntry.m_pendingCache.size());

                AZStd::merge(
                    globalLibraryEntry.m_readOnlyCache.begin(), globalLibraryEntry.m_readOnlyCache.end(),
                    globalLibraryEntry.m_pendingCache.begin(), globalLibraryEntry.m_pendingCache.end(),
                    AZStd::inserter(mergeResult, mergeResult.begin()));

                globalLibraryEntry.m_readOnlyCache.swap(mergeResult);
                globalLibraryEntry.m_pendingCache.clear();
            }
        }

        // If we had compilation events, then the thread-local caches are not empty and need to be cleared.
        if (hasCompiledPipelineStates)
        {
            const size_t libraryCount = m_globalLibrarySet.size();

            m_threadLibrarySet.ForEach([this, libraryCount](ThreadLibrarySet& threadLibrarySet)
            {
                for (size_t i = 0; i < libraryCount; ++i)
                {
                    if (m_globalLibraryActiveBits[i])
                    {
                        threadLibrarySet[i].m_threadLocalCache.clear();
                    }
                }
            });
        }

        ValidateCacheIntegrity();
    }

    const MultiDevicePipelineState* PipelineStateCache::FindPipelineState(
        const PipelineStateSet& pipelineStateSet, const PipelineStateDescriptor& descriptor)
    {
        auto pipelineStateIt = pipelineStateSet.find(PipelineStateEntry(descriptor.GetHash(), nullptr, descriptor));
        if (pipelineStateIt != pipelineStateSet.end())
        {
            return pipelineStateIt->m_pipelineState.get();
        }
        return nullptr;
    }

    bool PipelineStateCache::InsertPipelineState(PipelineStateSet& pipelineStateSet, PipelineStateEntry pipelineStateEntry)
    {
        auto ret = pipelineStateSet.insert(pipelineStateEntry);
        return ret.second;
    }

    const MultiDevicePipelineState* PipelineStateCache::AcquirePipelineState(
        MultiDevicePipelineLibraryHandle handle, const PipelineStateDescriptor& descriptor, const AZ::Name& name /*= AZ::Name()*/)
    {
        if (handle.IsNull())
        {
            return nullptr;
        }

        AZStd::shared_lock<AZStd::shared_mutex> lock(m_mutex);

        GlobalLibraryEntry& globalLibraryEntry = m_globalLibrarySet[handle.GetIndex()];
        PipelineStateHash pipelineStateHash = descriptor.GetHash();

        // Search the read-only cache first.
        if (const MultiDevicePipelineState* pipelineState = FindPipelineState(globalLibraryEntry.m_readOnlyCache, descriptor))
        {
            return pipelineState;
        }

        // Search the thread-local cache next.
        {
            ThreadLibrarySet& threadLibrarySet = m_threadLibrarySet.GetStorage();
            ThreadLibraryEntry& threadLibraryEntry = threadLibrarySet[handle.GetIndex()];
            PipelineStateSet& threadLocalCache = threadLibraryEntry.m_threadLocalCache;

            if (const MultiDevicePipelineState* pipelineState = FindPipelineState(threadLocalCache, descriptor))
            {
                return pipelineState;
            }

            // No entry in the thread-local set. Request a pipeline state from the pending cache and add
            // it to the thread-local cache to reduce contention on the pending cache.
            {
                // Lazy-init the library on first access.
                if (!threadLibraryEntry.m_library)
                {
                    Ptr<MultiDevicePipelineLibrary> pipelineLibrary = aznew MultiDevicePipelineLibrary;
                    RHI::ResultCode resultCode = pipelineLibrary->Init(m_deviceMask, globalLibraryEntry.m_pipelineLibraryDescriptor);
                    if (resultCode != RHI::ResultCode::Success)
                    {
                        AZ_Warning(
                            "PipelineStateCache",
                            false,
                            "Failed to initialize pipeline library. MultiDevicePipelineLibrary usage is disabled.");
                    }

                    // We store a valid pointer even if initialization failed, to avoid attempting
                    // to re-create it with every access.
                    threadLibraryEntry.m_library = AZStd::move(pipelineLibrary);
                }

                ConstPtr<MultiDevicePipelineState> pipelineState =
                    CompilePipelineState(globalLibraryEntry, threadLibraryEntry, descriptor, pipelineStateHash, name);

                [[maybe_unused]] bool success =
                    InsertPipelineState(threadLocalCache, PipelineStateEntry(pipelineStateHash, pipelineState, descriptor));
                AZ_Assert(success, "PipelineStateEntry already exists in the thread cache.");

                return pipelineState.get();
            }
        }
    }

    ConstPtr<MultiDevicePipelineState> PipelineStateCache::CompilePipelineState(
        GlobalLibraryEntry& globalLibraryEntry,
        ThreadLibraryEntry& threadLibraryEntry,
        const PipelineStateDescriptor& descriptor,
        PipelineStateHash pipelineStateHash,
        const AZ::Name& name)
    {
        Ptr<MultiDevicePipelineState> pipelineState;

        PipelineStateSet& pendingCache = globalLibraryEntry.m_pendingCache;

        {
            AZStd::lock_guard<AZStd::mutex> lock(globalLibraryEntry.m_pendingCacheMutex);

            // Another thread may have started compiling this pipeline state. Check the pending cache.
            if (const MultiDevicePipelineState* pipeline = FindPipelineState(pendingCache, descriptor))
            {
                return pipeline;
            }

            // We need to create and insert the pipeline state into the locked cache. Create the pipeline state
            // but don't initialize it yet. We can safely allocate the 'empty' instance and cache it.
            pipelineState = aznew MultiDevicePipelineState;

            [[maybe_unused]] bool success =
                InsertPipelineState(pendingCache, PipelineStateEntry(pipelineStateHash, pipelineState, descriptor));
            AZ_Assert(success, "PipelineStateEntry already exists in the pending cache.");
        }

        [[maybe_unused]] ResultCode resultCode = ResultCode::InvalidArgument;

        // Increment the pending compile count on the global entry, which tracks how many pipeline states
        // are currently being compiled across all threads.
        if (Validation::IsEnabled())
        {
            ++globalLibraryEntry.m_pendingCompileCount;
        }

        // If the pipeline library failed to initialize, then we don't use it.
        MultiDevicePipelineLibrary* pipelineLibrary = threadLibraryEntry.m_library.get();
        if (!pipelineLibrary->IsInitialized())
        {
            pipelineLibrary = nullptr;
        }

        // We no longer have the lock, but we own compilation of the pipeline state. Use the
        // thread-local library to perform compilation without blocking other threads.
        resultCode = pipelineState->Init(m_deviceMask, descriptor, pipelineLibrary);

        pipelineState->SetName(name);

        if (Validation::IsEnabled())
        {
            --globalLibraryEntry.m_pendingCompileCount;
        }

        // NOTE: We can't return null on a failure, since other threads will return the entry without compiling
        // it. Instead, the pipeline state remains uninitialized.

        AZ_Error(
            "PipelineStateCache",
            resultCode == ResultCode::Success,
            "Failed to compile pipeline state. It will remain in an initialized state.");
        return AZStd::move(pipelineState);
    }

    PipelineStateCache::PipelineStateEntry::PipelineStateEntry(
        PipelineStateHash hash, ConstPtr<MultiDevicePipelineState> pipelineState, const PipelineStateDescriptor& descriptor)
        : m_hash{ hash }
        , m_pipelineState{ AZStd::move(pipelineState) }
    {
        switch (descriptor.GetType())
        {
        case PipelineStateType::Dispatch:
            m_pipelineStateDescriptorVariant = static_cast<const AZ::RHI::PipelineStateDescriptorForDispatch&>(descriptor);
            break;

        case PipelineStateType::Draw:
            m_pipelineStateDescriptorVariant = static_cast<const AZ::RHI::PipelineStateDescriptorForDraw&>(descriptor);
            break;

        case PipelineStateType::RayTracing:
            m_pipelineStateDescriptorVariant = static_cast<const AZ::RHI::PipelineStateDescriptorForRayTracing&>(descriptor);
            break;
        }
    }

    bool PipelineStateCache::PipelineStateEntry::operator == (const PipelineStateCache::PipelineStateEntry& rhs) const
    {
        if(AZStd::get_if<AZ::RHI::PipelineStateDescriptorForDispatch>(&rhs.m_pipelineStateDescriptorVariant) &&
            AZStd::get_if<AZ::RHI::PipelineStateDescriptorForDispatch>(&m_pipelineStateDescriptorVariant))
        {
            const AZ::RHI::PipelineStateDescriptorForDispatch& lhsDesc = AZStd::get<PipelineStateDescriptorForDispatch>(m_pipelineStateDescriptorVariant);
            const AZ::RHI::PipelineStateDescriptorForDispatch& rhsDesc = AZStd::get<PipelineStateDescriptorForDispatch>(rhs.m_pipelineStateDescriptorVariant);

            return lhsDesc == rhsDesc;
        }
        else if(AZStd::get_if<AZ::RHI::PipelineStateDescriptorForDraw>(&rhs.m_pipelineStateDescriptorVariant) &&
            AZStd::get_if<AZ::RHI::PipelineStateDescriptorForDraw>(&m_pipelineStateDescriptorVariant))
        {
            const AZ::RHI::PipelineStateDescriptorForDraw& lhsDesc = AZStd::get<PipelineStateDescriptorForDraw>(m_pipelineStateDescriptorVariant);
            const AZ::RHI::PipelineStateDescriptorForDraw& rhsDesc = AZStd::get<PipelineStateDescriptorForDraw>(rhs.m_pipelineStateDescriptorVariant);

            return lhsDesc == rhsDesc;
        }
        else if(AZStd::get_if<AZ::RHI::PipelineStateDescriptorForRayTracing>(&rhs.m_pipelineStateDescriptorVariant) &&
            AZStd::get_if<AZ::RHI::PipelineStateDescriptorForRayTracing>(&m_pipelineStateDescriptorVariant))
        {
            const AZ::RHI::PipelineStateDescriptorForRayTracing& lhsDesc = AZStd::get<PipelineStateDescriptorForRayTracing>(m_pipelineStateDescriptorVariant);
            const AZ::RHI::PipelineStateDescriptorForRayTracing& rhsDesc = AZStd::get<PipelineStateDescriptorForRayTracing>(rhs.m_pipelineStateDescriptorVariant);

            return lhsDesc == rhsDesc;
        }

        return false;
    }
}
