/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzTest/AzTest.h>

class AtomToolsFrameworkTest
    : public ::testing::Test
{
protected:
    void SetUp() override
    {

    }

    void TearDown() override
    {

    }
};

TEST_F(AtomToolsFrameworkTest, SanityTest)
{
    ASSERT_TRUE(true);
}

AZ_UNIT_TEST_HOOK(DEFAULT_UNIT_TEST_ENV);
