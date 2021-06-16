// {BEGIN_LICENSE}
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
// {END_LICENSE}


#include <${Name}ModuleInterface.h>
#include <${Name}SystemComponent.h>

namespace ${SanitizedCppName}
{
    class ${SanitizedCppName}Module
        : public ${SanitizedCppName}ModuleInterface
    {
    public:
        AZ_RTTI(${SanitizedCppName}Module, "${ModuleClassId}", ${SanitizedCppName}ModuleInterface);
        AZ_CLASS_ALLOCATOR(${SanitizedCppName}Module, AZ::SystemAllocator, 0);
    };
}// namespace ${SanitizedCppName}

AZ_DECLARE_MODULE_CLASS(Gem_${SanitizedCppName}, ${SanitizedCppName}::${SanitizedCppName}Module)
