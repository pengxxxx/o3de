#
# All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
# its licensors.
#
# For complete copyright and license terms please see the LICENSE at the root of this
# distribution (the "License"). All use of this software is governed by the License,
# or, if provided, by the license below or the license accompanying this file. Do not
# remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#

set(LY_RUNTIME_DEPENDENCIES_HEADER
"function(ly_copy source_file target_directory)
    get_filename_component(target_filename \"\${source_file}\" NAME)
    if(NOT \"\${source_file}\" STREQUAL \"\${target_directory}/\${target_filename}\")
        if(NOT EXISTS \"\${target_directory}\")
            file(MAKE_DIRECTORY \"\${target_directory}\")
        endif()
        if(\"\${source_file}\" IS_NEWER_THAN \"\${target_directory}/\${target_filename}\")
            file(LOCK \"\${CMAKE_BINARY_DIR}/runtimedependencies.lock\" GUARD FUNCTION TIMEOUT 30)
            file(COPY \"\${source_file}\" DESTINATION \"\${target_directory}\" FILE_PERMISSIONS ${LY_COPY_PERMISSIONS} FOLLOW_SYMLINK_CHAIN)
        endif()
    endif()    
endfunction()
\n")

include(cmake/Platform/Common/RuntimeDependencies_common.cmake)