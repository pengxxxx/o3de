/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Memory/Memory.h>
#include <AzCore/Memory/AllocatorManager.h>
#include <AzCore/Memory/MemoryDrillerBus.h>

using namespace AZ;

#define RECORDING_ENABLED 0

#if RECORDING_ENABLED

struct AllocatorOperation
{
    enum OperationType : unsigned int
    {
        ALLOCATE,
        DEALLOCATE,
        REALLOCATE,
        RESIZE
    };
    OperationType m_operationType : 2;
    size_t m_size : 46;
    size_t m_alignment : 16;
    void* m_ptr;
    void* m_newptr; // required for resize
};
static constexpr size_t s_allocationOperationCount = 5 * 1024;
static AZStd::array<AllocatorOperation, s_allocationOperationCount> s_operations = {};
static uint64_t s_operationCounter = 0;
static AZStd::mutex s_operationsMutex;

AllocatorOperation& GetNextAllocatorOperation()
{
    AZStd::scoped_lock<AZStd::mutex> lock(s_operationsMutex);
    if (s_operationCounter == s_allocationOperationCount)
    {
        FILE* file = nullptr;
        fopen_s(&file, "memoryrecordings.bin", "ab");
        if (file)
        {
            fwrite(&s_operations, sizeof(AllocatorOperation), s_allocationOperationCount, file);
            fclose(file);
        }
        s_operationCounter = 0;
    }
    return s_operations[s_operationCounter++];
}

#endif

AllocatorBase::AllocatorBase(IAllocatorAllocate* allocationSource, const char* name, const char* desc) :
    IAllocator(allocationSource),
    m_name(name),
    m_desc(desc)
{
}

AllocatorBase::~AllocatorBase()
{
    AZ_Assert(!m_isReady, "Allocator %s (%s) is being destructed without first having gone through proper calls to PreDestroy() and Destroy(). Use AllocatorInstance<> for global allocators or AllocatorWrapper<> for local allocators.", m_name, m_desc);
}

const char* AllocatorBase::GetName() const
{
    return m_name;
}

const char* AllocatorBase::GetDescription() const
{
    return m_desc;
}

IAllocatorAllocate* AllocatorBase::GetSchema()
{
    return nullptr;
}

Debug::AllocationRecords* AllocatorBase::GetRecords()
{
    return m_records;
}

void AllocatorBase::SetRecords(Debug::AllocationRecords* records)
{
    m_records = records;
    m_memoryGuardSize = records ? records->MemoryGuardSize() : 0;
}

bool AllocatorBase::IsReady() const
{
    return m_isReady;
}

bool AllocatorBase::CanBeOverridden() const
{
    return m_canBeOverridden;
}

void AllocatorBase::PostCreate()
{
    if (m_registrationEnabled)
    {
        if (AZ::Environment::IsReady())
        {
            AllocatorManager::Instance().RegisterAllocator(this);
        }
        else
        {
            AllocatorManager::PreRegisterAllocator(this);
        }
    }

#if PLATFORM_MEMORY_INSTRUMENTATION_ENABLED
    m_platformMemoryInstrumentationGroupId = AZ::PlatformMemoryInstrumentation::GetNextGroupId();
    AZ::PlatformMemoryInstrumentation::RegisterGroup(m_platformMemoryInstrumentationGroupId, GetDescription(), AZ::PlatformMemoryInstrumentation::m_groupRoot);
#endif

    m_isReady = true;
}

void AllocatorBase::PreDestroy()
{
    if (m_registrationEnabled && AZ::AllocatorManager::IsReady())
    {
        AllocatorManager::Instance().UnRegisterAllocator(this);
    }

    m_isReady = false;
}

void AllocatorBase::SetLazilyCreated(bool lazy)
{
    m_isLazilyCreated = lazy;
}

bool AllocatorBase::IsLazilyCreated() const
{
    return m_isLazilyCreated;
}

void AllocatorBase::SetProfilingActive(bool active)
{
    m_isProfilingActive = active;
}

bool AllocatorBase::IsProfilingActive() const
{
    return m_isProfilingActive;
}

void AllocatorBase::DisableOverriding()
{
    m_canBeOverridden = false;
}

void AllocatorBase::DisableRegistration()
{
    m_registrationEnabled = false;
}

void AllocatorBase::ProfileAllocation(void* ptr, size_t byteSize, size_t alignment, const char* name, const char* fileName, int lineNum, int suppressStackRecord)
{
#if defined(AZ_HAS_VARIADIC_TEMPLATES) && defined(AZ_DEBUG_BUILD)
    ++suppressStackRecord; // one more for the fact the ebus is a function
#endif // AZ_HAS_VARIADIC_TEMPLATES

    if (m_isProfilingActive)
    {
#if PLATFORM_MEMORY_INSTRUMENTATION_ENABLED
        AZ::PlatformMemoryInstrumentation::Alloc(ptr, byteSize, 0, m_platformMemoryInstrumentationGroupId);
#else
        EBUS_EVENT(AZ::Debug::MemoryDrillerBus, RegisterAllocation, this, ptr, byteSize, alignment, name, fileName, lineNum, suppressStackRecord);
#endif
    }

#if RECORDING_ENABLED
    {
        AllocatorOperation& op = GetNextAllocatorOperation();
        op.m_operationType = AllocatorOperation::ALLOCATE;
        op.m_size = byteSize;
        op.m_alignment = alignment;
        op.m_ptr = ptr;
    }
#endif
}

void AllocatorBase::ProfileDeallocation(void* ptr, size_t byteSize, size_t alignment, Debug::AllocationInfo* info)
{
    if (m_isProfilingActive)
    {
#if PLATFORM_MEMORY_INSTRUMENTATION_ENABLED
        AZ::PlatformMemoryInstrumentation::Free(ptr);
#else
        EBUS_EVENT(AZ::Debug::MemoryDrillerBus, UnregisterAllocation, this, ptr, byteSize, alignment, info);
#endif
    }
#if RECORDING_ENABLED
    {
        AllocatorOperation& op = GetNextAllocatorOperation();
        op.m_operationType = AllocatorOperation::DEALLOCATE;
        op.m_size = byteSize;
        op.m_alignment = alignment;
        op.m_ptr = ptr;
    }
#endif
}

void AllocatorBase::ProfileReallocationBegin(void* ptr, size_t newSize)
{
    if (m_isProfilingActive)
    {
#if PLATFORM_MEMORY_INSTRUMENTATION_ENABLED
        AZ::PlatformMemoryInstrumentation::ReallocBegin(ptr, newSize, m_platformMemoryInstrumentationGroupId);
#else
        // Driller API intensionally not called, only End is required.
        AZ_UNUSED(ptr);
        AZ_UNUSED(newSize);
#endif
    }
}

void AllocatorBase::ProfileReallocationEnd(void* ptr, void* newPtr, size_t newSize, size_t newAlignment)
{
    if (m_isProfilingActive)
    {
#if PLATFORM_MEMORY_INSTRUMENTATION_ENABLED
        AZ::PlatformMemoryInstrumentation::ReallocEnd(newPtr, newSize, 0);
#else
        EBUS_EVENT(AZ::Debug::MemoryDrillerBus, ReallocateAllocation, this, ptr, newPtr, newSize, newAlignment);
#endif
    }
#if RECORDING_ENABLED
    {
        AllocatorOperation& op = GetNextAllocatorOperation();
        op.m_operationType = AllocatorOperation::REALLOCATE;
        op.m_size = newSize;
        op.m_alignment = newAlignment;
        op.m_ptr = ptr;
        op.m_newptr = newPtr;
    }
#endif
}

void AllocatorBase::ProfileReallocation(void* ptr, void* newPtr, size_t newSize, size_t newAlignment)
{
    ProfileReallocationEnd(ptr, newPtr, newSize, newAlignment);
}

void AllocatorBase::ProfileResize(void* ptr, size_t newSize)
{
    if (newSize && m_isProfilingActive)
    {
        EBUS_EVENT(AZ::Debug::MemoryDrillerBus, ResizeAllocation, this, ptr, newSize);
    }
#if RECORDING_ENABLED
    {
        AllocatorOperation& op = GetNextAllocatorOperation();
        op.m_operationType = AllocatorOperation::RESIZE;
        op.m_size = newSize;
        op.m_alignment = 0;
        op.m_ptr = ptr;
    }
#endif
}

bool AllocatorBase::OnOutOfMemory(size_t byteSize, size_t alignment, int flags, const char* name, const char* fileName, int lineNum)
{
    if (AllocatorManager::IsReady() && AllocatorManager::Instance().m_outOfMemoryListener)
    {
        AllocatorManager::Instance().m_outOfMemoryListener(this, byteSize, alignment, flags, name, fileName, lineNum);
        return true;
    }
    return false;
}
