/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <AzToolsFramework/Metadata/MetadataManager.h>
#include <AzCore/Serialization/Json/JsonSystemComponent.h>
#include <AzCore/Serialization/Json/RegistrationContext.h>
#include <AzCore/UnitTest/Mocks/MockFileIOBase.h>
#include <AzCore/Component/ComponentApplication.h>
#include <AzCore/Utils/Utils.h>
#include <native/utilities/UuidManager.h>
#include <AssetManager/SourceAssetReference.h>
#include <tests/UnitTestUtilities.h>

namespace UnitTests
{
    struct UuidManagerTests
        : UnitTest::LeakDetectionFixture
    {
        void SetUp() override
        {
            UnitTest::TestRunner::Instance().m_suppressPrintf = false;
            UnitTest::TestRunner::Instance().m_suppressAsserts = false;
            UnitTest::TestRunner::Instance().m_suppressErrors = false;
            UnitTest::TestRunner::Instance().m_suppressOutput = false;
            UnitTest::TestRunner::Instance().m_suppressWarnings = false;

            m_serializeContext = AZStd::make_unique<AZ::SerializeContext>();
            m_jsonRegistrationContext = AZStd::make_unique<AZ::JsonRegistrationContext>();
            m_componentApplication = AZStd::make_unique<testing::NiceMock<MockComponentApplication>>();

            using namespace testing;

            ON_CALL(*m_componentApplication.get(), GetSerializeContext()).WillByDefault(Return(m_serializeContext.get()));
            ON_CALL(*m_componentApplication.get(), GetJsonRegistrationContext()).WillByDefault(Return(m_jsonRegistrationContext.get()));

            AZ::JsonSystemComponent::Reflect(m_jsonRegistrationContext.get());

            AssetProcessor::UuidManager::Reflect(m_serializeContext.get());

            m_uuidInterface = AZ::Interface<AssetProcessor::IUuidRequests>::Get();

            ASSERT_TRUE(m_uuidInterface);

            // Enable txt files by default for these tests
            m_uuidInterface->EnableGenerationForTypes({ ".txt" });
        }

        void TearDown() override
        {
            m_jsonRegistrationContext->EnableRemoveReflection();
            AZ::JsonSystemComponent::Reflect(m_jsonRegistrationContext.get());
            m_jsonRegistrationContext->DisableRemoveReflection();

            m_jsonRegistrationContext.reset();
            m_serializeContext.reset();
        }

        AZStd::unique_ptr<AZ::SerializeContext> m_serializeContext;
        AZStd::unique_ptr<AZ::JsonRegistrationContext> m_jsonRegistrationContext;

        AzToolsFramework::MetadataManager m_metadataManager;
        AssetProcessor::UuidManager m_uuidManager;
        MockPathConversion m_pathConversion;
        MockVirtualFileIO m_virtualFileIO;
        AZStd::unique_ptr<testing::NiceMock<MockComponentApplication>> m_componentApplication;

        AssetProcessor::IUuidRequests* m_uuidInterface{};
    };

    TEST_F(UuidManagerTests, GetUuid_FirstTime_ReturnsRandomUuid)
    {
        static constexpr const char* TestFile = "c:/somepath/mockfile.txt";
        auto uuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(TestFile));

        EXPECT_FALSE(uuid.IsNull());
        // Make sure a metadata file was created
        EXPECT_TRUE(AZ::IO::FileIOBase::GetInstance()->Exists(AZStd::string::format("%s%s", TestFile, AzToolsFramework::MetadataManager::MetadataFileExtension).c_str()));
    }

    TEST_F(UuidManagerTests, GetUuidTwice_ReturnsSameUuid)
    {
        static constexpr const char* TestFile = "c:/somepath/Mockfile.txt";

        auto uuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(TestFile));

        EXPECT_FALSE(uuid.IsNull());

        auto uuid2 = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(TestFile));

        EXPECT_EQ(uuid, uuid2);
    }

    TEST_F(UuidManagerTests, GetUuid_DifferentFiles_ReturnsDifferentUuid)
    {
        static constexpr const char* FileA = "c:/somepath/fileA.txt";
        static constexpr const char* FileB = "c:/somepath/fileB.txt";

        auto uuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(FileA));

        EXPECT_FALSE(uuid.IsNull());

        auto uuid2 = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(FileB));

        EXPECT_NE(uuid, uuid2);
    }

    TEST_F(UuidManagerTests, GetLegacyUuids_UppercaseFileName_ReturnsTwoDifferentUuids)
    {
        auto uuids = m_uuidInterface->GetLegacyUuids(AssetProcessor::SourceAssetReference("c:/somepath/Mockfile.txt"));

        ASSERT_EQ(uuids.size(), 2);
        EXPECT_NE(*uuids.begin(), *++uuids.begin());
    }

    TEST_F(UuidManagerTests, GetLegacyUuids_LowercaseFileName_ReturnsOneUuid)
    {
        auto uuids = m_uuidInterface->GetLegacyUuids(AssetProcessor::SourceAssetReference("c:/somepath/mockfile.txt"));

        EXPECT_EQ(uuids.size(), 1);
    }

    TEST_F(UuidManagerTests, GetLegacyUuids_DifferentFromCanonicalUuid)
    {
        static constexpr const char* TestFile = "c:/somepath/Mockfile.txt";

        auto legacyUuids = m_uuidInterface->GetLegacyUuids(AssetProcessor::SourceAssetReference(TestFile));

        ASSERT_EQ(legacyUuids.size(), 2);

        auto canonicalUuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(TestFile));

        EXPECT_THAT(legacyUuids, ::testing::Not(::testing::Contains(canonicalUuid)));
    }

    TEST_F(UuidManagerTests, MoveFile_UuidRemainsTheSame)
    {
        static constexpr const char* FileA = "c:/somepath/mockfile.txt";
        static constexpr const char* FileB = "c:/somepath/newfile.txt";

        auto uuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(FileA));

        AZ::IO::FileIOBase::GetInstance()->Rename(
            (AZStd::string(FileA) + AzToolsFramework::MetadataManager::MetadataFileExtension).c_str(),
            (AZStd::string(FileB) + AzToolsFramework::MetadataManager::MetadataFileExtension).c_str());

        m_uuidInterface->FileRemoved((AZStd::string(FileA) + AzToolsFramework::MetadataManager::MetadataFileExtension).c_str());

        auto movedUuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(FileB));

        EXPECT_EQ(uuid, movedUuid);
    }

    TEST_F(UuidManagerTests, MoveFileWithComplexName_UuidRemainsTheSame)
    {
        static constexpr const char* FileA = "c:/somepath/mockfile.ext1.ext2.txt";
        static constexpr const char* FileB = "c:/somepath/newfile.txt";

        auto uuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(FileA));

        AZ::IO::FileIOBase::GetInstance()->Rename(
            (AZStd::string(FileA) + AzToolsFramework::MetadataManager::MetadataFileExtension).c_str(),
            (AZStd::string(FileB) + AzToolsFramework::MetadataManager::MetadataFileExtension).c_str());

        m_uuidInterface->FileRemoved((AZStd::string(FileA) + AzToolsFramework::MetadataManager::MetadataFileExtension).c_str());

        auto movedUuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(FileB));

        EXPECT_EQ(uuid, movedUuid);
    }

    TEST_F(UuidManagerTests, MetadataRemoved_NewUuidAssigned)
    {
        static constexpr const char* TestFile = "c:/somepath/mockfile.txt";

        auto uuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(TestFile));

        AZ::IO::FileIOBase::GetInstance()->Remove(
            (AZStd::string(TestFile) + AzToolsFramework::MetadataManager::MetadataFileExtension).c_str());

        m_uuidInterface->FileRemoved((AZStd::string(TestFile) + AzToolsFramework::MetadataManager::MetadataFileExtension).c_str());

        auto movedUuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(TestFile));

        EXPECT_NE(uuid, movedUuid);
    }

    TEST_F(UuidManagerTests, MetadataUpdated_NewUuidAssigned)
    {
        static constexpr const char* FileA = "c:/somepath/mockfile.test.txt";
        static constexpr const char* FileB = "c:/somepath/someotherfile.txt";

        auto uuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(FileA));

        // Generate another metadata file, its the easiest way to "change" a UUID in the metadata file for this test
        m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(FileB));

        // Copy FileB's metadata onto the FileA metadata
        AZ::IO::FileIOBase::GetInstance()->Rename(
            (AZStd::string(FileB) + AzToolsFramework::MetadataManager::MetadataFileExtension).c_str(),
            (AZStd::string(FileA) + AzToolsFramework::MetadataManager::MetadataFileExtension).c_str());

        m_uuidInterface->FileChanged((AZStd::string(FileA) + AzToolsFramework::MetadataManager::MetadataFileExtension).c_str());

        auto newUuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(FileA));

        EXPECT_NE(uuid, newUuid);
    }

    TEST_F(UuidManagerTests, RequestUuid_DisabledType_ReturnsLegacyUuid)
    {
        static constexpr const char* TestFile = "c:/somepath/mockfile.png";

        auto uuid = m_uuidInterface->GetUuid(AssetProcessor::SourceAssetReference(TestFile));
        auto legacyUuids = m_uuidInterface->GetLegacyUuids(AssetProcessor::SourceAssetReference(TestFile));

        EXPECT_THAT(legacyUuids, ::testing::Contains(uuid));

        // Make sure no metadata file was created
        EXPECT_FALSE(AZ::IO::FileIOBase::GetInstance()->Exists(
            AZStd::string::format("%s%s", TestFile, AzToolsFramework::MetadataManager::MetadataFileExtension).c_str()));
    }
}
