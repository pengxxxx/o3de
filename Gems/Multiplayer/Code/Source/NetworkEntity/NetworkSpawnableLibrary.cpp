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

#include <Source/NetworkEntity/NetworkSpawnableLibrary.h>
#include <AzCore/Asset/AssetManagerBus.h>
#include <AzFramework/Spawnable/Spawnable.h>
#include <AzCore/StringFunc/StringFunc.h>
#include <AzCore/Interface/Interface.h>

namespace Multiplayer
{
    NetworkSpawnableLibrary::NetworkSpawnableLibrary()
    {
        AZ::Interface<INetworkSpawnableLibrary>::Register(this);
        AzFramework::AssetCatalogEventBus::Handler::BusConnect();
    }

    NetworkSpawnableLibrary::~NetworkSpawnableLibrary()
    {
        AzFramework::AssetCatalogEventBus::Handler::BusDisconnect();
        AZ::Interface<INetworkSpawnableLibrary>::Unregister(this);
    }

    void NetworkSpawnableLibrary::BuildSpawnablesList()
    {
        m_spawnables.clear();
        m_spawnablesReverseLookup.clear();

        auto enumerateCallback = [this](const AZ::Data::AssetId id, const AZ::Data::AssetInfo& info)
        {
            if (info.m_assetType == AZ::AzTypeInfo<AzFramework::Spawnable>::Uuid())
            {
                ProcessSpawnableAsset(info.m_relativePath, id);
            }
        };

        AZ::Data::AssetCatalogRequestBus::Broadcast(&AZ::Data::AssetCatalogRequests::EnumerateAssets, nullptr,
            enumerateCallback, nullptr);
    }

    void NetworkSpawnableLibrary::ProcessSpawnableAsset(const AZStd::string& relativePath, const AZ::Data::AssetId id)
    {
        const AZ::Name name = AZ::Name(relativePath);
        m_spawnables[name] = id;
        m_spawnablesReverseLookup[id] = name;
    }

    void NetworkSpawnableLibrary::OnCatalogLoaded([[maybe_unused]] const char* catalogFile)
    {
        BuildSpawnablesList();
    }

    AZ::Name NetworkSpawnableLibrary::GetSpawnableNameFromAssetId(AZ::Data::AssetId assetId)
    {
        if (assetId.IsValid())
        {
            auto it = m_spawnablesReverseLookup.find(assetId);
            if (it != m_spawnablesReverseLookup.end())
            {
                return it->second;
            }
        }

        return {};
    }

    AZ::Data::AssetId NetworkSpawnableLibrary::GetAssetIdByName(AZ::Name name)
    {
        auto it = m_spawnables.find(name);
        if (it != m_spawnables.end())
        {
            return it->second;
        }

        return {};
    }
}
