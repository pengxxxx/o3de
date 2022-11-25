/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <utilities/AssetUtilEBusHelper.h>
#include <utilities/assetUtils.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Interface/Interface.h>
#include <gmock/gmock.h>
#include <AzCore/UnitTest/Mocks/MockFileIOBase.h>

namespace UnitTests
{
    struct MockMultiBuilderInfoHandler : public AssetProcessor::AssetBuilderInfoBus::Handler
    {
        ~MockMultiBuilderInfoHandler() override;

        struct AssetBuilderExtraInfo
        {
            QString m_jobFingerprint;
            QString m_dependencyFilePath;
            QString m_jobDependencyFilePath;
            QString m_analysisFingerprint;
            AZStd::vector<AZ::u32> m_subIdDependencies;
        };

        //! AssetProcessor::AssetBuilderInfoBus Interface
        void GetMatchingBuildersInfo(const AZStd::string& assetPath, AssetProcessor::BuilderInfoList& builderInfoList) override;
        void GetAllBuildersInfo(AssetProcessor::BuilderInfoList& builderInfoList) override;

        void CreateJobs(
            AssetBuilderExtraInfo extraInfo,
            const AssetBuilderSDK::CreateJobsRequest& request,
            AssetBuilderSDK::CreateJobsResponse& response);
        void ProcessJob(
            AssetBuilderExtraInfo extraInfo,
            const AssetBuilderSDK::ProcessJobRequest& request,
            AssetBuilderSDK::ProcessJobResponse& response);

        void CreateBuilderDesc(
            const QString& builderName,
            const QString& builderId,
            const AZStd::vector<AssetBuilderSDK::AssetBuilderPattern>& builderPatterns,
            const AssetBuilderExtraInfo& extraInfo);

        // Use this version if you intend to update the extraInfo struct dynamically (be sure extraInfo does not go out of scope)
        void CreateBuilderDescInfoRef(
            const QString& builderName,
            const QString& builderId,
            const AZStd::vector<AssetBuilderSDK::AssetBuilderPattern>& builderPatterns,
            const AssetBuilderExtraInfo& extraInfo);

        void CreateBuilderDesc(
            const QString& builderName,
            const QString& builderId,
            const AZStd::vector<AssetBuilderSDK::AssetBuilderPattern>& builderPatterns,
            const AssetBuilderSDK::CreateJobFunction& createJobsFunction,
            const AssetBuilderSDK::ProcessJobFunction& processJobFunction,
            AZStd::optional<QString> analysisFingerprint = AZStd::nullopt);

        AZStd::vector<AssetUtilities::BuilderFilePatternMatcher> m_matcherBuilderPatterns;
        AZStd::unordered_map<AZ::Uuid, AssetBuilderSDK::AssetBuilderDesc> m_builderDescMap;

        int m_createJobsCount = 0;
    };

    class MockComponentApplication
        : public AZ::ComponentApplicationBus::Handler
    {
    public:
        MockComponentApplication()
        {
            AZ::ComponentApplicationBus::Handler::BusConnect();
            AZ::Interface<AZ::ComponentApplicationRequests>::Register(this);
        }

        ~MockComponentApplication()
        {
            AZ::Interface<AZ::ComponentApplicationRequests>::Unregister(this);
            AZ::ComponentApplicationBus::Handler::BusDisconnect();
        }

    public:
        MOCK_METHOD1(FindEntity, AZ::Entity* (const AZ::EntityId&));
        MOCK_METHOD1(AddEntity, bool(AZ::Entity*));
        MOCK_METHOD0(Destroy, void());
        MOCK_METHOD1(RegisterComponentDescriptor, void(const AZ::ComponentDescriptor*));
        MOCK_METHOD1(UnregisterComponentDescriptor, void(const AZ::ComponentDescriptor*));
        MOCK_METHOD1(RegisterEntityAddedEventHandler, void(AZ::EntityAddedEvent::Handler&));
        MOCK_METHOD1(RegisterEntityRemovedEventHandler, void(AZ::EntityRemovedEvent::Handler&));
        MOCK_METHOD1(RegisterEntityActivatedEventHandler, void(AZ::EntityActivatedEvent::Handler&));
        MOCK_METHOD1(RegisterEntityDeactivatedEventHandler, void(AZ::EntityDeactivatedEvent::Handler&));
        MOCK_METHOD1(SignalEntityActivated, void(AZ::Entity*));
        MOCK_METHOD1(SignalEntityDeactivated, void(AZ::Entity*));
        MOCK_METHOD1(RemoveEntity, bool(AZ::Entity*));
        MOCK_METHOD1(DeleteEntity, bool(const AZ::EntityId&));
        MOCK_METHOD1(GetEntityName, AZStd::string(const AZ::EntityId&));
        MOCK_METHOD1(EnumerateEntities, void(const ComponentApplicationRequests::EntityCallback&));
        MOCK_METHOD0(GetApplication, AZ::ComponentApplication* ());
        MOCK_METHOD0(GetSerializeContext, AZ::SerializeContext* ());
        MOCK_METHOD0(GetJsonRegistrationContext, AZ::JsonRegistrationContext* ());
        MOCK_METHOD0(GetBehaviorContext, AZ::BehaviorContext* ());
        MOCK_CONST_METHOD0(GetEngineRoot, const char* ());
        MOCK_CONST_METHOD0(GetExecutableFolder, const char* ());
        MOCK_CONST_METHOD1(QueryApplicationType, void(AZ::ApplicationTypeQuery&));
    };

    struct MockPathConversion : AZ::Interface<AssetProcessor::IPathConversion>::Registrar
    {
        MockPathConversion(const char* scanfolder = "c:/somepath")
        {
            m_scanFolderInfo = AssetProcessor::ScanFolderInfo{ scanfolder, "scanfolder", "scanfolder", true, true, { AssetBuilderSDK::PlatformInfo{ "pc", {} } }, 0, 1 };
        }

        bool ConvertToRelativePath(QString fullFileName, QString& databaseSourceName, QString& scanFolderName) const override
        {
            EXPECT_TRUE(fullFileName.startsWith(m_scanFolderInfo.ScanPath(), Qt::CaseInsensitive));

            scanFolderName = m_scanFolderInfo.ScanPath();
            databaseSourceName = fullFileName.mid(scanFolderName.size() + 1);

            return true;
        }

        const AssetProcessor::ScanFolderInfo* GetScanFolderForFile(const QString& /*fullFileName*/) const override
        {
            return &m_scanFolderInfo;
        }

        const AssetProcessor::ScanFolderInfo* GetScanFolderById(AZ::s64 /*id*/) const override
        {
            return &m_scanFolderInfo;
        }

    private:
        AssetProcessor::ScanFolderInfo m_scanFolderInfo;
    };

    struct MockVirtualFileIO
    {
        MockVirtualFileIO()
        {
            // Cache the existing file io instance and build our mock file io
            m_priorFileIO = AZ::IO::FileIOBase::GetInstance();
            m_fileIOMock = AZStd::make_unique<testing::NiceMock<AZ::IO::MockFileIOBase>>();

            // Swap out current file io instance for our mock
            AZ::IO::FileIOBase::SetInstance(nullptr);
            AZ::IO::FileIOBase::SetInstance(m_fileIOMock.get());

            // Setup the default returns for our mock file io calls
            AZ::IO::MockFileIOBase::InstallDefaultReturns(*m_fileIOMock.get());

            using namespace ::testing;
            using namespace AZ;

            ON_CALL(*m_fileIOMock, Open(_, _, _))
                .WillByDefault(Invoke(
                    [](auto filePath, auto, IO::HandleType& handle)
                    {
                        handle = AZ::u32(AZStd::hash<AZStd::string>{}(filePath));
                        return AZ::IO::Result(AZ::IO::ResultCode::Success);
                    }));

            ON_CALL(*m_fileIOMock, Tell(_, _))
                .WillByDefault(Invoke(
                    [](auto, auto& offset)
                    {
                        offset = 0;
                        return AZ::IO::ResultCode::Success;
                    }));

            ON_CALL(*m_fileIOMock, Size(An<AZ::IO::HandleType>(), _))
                .WillByDefault(Invoke(
                    [this](auto handle, AZ::u64& size)
                    {
                        size = m_mockFiles[handle].size();
                        return AZ::IO::ResultCode::Success;
                    }));

            ON_CALL(*m_fileIOMock, Size(An<const char*>(), _))
                .WillByDefault(Invoke(
                    [this](const char* filePath, AZ::u64& size)
                    {
                        auto handle = AZ::u32(AZStd::hash<AZStd::string>{}(filePath));
                        size = m_mockFiles[handle].size();
                        return AZ::IO::ResultCode::Success;
                    }));

            ON_CALL(*m_fileIOMock, Exists(_))
                .WillByDefault(Invoke(
                    [this](const char* filePath)
                    {
                        auto handle = AZ::u32(AZStd::hash<AZStd::string>{}(filePath));
                        auto itr = m_mockFiles.find(handle);
                        return itr != m_mockFiles.end() && itr->second.size() > 0;
                    }));

            ON_CALL(*m_fileIOMock, Rename(_, _))
                .WillByDefault(Invoke(
                    [this](const char* originalPath, const char* newPath)
                    {
                        auto originalHandle = AZ::u32(AZStd::hash<AZStd::string>{}(originalPath));
                        auto newHandle = AZ::u32(AZStd::hash<AZStd::string>{}(newPath));
                        auto itr = m_mockFiles.find(originalHandle);

                        if (itr != m_mockFiles.end())
                        {
                            m_mockFiles[newHandle] = itr->second;
                            m_mockFiles.erase(itr);

                            return AZ::IO::ResultCode::Success;
                        }

                        return AZ::IO::ResultCode::Error;
                    }));

            ON_CALL(*m_fileIOMock, Remove(_))
                .WillByDefault(Invoke(
                    [this](const char* path)
                    {
                        auto handle = AZ::u32(AZStd::hash<AZStd::string>{}(path));

                        m_mockFiles.erase(handle);

                        return AZ::IO::ResultCode::Success;
                    }));

            ON_CALL(*m_fileIOMock, Read(_, _, _, _, _))
                .WillByDefault(Invoke(
                    [this](auto handle, void* buffer, auto, auto, AZ::u64* bytesRead)
                    {
                        auto itr = m_mockFiles.find(handle);

                        if (itr == m_mockFiles.end())
                        {
                            return AZ::IO::ResultCode::Error;
                        }

                        memcpy(buffer, itr->second.c_str(), itr->second.size());
                        *bytesRead = itr->second.size();
                        return AZ::IO::ResultCode::Success;
                    }));

            ON_CALL(*m_fileIOMock, Write(_, _, _, _))
                .WillByDefault(Invoke(
                    [this](IO::HandleType fileHandle, const void* buffer, AZ::u64 size, AZ::u64* bytesWritten)
                    {
                        AZStd::string& file = m_mockFiles[fileHandle];

                        file.resize(size);
                        memcpy((void*)file.c_str(), buffer, size);

                        if (bytesWritten)
                        {
                            *bytesWritten = size;
                        }

                        return AZ::IO::ResultCode::Success;
                    }));
        }

        ~MockVirtualFileIO()
        {
            AZ::IO::FileIOBase::SetInstance(nullptr);
            AZ::IO::FileIOBase::SetInstance(m_priorFileIO);
        }

        AZ::IO::FileIOBase* m_priorFileIO = nullptr;
        AZStd::unordered_map<AZ::IO::HandleType, AZStd::string> m_mockFiles;
        AZStd::unique_ptr<testing::NiceMock<AZ::IO::MockFileIOBase>> m_fileIOMock;
    };
}
