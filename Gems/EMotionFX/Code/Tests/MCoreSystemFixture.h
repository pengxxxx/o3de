/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/UnitTest/TestTypes.h>

namespace EMotionFX
{
    class MCoreSystemFixture
        : public UnitTest::AllocatorsTestFixture
    {
    public:
        void SetUp() override;
        void TearDown() override;
    };
}
