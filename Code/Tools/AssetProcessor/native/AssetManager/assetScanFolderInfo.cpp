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

#include <native/AssetManager/assetScanFolderInfo.h>
#include <native/utilities/assetUtils.h>

namespace AssetProcessor
{
        ScanFolderInfo::ScanFolderInfo(
            QString path,
            QString displayName,
            QString portableKey,
            bool isRoot,
            bool recurseSubFolders,
            AZStd::vector<AssetBuilderSDK::PlatformInfo> platforms,
            int order,
            AZ::s64 scanFolderID,
            bool canSaveNewAssets)
            : m_scanPath(path)
            , m_displayName(displayName)
            , m_portableKey (portableKey)
            , m_isRoot(isRoot)
            , m_recurseSubFolders(recurseSubFolders)
            , m_order(order)
            , m_scanFolderID(scanFolderID)
            , m_platforms(platforms)
            , m_canSaveNewAssets(canSaveNewAssets)
        {
            m_scanPath = AssetUtilities::NormalizeFilePath(m_scanPath);
            // note that m_scanFolderID is 0 unless its filled in from the DB.
        }

} // end namespace AssetProcessor
