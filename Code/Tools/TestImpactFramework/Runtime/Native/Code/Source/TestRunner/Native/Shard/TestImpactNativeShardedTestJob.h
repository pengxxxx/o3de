/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <TestImpactFramework/TestImpactUtils.h>

#include <Process/JobRunner/TestImpactProcessJobInfo.h>
#include <Process/JobRunner/TestImpactProcessJobMeta.h>
#include <TestRunner/Native/Job/TestImpactNativeShardedTestJobInfoGenerator.h>

#include <AzCore/std/optional.h>
#include <AzCore/std/utility/to_underlying.h>

namespace TestImpact
{
    //!
    template<typename TestRunnerType>
    class ShardedTestJob
    {
    public:
        using ShardedTestJobInfoType = ShardedTestJobInfo<TestRunnerType>;
        using JobInfo = typename TestRunnerType::Job::Info;

        //!
        struct JobData;

        //!
        ShardedTestJob(const ShardedTestJobInfoType& shardedTestJobInfo);

        //!
        bool IsComplete() const;

        //!
        void RegisterCompletedSubJob(const JobInfo& jobInfo, const JobMeta& meta, const StdContent& std);

        //!
        const AZStd::optional<JobData>& GetConsolidatedJobData() const;

        //!
        const AZStd::vector<JobData>& GetSubJobs() const;

        //!
        static JobResult ResolveJobResult(const AZStd::optional<JobResult> jobResult, const JobResult subJobResult);

    private:
        const ShardedTestJobInfoType* m_shardedTestJobInfo = nullptr; //!<
        AZStd::vector<JobData> m_subJobs; //!<
        AZStd::optional<JobData> m_consolidatedJobData; //!<
        Timer m_timer; //!<
    };

    template<typename TestRunnerType>
    struct ShardedTestJob<TestRunnerType>::JobData
    {
        JobData(const JobInfo& jobInfo)
            : m_jobInfo(jobInfo)
        {
        }

        JobData(const JobInfo& jobInfo, const JobMeta& meta, const StdContent& std)
            : m_jobInfo(jobInfo)
            , m_meta(meta)
            , m_std(std)
        {
        }

        JobInfo m_jobInfo;
        JobMeta m_meta;
        StdContent m_std;
    };

    template<typename TestRunnerType>
    JobResult ShardedTestJob<TestRunnerType>::ResolveJobResult(const AZStd::optional<JobResult> jobResult, const JobResult subJobResult)
    {
        if (jobResult.has_value())
        {
            // Unless the subjob result is not executed, take the job result in reverse order of precedence
            // of the JobResult enumeration
            if (AZStd::to_underlying(subJobResult) < AZStd::to_underlying(jobResult.value()))
            {
                if (subJobResult == JobResult::NotExecuted)
                {
                    return jobResult.value();
                }

                return subJobResult;
            }
        }

        return subJobResult;
    }

    template<typename TestRunnerType>
    ShardedTestJob<TestRunnerType>::ShardedTestJob(const ShardedTestJobInfoType& shardedTestJobInfo)
        : m_shardedTestJobInfo(&shardedTestJobInfo)
    {
        m_subJobs.reserve(m_shardedTestJobInfo->GetJobInfos().size());
    }

    template<typename TestRunnerType>
    bool ShardedTestJob<TestRunnerType>::IsComplete() const
    {
        return m_subJobs.size() == m_shardedTestJobInfo->GetJobInfos().size();
    }

    template<typename TestRunnerType>
    void ShardedTestJob<TestRunnerType>::RegisterCompletedSubJob(const JobInfo& jobInfo, const JobMeta& meta, const StdContent& std)
    {
        m_subJobs.emplace_back(jobInfo, meta, std);

        if (IsComplete())
        {
            // Take the first job to be scheduled as for no sharding this will be the actual job and for sharding it
            // doesn't make a great deal of sense to try and consolodate the jobs at this level anyway (the completed
            // jobs returned by the sharded test runner will present all shards as a single completed job)
            m_consolidatedJobData = JobData(m_shardedTestJobInfo->GetJobInfos().front());

            AZStd::optional<JobResult> consolidatedJobResult;
            JobMeta& consolidatedMeta = m_consolidatedJobData->m_meta;

            consolidatedMeta.m_startTime = m_timer.GetStartTimePoint();
            consolidatedMeta.m_duration = m_timer.GetElapsedMs();

            for (const auto& subJob : m_subJobs)
            {
                // Resolve consolidated job result from existing sub job results
                consolidatedJobResult = ResolveJobResult(consolidatedJobResult, subJob.m_meta.m_result);

                // Technically, it would be possible to consolidate return codes at the job level as we could use the
                // platform/framework error code checkers that the test engine uses to determine what error codes map
                // to what test run results but it's not worth it so just take the highest error code value
                if (subJob.m_meta.m_returnCode.has_value() &&
                    (!consolidatedMeta.m_returnCode.has_value() ||
                     consolidatedMeta.m_returnCode.value() < subJob.m_meta.m_returnCode.value()))
                {
                    consolidatedMeta.m_returnCode = subJob.m_meta.m_returnCode;
                }

                // Accumulate the standard out/error of each sub job
                const auto stdAccumulator = [](const AZStd::optional<AZStd::string>& source, AZStd::optional<AZStd::string>& dest)
                {
                    if (source.has_value())
                    {
                        dest = dest.has_value() ? dest.value() + source.value() : source.value();
                    }
                };
                stdAccumulator(subJob.m_std.m_out, m_consolidatedJobData->m_std.m_out);
                stdAccumulator(subJob.m_std.m_err, m_consolidatedJobData->m_std.m_err);
            }

            consolidatedMeta.m_result = consolidatedJobResult.value_or(JobResult::NotExecuted);
        }
    }

    template<typename TestRunnerType>
    auto ShardedTestJob<TestRunnerType>::GetConsolidatedJobData() const -> const AZStd::optional<JobData>&
    {
        return m_consolidatedJobData;
    }

    template<typename TestRunnerType>
    auto ShardedTestJob<TestRunnerType>::GetSubJobs() const -> const AZStd::vector<JobData>&
    {
        return m_subJobs;
    }
} // namespace TestImpact
