/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

// include the required headers
#include "MultiThreadScheduler.h"
#include "ActorManager.h"
#include "ActorInstance.h"
#include "AnimGraphInstance.h"
#include "Attachment.h"
#include "EMotionFXManager.h"
#include <EMotionFX/Source/Allocators.h>
#include <MCore/Source/LogManager.h>

#include <AzCore/Jobs/JobFunction.h>
#include <AzCore/Jobs/JobCompletion.h>
#include <AzCore/Jobs/JobManagerBus.h>
#include <AzCore/Jobs/JobContext.h>


namespace EMotionFX
{
    AZ_CLASS_ALLOCATOR_IMPL(MultiThreadScheduler, ActorUpdateAllocator, 0)

    // constructor
    MultiThreadScheduler::MultiThreadScheduler()
        : ActorUpdateScheduler()
    {
        mSteps.SetMemoryCategory(EMFX_MEMCATEGORY_UPDATESCHEDULERS);
        mCleanTimer     = 0.0f; // time passed since last schedule cleanup, in seconds
        mSteps.Reserve(1000);
    }


    // destructor
    MultiThreadScheduler::~MultiThreadScheduler()
    {
    }


    // create
    MultiThreadScheduler* MultiThreadScheduler::Create()
    {
        return aznew MultiThreadScheduler();
    }


    // clear the schedule
    void MultiThreadScheduler::Clear()
    {
        Lock();
        mSteps.Clear();
        Unlock();
    }


    // add the actor instance dependencies to the schedule step
    void MultiThreadScheduler::AddDependenciesToStep(ActorInstance* instance, ScheduleStep* outStep)
    {
        MCORE_UNUSED(outStep);
        MCORE_UNUSED(instance);
    }


    // check if there is a matching dependency for a given actor
    bool MultiThreadScheduler::CheckIfHasMatchingDependency(ActorInstance* instance, ScheduleStep* step) const
    {
        MCORE_UNUSED(instance);
        MCORE_UNUSED(step);
        return false;
    }


    // log it, for debugging purposes
    void MultiThreadScheduler::Print()
    {
        // for all steps
        const uint32 numSteps = mSteps.GetLength();
        for (uint32 i = 0; i < numSteps; ++i)
        {
            AZ_Printf("EMotionFX", "STEP %.3d - %d", i, mSteps[i].mActorInstances.size());
        }

        AZ_Printf("EMotionFX", "---------");
    }


    void MultiThreadScheduler::RemoveEmptySteps()
    {
        // process all steps
        for (uint32 s = 0; s < mSteps.GetLength(); )
        {
            // if the step isn't empty
            if (mSteps[s].mActorInstances.size() > 0)
            {
                s++;
            }
            else // otherwise remove it
            {
                mSteps.Remove(s);
            }
        }
    }


    // execute the schedule
    void MultiThreadScheduler::Execute(float timePassedInSeconds)
    {
        MCore::LockGuardRecursive guard(mMutex);

        uint32 numSteps = mSteps.GetLength();
        if (numSteps == 0)
        {
            return;
        }

        // check if we need to cleanup the schedule
        mCleanTimer += timePassedInSeconds;
        if (mCleanTimer >= 1.0f)
        {
            mCleanTimer = 0.0f;
            RemoveEmptySteps();
            numSteps = mSteps.GetLength();
        }

        //-----------------------------------------------------------

        // propagate root actor instance visibility to their attachments
        const ActorManager& actorManager = GetActorManager();
        const uint32 numRootActorInstances = actorManager.GetNumRootActorInstances();
        for (uint32 i = 0; i < numRootActorInstances; ++i)
        {
            ActorInstance* rootInstance = actorManager.GetRootActorInstance(i);
            if (rootInstance->GetIsEnabled() == false)
            {
                continue;
            }

            rootInstance->RecursiveSetIsVisible(rootInstance->GetIsVisible());
        }

        // reset stats
        mNumUpdated.SetValue(0);
        mNumVisible.SetValue(0);
        mNumSampled.SetValue(0);

        for (uint32 s = 0; s < numSteps; ++s)
        {
            const ScheduleStep& currentStep = mSteps[s];

            // skip empty steps
            const size_t numStepEntries = currentStep.mActorInstances.size();
            if (numStepEntries == 0)
            {
                continue;
            }

            // process the actor instances in the current step in parallel
            AZ::JobCompletion jobCompletion;           
            for (uint32 c = 0; c < numStepEntries; ++c)
            {
                ActorInstance* actorInstance = currentStep.mActorInstances[c];
                if (actorInstance->GetIsEnabled() == false)
                {
                    continue;
                }

                AZ::JobContext* jobContext = nullptr;
                AZ::Job* job = AZ::CreateJobFunction([this, timePassedInSeconds, actorInstance]()
                {
                    AZ_PROFILE_SCOPE(AZ::Debug::ProfileCategory::Animation, "MultiThreadScheduler::Execute::ActorInstanceUpdateJob");

                    const AZ::u32 threadIndex = AZ::JobContext::GetGlobalContext()->GetJobManager().GetWorkerThreadId();                    
                    actorInstance->SetThreadIndex(threadIndex);

                    const bool isVisible = actorInstance->GetIsVisible();
                    if (isVisible)
                    {
                        mNumVisible.Increment();
                    }

                    // check if we want to sample motions
                    bool sampleMotions = false;
                    actorInstance->SetMotionSamplingTimer(actorInstance->GetMotionSamplingTimer() + timePassedInSeconds);
                    if (actorInstance->GetMotionSamplingTimer() >= actorInstance->GetMotionSamplingRate())
                    {
                        sampleMotions = true;
                        actorInstance->SetMotionSamplingTimer(0.0f);

                        if (isVisible)
                        {
                            mNumSampled.Increment();
                        }
                    }

                    // update the actor instance
                    actorInstance->UpdateTransformations(timePassedInSeconds, isVisible, sampleMotions);
                }, true, jobContext);

                job->SetDependent(&jobCompletion);               
                job->Start();

                mNumUpdated.Increment();
            }

            jobCompletion.StartAndWaitForCompletion();
        } // for all steps
    }


    // find the next free spot in the schedule
    bool MultiThreadScheduler::FindNextFreeItem(ActorInstance* actorInstance, uint32 startStep, uint32* outStepNr)
    {
        // try out all steps
        const uint32 numSteps = mSteps.GetLength();
        for (uint32 s = startStep; s < numSteps; ++s)
        {
            // if there is a conflicting dependency, skip this step
            if (CheckIfHasMatchingDependency(actorInstance, &mSteps[s]))
            {
                continue;
            }

            // we found a free spot!
            *outStepNr = s;
            return true;
        }

        *outStepNr = numSteps;
        return false;
    }

    bool MultiThreadScheduler::HasActorInstanceInSteps(const ActorInstance* actorInstance) const
    {
        const uint32 numSteps = mSteps.GetLength();
        for (uint32 s = 0; s < numSteps; ++s)
        {
            const ScheduleStep& step = mSteps[s];
            if (AZStd::find(step.mActorInstances.begin(), step.mActorInstances.end(), actorInstance) != step.mActorInstances.end())
            {
                return true;
            }
        }

        return false;
    }

    void MultiThreadScheduler::RecursiveInsertActorInstance(ActorInstance* instance, uint32 startStep)
    {
        MCore::LockGuardRecursive guard(mMutex);
        AZ_Assert(!HasActorInstanceInSteps(instance), "Expected the actor instance not being part of another step already.");

        // find the first free location that doesn't conflict
        uint32 outStep = startStep;
        if (!FindNextFreeItem(instance, startStep, &outStep))
        {
            mSteps.Reserve(10);
            mSteps.AddEmpty();
            outStep = mSteps.GetLength() - 1;
        }

        // pre-allocate step size
        if (mSteps[outStep].mActorInstances.size() % 10 == 0)
        {
            mSteps[outStep].mActorInstances.reserve(mSteps[outStep].mActorInstances.size() + 10);
        }

        if (mSteps[outStep].mDependencies.GetLength() % 5 == 0)
        {
            mSteps[outStep].mDependencies.Reserve(mSteps[outStep].mDependencies.GetLength() + 5);
        }

        // add the actor instance and its dependencies
        mSteps[ outStep ].mActorInstances.reserve(GetEMotionFX().GetNumThreads());
        mSteps[ outStep ].mActorInstances.emplace_back(instance);
        AddDependenciesToStep(instance, &mSteps[outStep]);

        // recursively add all attachments too
        const uint32 numAttachments = instance->GetNumAttachments();
        for (uint32 i = 0; i < numAttachments; ++i)
        {
            ActorInstance* attachment = instance->GetAttachment(i)->GetAttachmentActorInstance();
            if (attachment)
            {
                RecursiveInsertActorInstance(attachment, outStep + 1);
            }
        }
    }


    // remove the actor instance from the schedule (excluding attachments)
    uint32 MultiThreadScheduler::RemoveActorInstance(ActorInstance* actorInstance, uint32 startStep)
    {
        MCore::LockGuardRecursive guard(mMutex);

        // for all scheduler steps, starting from the specified start step number
        const uint32 numSteps = mSteps.GetLength();
        for (uint32 s = startStep; s < numSteps; ++s)
        {
            ScheduleStep& step = mSteps[s];

            // Remove all occurrences of the actor instance.
            const size_t numActorInstancesPreRemove = step.mActorInstances.size();
            step.mActorInstances.erase(AZStd::remove(step.mActorInstances.begin(), step.mActorInstances.end(), actorInstance), step.mActorInstances.end());

            // try to see if there is anything to remove in this step
            // and if so, reconstruct the dependencies of this step
            if (step.mActorInstances.size() < numActorInstancesPreRemove)
            {
                // clear the dependencies (but don't delete the memory)
                step.mDependencies.Clear(false);

                // calculate the new dependencies for this step
                for (ActorInstance* stepActorInstance : step.mActorInstances)
                {
                    AddDependenciesToStep(stepActorInstance, &step);
                }

                // assume that there is only one of the same actor instance in the whole schedule
                RemoveEmptySteps();
                return s;
            }
        }

        return 0;
    }


    // remove the actor instance (including all of its attachments)
    void MultiThreadScheduler::RecursiveRemoveActorInstance(ActorInstance* actorInstance, uint32 startStep)
    {
        MCore::LockGuardRecursive guard(mMutex);

        // remove the actual actor instance
        const uint32 step = RemoveActorInstance(actorInstance, startStep);

        // recursively remove all attachments as well
        const uint32 numAttachments = actorInstance->GetNumAttachments();
        for (uint32 i = 0; i < numAttachments; ++i)
        {
            ActorInstance* attachment = actorInstance->GetAttachment(i)->GetAttachmentActorInstance();
            if (attachment)
            {
                RecursiveRemoveActorInstance(attachment, step);
            }
        }
    }


    void MultiThreadScheduler::Lock()
    {
        mMutex.Lock();
    }


    void MultiThreadScheduler::Unlock()
    {
        mMutex.Unlock();
    }
}   // namespace EMotionFX
