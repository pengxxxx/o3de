/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>

#pragma once

namespace TestImpact
{
    namespace Client
    {
        //! The set of test targets selected to run regardless of whether or not the test targets are to be excluded either for being on the primary exclude
        //! list and/or being part of a test suite excluded from this run.
        //! @note Only the included test targets will be run. The excluded test targets, although selected, will not be run.
        class TestRunSelection
        {
        public:
            TestRunSelection(const AZStd::vector<AZStd::string>& includedTests, const AZStd::vector<AZStd::string>& excludedTests);
            TestRunSelection(AZStd::vector<AZStd::string>&& includedTests, AZStd::vector<AZStd::string>&& excludedTests);

            //! Returns the test runs that were selected to be run and will actually be run.
            const AZStd::vector<AZStd::string>& GetIncludededTestRuns() const;

            //! Returns the test runs that were selected to be run but will not actually be run.
            const AZStd::vector<AZStd::string>& GetExcludedTestRuns() const;

            //! Returns the number of selected test runs that will be run.
            size_t GetNumIncludedTestRuns() const;

            //! Returns the number of selected test runs that will not be run.
            size_t GetNumExcludedTestRuns() const;

            //! Returns the total number of test runs selected regardless of whether or not they will actually be run.
            size_t GetTotalNumTests() const;

        private:
            AZStd::vector<AZStd::string> m_includedTestRuns;
            AZStd::vector<AZStd::string> m_excludedTestRuns;
        };
    } // namespace Client
} // namespace TestImpact
