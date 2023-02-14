/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <QCoreApplication>
#include <native/tests/assetmanager/DelayRelocationTests.h>
#include <native/unittests/UnitTestUtils.h>
#include <AzFramework/IO/LocalFileIO.h>

namespace UnitTests
{
    void DelayRelocationTests::SetUp()
    {
        AssetManagerTestingBase::SetUp();

        using namespace AssetBuilderSDK;

        m_uuidInterface = AZ::Interface<AssetProcessor::IUuidRequests>::Get();
        ASSERT_TRUE(m_uuidInterface);

        m_uuidInterface->EnableGenerationForTypes({ ".stage1" });

        m_assetProcessorManager->SetMetaCreationDelay(MetadataProcessingDelayMs);

        CreateBuilder("stage1", "*.stage1", "stage2", false, ProductOutputFlags::ProductAsset);
        ProcessFileMultiStage(1, true);
        QCoreApplication::processEvents();
    }

    TEST_F(DelayRelocationTests, DeleteMetadata_WithDelay_MetadataIsRecreated)
    {
        bool delayed = false;
        QObject::connect(
            m_assetProcessorManager.get(),
            &AssetProcessor::AssetProcessorManager::ProcessingDelayed,
            [&delayed](QString)
            {
                delayed = true;
            });

        auto expectedMetadataPath = AzToolsFramework::MetadataManager::ToMetadataPath(m_testFilePath);

        EXPECT_TRUE(AZ::IO::FileIOBase::GetInstance()->Exists(expectedMetadataPath.c_str())) << expectedMetadataPath.c_str();

        AZ::IO::FileIOBase::GetInstance()->Remove(expectedMetadataPath.c_str());
        m_uuidInterface->FileRemoved(expectedMetadataPath);

        // Reprocess
        QMetaObject::invokeMethod(
            m_assetProcessorManager.get(), "AssessDeletedFile", Qt::QueuedConnection, Q_ARG(QString, expectedMetadataPath.c_str()));
        QCoreApplication::processEvents();

        // Reset state
        m_jobDetailsList.clear();
        m_fileCompiled = false;
        m_fileFailed = false;

        RunFile(0, 1);

        // Check the metadata file has been recreated
        EXPECT_TRUE(AZ::IO::FileIOBase::GetInstance()->Exists(expectedMetadataPath.c_str())) << expectedMetadataPath.c_str();
        EXPECT_TRUE(delayed);
    }

    TEST_F(DelayRelocationTests, RenameSource_WithDelay_MetadataIsCreated)
    {
        bool delayed = false;
        QObject::connect(
            m_assetProcessorManager.get(),
            &AssetProcessor::AssetProcessorManager::ProcessingDelayed,
            [&delayed](QString)
            {
                delayed = true;
            });

        auto oldPath = m_testFilePath;
        AZ::IO::Path scanFolderDir(m_scanfolder.m_scanFolder);
        AZStd::string testFilename = "renamed.stage1";
        AZStd::string newPath = (scanFolderDir / testFilename).AsPosix().c_str();

        AZ::IO::FileIOBase::GetInstance()->Rename(oldPath.c_str(), newPath.c_str());
        m_uuidInterface->FileRemoved(oldPath.c_str());

        // Process the delete first
        QMetaObject::invokeMethod(
            m_assetProcessorManager.get(), "AssessDeletedFile", Qt::QueuedConnection, Q_ARG(QString, oldPath.c_str()));
        QCoreApplication::processEvents();


        // Reset state
        m_jobDetailsList.clear();
        m_fileCompiled = false;
        m_fileFailed = false;

        RunFile(0, 1);
        EXPECT_FALSE(delayed);

        QCoreApplication::processEvents();

        QMetaObject::invokeMethod(m_assetProcessorManager.get(), "AssessAddedFile", Qt::QueuedConnection, Q_ARG(QString, newPath.c_str()));
        QCoreApplication::processEvents();

        // Reset state
        m_jobDetailsList.clear();
        m_fileCompiled = false;
        m_fileFailed = false;

        RunFile(1, 1);

        // Check the metadata file has been created
        auto expectedMetadataPath = AzToolsFramework::MetadataManager::ToMetadataPath(newPath);
        EXPECT_TRUE(AZ::IO::SystemFile::Exists(expectedMetadataPath.c_str())) << expectedMetadataPath.c_str();
        EXPECT_TRUE(delayed);
    }

    TEST_F(DelayRelocationTests, RenameSource_RenameMetadataDuringDelay_NoMetadataCreated)
    {
        auto oldPath = m_testFilePath;
        AZ::IO::Path scanFolderDir(m_scanfolder.m_scanFolder);
        AZStd::string testFilename = "renamed.stage1";
        AZStd::string newPath = (scanFolderDir / testFilename).AsPosix().c_str();
        bool delayed = false;

        auto originalUuid = AssetUtilities::GetSourceUuid(AssetProcessor::SourceAssetReference(oldPath.c_str()));
        ASSERT_TRUE(originalUuid);

        QObject::connect(
            m_assetProcessorManager.get(),
            &AssetProcessor::AssetProcessorManager::ProcessingDelayed,
            [&delayed, oldPath, newPath, this](QString)
            {
                delayed = true;

                // During the delay period, rename the metadata file
                AZ::IO::FileIOBase::GetInstance()->Rename(
                    AzToolsFramework::MetadataManager::ToMetadataPath(oldPath).c_str(),
                    AzToolsFramework::MetadataManager::ToMetadataPath(newPath).c_str());
                m_uuidInterface->FileRemoved(AzToolsFramework::MetadataManager::ToMetadataPath(oldPath));
            });

        AZ::IO::FileIOBase::GetInstance()->Rename(oldPath.c_str(), newPath.c_str());
        m_uuidInterface->FileRemoved(oldPath.c_str());

        // Process the delete first
        QMetaObject::invokeMethod(
            m_assetProcessorManager.get(), "AssessDeletedFile", Qt::QueuedConnection, Q_ARG(QString, oldPath.c_str()));
        QCoreApplication::processEvents();

        // Reset state
        m_jobDetailsList.clear();
        m_fileCompiled = false;
        m_fileFailed = false;

        RunFile(0, 1);
        EXPECT_FALSE(delayed);

        QCoreApplication::processEvents();

        QMetaObject::invokeMethod(m_assetProcessorManager.get(), "AssessAddedFile", Qt::QueuedConnection, Q_ARG(QString, newPath.c_str()));
        QCoreApplication::processEvents();

        // Reset state
        m_jobDetailsList.clear();
        m_fileCompiled = false;
        m_fileFailed = false;

        RunFile(1, 1);

        auto expectedMetadataPath = AzToolsFramework::MetadataManager::ToMetadataPath(newPath);
        EXPECT_TRUE(AZ::IO::SystemFile::Exists(expectedMetadataPath.c_str())) << expectedMetadataPath.c_str();
        EXPECT_TRUE(delayed);

        // Verify that the metadata file we renamed didn't get overwritten
        auto currentUuid = AssetUtilities::GetSourceUuid(AssetProcessor::SourceAssetReference(newPath.c_str()));
        ASSERT_TRUE(currentUuid);

        EXPECT_EQ(originalUuid.GetValue(), currentUuid.GetValue());
    }
} // namespace UnitTests
