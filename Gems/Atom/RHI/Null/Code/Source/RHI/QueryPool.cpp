/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include "Atom_RHI_Null_precompiled.h"
#include <RHI/QueryPool.h>

namespace AZ
{
    namespace Null
    {
        RHI::Ptr<QueryPool> QueryPool::Create()
        {
            return aznew QueryPool();
        }

        void QueryPool::OnFrameEnd()
        {
            Base::OnFrameEnd();
        }
    }
}
