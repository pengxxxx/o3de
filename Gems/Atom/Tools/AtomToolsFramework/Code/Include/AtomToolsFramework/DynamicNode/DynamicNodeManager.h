/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AtomToolsFramework/DynamicNode/DynamicNodeManagerRequestBus.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/std/containers/unordered_map.h>

namespace AtomToolsFramework
{
    //! Manages a collection of dynamic node configuration objects.
    //! Provides functions for loading, registering, clearing, and sorting configurations as well as generating node palette items.
    class DynamicNodeManager : public DynamicNodeManagerRequestBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR(DynamicNodeManager, AZ::SystemAllocator, 0);
        AZ_RTTI(Graph, "{0DE0A2FA-3296-4E11-AA7F-831FAFA4126F}");
        AZ_DISABLE_COPY_MOVE(DynamicNodeManager);

        DynamicNodeManager(const AZ::Crc32& toolId);
        virtual ~DynamicNodeManager();

        //! DynamicNodeManagerRequestBus::Handler overrides...
        void LoadConfigFiles(const AZStd::unordered_set<AZStd::string>& extensions) override;
        void RegisterConfig(const AZStd::string& configId, const DynamicNodeConfig& config) override;
        DynamicNodeConfig GetConfig(const AZStd::string& configId) const override;
        void Clear() override;
        GraphCanvas::GraphCanvasTreeItem* CreateNodePaletteTree() const override;

    private:
        const AZ::Crc32 m_toolId = {};
        AZStd::unordered_map<AZStd::string, DynamicNodeConfig> m_dynamicNodeConfigMap;
    };

} // namespace AtomToolsFramework
