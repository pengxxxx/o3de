/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/functional.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzToolsFramework/API/ViewportEditorModeTrackerNotificationBus.h>
#include <Atom/Feature/PostProcess/EditorModeFeedback/EditorModeFeedbackInterface.h>
#include <AtomCore/Instance/Instance.h>

namespace AZ
{
    namespace RPI
    {
        class MeshDrawPacket;
        class Material;
    }

    namespace Render
    {
        //! Component for the editor mode feedback system.
        class EditorEditorModeFeedbackSystemComponent
            : public AzToolsFramework::Components::EditorComponentBase
            , public EditorModeFeedbackInterface
            , public AZ::TickBus::Handler
            , private AzToolsFramework::ViewportEditorModeNotificationsBus::Handler
        {
        public:
            AZ_EDITOR_COMPONENT(EditorEditorModeFeedbackSystemComponent, "{A88EE29D-4C72-4995-B3BD-41EEDE480487}");

            static void Reflect(AZ::ReflectContext* context);

            ~EditorEditorModeFeedbackSystemComponent();

            // EditorComponentBase overrides ...
            void Init() override;
            void Activate() override;
            void Deactivate() override;

            // EditorModeFeedbackInterface overrides ...
            bool IsEnabled() const override;
            void RegisterDrawableComponent(EntityComponentIdPair entityComponentId, const MeshFeatureProcessorInterface::MeshHandle& meshHandle) override;

        private:
            // ViewportEditorModeNotificationsBus overrides ...
            void OnEditorModeActivated(
                const AzToolsFramework::ViewportEditorModesInterface& editorModeState, AzToolsFramework::ViewportEditorMode mode) override;
            void OnEditorModeDeactivated(
                const AzToolsFramework::ViewportEditorModesInterface& editorModeState, AzToolsFramework::ViewportEditorMode mode) override;

            // TickBus overrides ...
            void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
            int GetTickOrder() override;

            //! Flag to specify whether or not the editor feedback effects are active.
            bool m_enabled = false;

            //! Data to construct draw packets for meshes.
            struct MeshHandleDrawPackets
            {
                ~MeshHandleDrawPackets();

                const MeshFeatureProcessorInterface::MeshHandle* m_meshHandle;
                AZStd::vector<RPI::MeshDrawPacket> m_meshDrawPackets;
            };
            
            //! Map for entities and their drawable components.
            AZStd::unordered_map<EntityId, AZStd::unordered_map<ComponentId, MeshHandleDrawPackets>> m_entityComponentMeshHandleDrawPackets;

            //! Material for sending draw packets to the entity mask pass.
            Data::Instance<RPI::Material> m_maskMaterial = nullptr;
        };
    }
}
