/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "BoxComponentMode.h"
#include <AzToolsFramework/API/ComponentModeCollectionInterface.h>
#include <AzToolsFramework/ActionManager/Action/ActionManagerInterface.h>
#include <AzToolsFramework/ActionManager/HotKey/HotKeyManagerInterface.h>
#include <AzToolsFramework/ActionManager/Menu/MenuManagerInterface.h>

namespace AzToolsFramework
{
    static constexpr AZStd::string_view EditorMainWindowActionContextIdentifier = "o3de.context.editor.mainwindow";
    static constexpr AZStd::string_view EditMenuIdentifier = "o3de.menu.editor.edit";

    namespace
    {
        //! Uri's for shortcut actions.
        const AZ::Crc32 SetDimensionsSubModeActionUri = AZ_CRC_CE("org.o3de.action.box.setdimensionssubmode");
        const AZ::Crc32 SetTranslationOffsetSubModeActionUri = AZ_CRC_CE("org.o3de.action.box.settranslationoffsetsubmode");
        const AZ::Crc32 ResetSubModeActionUri = AZ_CRC_CE("org.o3de.action.box.resetsubmode");
    } // namespace

    AZ_CLASS_ALLOCATOR_IMPL(BoxComponentMode, AZ::SystemAllocator, 0)

    void BoxComponentMode::Reflect(AZ::ReflectContext* context)
    {
        ComponentModeFramework::ReflectEditorBaseComponentModeDescendant<BoxComponentMode>(context);
    }

    BoxComponentMode::BoxComponentMode(
        const AZ::EntityComponentIdPair& entityComponentIdPair, const AZ::Uuid componentType, bool allowAsymmetricalEditing)
        : EditorBaseComponentMode(entityComponentIdPair, componentType)
        , m_entityComponentIdPair(entityComponentIdPair)
        , m_allowAsymmetricalEditing(allowAsymmetricalEditing)
    {
        m_subModes[static_cast<AZ::u32>(ShapeComponentModeRequests::SubMode::Dimensions)] =
            AZStd::make_unique<BoxViewportEdit>(m_allowAsymmetricalEditing);
        if (m_allowAsymmetricalEditing)
        {
            m_subModes[static_cast<AZ::u32>(ShapeComponentModeRequests::SubMode::TranslationOffset)] =
                AZStd::make_unique<ShapeTranslationOffsetViewportEdit>();
            SetupCluster();
            SetCurrentMode(ShapeComponentModeRequests::SubMode::Dimensions);
        }
        else
        {
            m_subModes[static_cast<AZ::u32>(ShapeComponentModeRequests::SubMode::Dimensions)]->Setup(entityComponentIdPair);
        }
        ShapeComponentModeRequestBus::Handler::BusConnect(m_entityComponentIdPair);
    }

    AZStd::vector<AzToolsFramework::ActionOverride> BoxComponentMode::PopulateActionsImpl()
    {
        if (!m_allowAsymmetricalEditing)
        {
            return {};
        }

        AzToolsFramework::ActionOverride setDimensionsModeAction;
        setDimensionsModeAction.SetUri(SetDimensionsSubModeActionUri);
        setDimensionsModeAction.SetKeySequence(QKeySequence(Qt::Key_1));
        setDimensionsModeAction.SetTitle("Set Dimensions Mode");
        setDimensionsModeAction.SetTip("Set dimensions mode");
        setDimensionsModeAction.SetEntityComponentIdPair(GetEntityComponentIdPair());
        setDimensionsModeAction.SetCallback(
            [this]()
            {
                SetCurrentMode(SubMode::Dimensions);
            });

        AzToolsFramework::ActionOverride setTranslationOffsetModeAction;
        setTranslationOffsetModeAction.SetUri(SetTranslationOffsetSubModeActionUri);
        setTranslationOffsetModeAction.SetKeySequence(QKeySequence(Qt::Key_2));
        setTranslationOffsetModeAction.SetTitle("Set Translation Offset Mode");
        setTranslationOffsetModeAction.SetTip("Set translation offset mode");
        setTranslationOffsetModeAction.SetEntityComponentIdPair(GetEntityComponentIdPair());
        setTranslationOffsetModeAction.SetCallback(
            [this]()
            {
                SetCurrentMode(SubMode::TranslationOffset);
            });

        AzToolsFramework::ActionOverride resetModeAction;
        resetModeAction.SetUri(ResetSubModeActionUri);
        resetModeAction.SetKeySequence(QKeySequence(Qt::Key_R));
        resetModeAction.SetTitle("Reset Current Mode");
        resetModeAction.SetTip("Reset current mode");
        resetModeAction.SetEntityComponentIdPair(GetEntityComponentIdPair());
        resetModeAction.SetCallback(
            [this]()
            {
                ResetCurrentMode();
            });

        return { setDimensionsModeAction, setTranslationOffsetModeAction, resetModeAction };
    }

    void BoxComponentMode::RegisterActions()
    {
        auto actionManagerInterface = AZ::Interface<AzToolsFramework::ActionManagerInterface>::Get();
        AZ_Assert(actionManagerInterface, "BoxComponentMode - could not get ActionManagerInterface on RegisterActions.");

        auto hotKeyManagerInterface = AZ::Interface<AzToolsFramework::HotKeyManagerInterface>::Get();
        AZ_Assert(hotKeyManagerInterface, "BoxComponentMode - could not get HotKeyManagerInterface on RegisterActions.");

        // Dimensions sub-mode
        {
            constexpr AZStd::string_view actionIdentifier = "o3de.action.boxComponentMode.setDimensionsSubMode";
            AzToolsFramework::ActionProperties actionProperties;
            actionProperties.m_name = "Set Dimensions Mode";
            actionProperties.m_description = "Set Dimensions Mode";
            actionProperties.m_category = "Box Component Mode";

            actionManagerInterface->RegisterAction(
                EditorMainWindowActionContextIdentifier,
                actionIdentifier,
                actionProperties,
                []
                {
                    auto componentModeCollectionInterface = AZ::Interface<AzToolsFramework::ComponentModeCollectionInterface>::Get();
                    AZ_Assert(componentModeCollectionInterface, "Could not retrieve component mode collection.");

                    componentModeCollectionInterface->EnumerateActiveComponents(
                        [](const AZ::EntityComponentIdPair& entityComponentIdPair, const AZ::Uuid&)
                        {
                            ShapeComponentModeRequestBus::Event(
                                entityComponentIdPair,
                                &ShapeComponentModeRequests::SetCurrentMode,
                                ShapeComponentModeRequests::SubMode::Dimensions);
                        });
                });

            hotKeyManagerInterface->SetActionHotKey(actionIdentifier, "1");
        }

        // Translation offset sub-mode
        {
            constexpr AZStd::string_view actionIdentifier = "o3de.action.boxComponentMode.setTranslationOffsetSubMode";
            AzToolsFramework::ActionProperties actionProperties;
            actionProperties.m_name = "Set Translation Offset Mode";
            actionProperties.m_description = "Set Translation Offset Mode";
            actionProperties.m_category = "Box Component Mode";

            actionManagerInterface->RegisterAction(
                EditorMainWindowActionContextIdentifier,
                actionIdentifier,
                actionProperties,
                []
                {
                    auto componentModeCollectionInterface = AZ::Interface<AzToolsFramework::ComponentModeCollectionInterface>::Get();
                    AZ_Assert(componentModeCollectionInterface, "Could not retrieve component mode collection.");

                    componentModeCollectionInterface->EnumerateActiveComponents(
                        [](const AZ::EntityComponentIdPair& entityComponentIdPair, const AZ::Uuid&)
                        {
                            ShapeComponentModeRequestBus::Event(
                                entityComponentIdPair,
                                &ShapeComponentModeRequests::SetCurrentMode,
                                ShapeComponentModeRequests::SubMode::TranslationOffset);
                        });
                });

            hotKeyManagerInterface->SetActionHotKey(actionIdentifier, "2");
        }

        // Reset current mode
        {
            constexpr AZStd::string_view actionIdentifier = "o3de.action.boxComponentMode.resetCurrentMode";
            AzToolsFramework::ActionProperties actionProperties;
            actionProperties.m_name = "Reset Current Mode";
            actionProperties.m_description = "Reset Current Mode";
            actionProperties.m_category = "Box Component Mode";

            actionManagerInterface->RegisterAction(
                EditorMainWindowActionContextIdentifier,
                actionIdentifier,
                actionProperties,
                []
                {
                    auto componentModeCollectionInterface = AZ::Interface<AzToolsFramework::ComponentModeCollectionInterface>::Get();
                    AZ_Assert(componentModeCollectionInterface, "Could not retrieve component mode collection.");

                    componentModeCollectionInterface->EnumerateActiveComponents(
                        [](const AZ::EntityComponentIdPair& entityComponentIdPair, const AZ::Uuid&)
                        {
                            ShapeComponentModeRequestBus::Event(entityComponentIdPair, &ShapeComponentModeRequests::ResetCurrentMode);
                        });
                });

            hotKeyManagerInterface->SetActionHotKey(actionIdentifier, "R");
        }
    }

    void BoxComponentMode::BindActionsToModes()
    {
        auto actionManagerInterface = AZ::Interface<AzToolsFramework::ActionManagerInterface>::Get();
        AZ_Assert(actionManagerInterface, "BoxComponentMode - could not get ActionManagerInterface on RegisterActions.");

        AZ::SerializeContext* serializeContext = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationRequests::GetSerializeContext);

        AZStd::string modeIdentifier =
            AZStd::string::format("o3de.context.mode.%s", serializeContext->FindClassData(azrtti_typeid<BoxComponentMode>())->m_name);

        actionManagerInterface->AssignModeToAction(modeIdentifier, "o3de.action.boxComponentMode.setDimensionsSubMode");
        actionManagerInterface->AssignModeToAction(modeIdentifier, "o3de.action.boxComponentMode.setTranslationOffsetSubMode");
        actionManagerInterface->AssignModeToAction(modeIdentifier, "o3de.action.boxComponentMode.resetCurrentMode");
    }

    void BoxComponentMode::BindActionsToMenus()
    {
        auto menuManagerInterface = AZ::Interface<AzToolsFramework::MenuManagerInterface>::Get();
        AZ_Assert(menuManagerInterface, "BoxComponentMode - could not get MenuManagerInterface on BindActionsToMenus.");

        menuManagerInterface->AddActionToMenu(EditMenuIdentifier, "o3de.action.boxComponentMode.setDimensionsSubMode", 6000);
        menuManagerInterface->AddActionToMenu(EditMenuIdentifier, "o3de.action.boxComponentMode.setTranslationOffsetSubMode", 6001);
        menuManagerInterface->AddActionToMenu(EditMenuIdentifier, "o3de.action.boxComponentMode.resetCurrentMode", 6002);
    }

    static ViewportUi::ButtonId RegisterClusterButton(
        AZ::s32 viewportId, ViewportUi::ClusterId clusterId, const char* iconName, const char* tooltip)
    {
        ViewportUi::ButtonId buttonId;
        ViewportUi::ViewportUiRequestBus::EventResult(
            buttonId,
            viewportId,
            &ViewportUi::ViewportUiRequestBus::Events::CreateClusterButton,
            clusterId,
            AZStd::string::format(":/stylesheet/img/UI20/toolbar/%s.svg", iconName));

        ViewportUi::ViewportUiRequestBus::Event(
            viewportId, &ViewportUi::ViewportUiRequestBus::Events::SetClusterButtonTooltip, clusterId, buttonId, tooltip);

        return buttonId;
    }

    BoxComponentMode::~BoxComponentMode()
    {
        ShapeComponentModeRequestBus::Handler::BusDisconnect();
        m_subModes[static_cast<AZ::u32>(m_subMode)]->Teardown();
        if (m_allowAsymmetricalEditing)
        {
            ViewportUi::ViewportUiRequestBus::Event(
                ViewportUi::DefaultViewportId, &ViewportUi::ViewportUiRequestBus::Events::RemoveCluster, m_clusterId);
            m_clusterId = ViewportUi::InvalidClusterId;
        }
    }

    void BoxComponentMode::Refresh()
    {
        m_subModes[static_cast<AZ::u32>(m_subMode)]->UpdateManipulators();
    }

    AZStd::string BoxComponentMode::GetComponentModeName() const
    {
        return "Box Edit Mode";
    }

    AZ::Uuid BoxComponentMode::GetComponentModeType() const
    {
        return azrtti_typeid<BoxComponentMode>();
    }

    void BoxComponentMode::SetupCluster()
    {
        ViewportUi::ViewportUiRequestBus::EventResult(
            m_clusterId,
            ViewportUi::DefaultViewportId,
            &ViewportUi::ViewportUiRequestBus::Events::CreateCluster,
            ViewportUi::Alignment::TopLeft);

        m_buttonIds[static_cast<AZ::u32>(ShapeComponentModeRequests::SubMode::Dimensions)] =
            RegisterClusterButton(ViewportUi::DefaultViewportId, m_clusterId, "Scale", DimensionsTooltip);
        m_buttonIds[static_cast<AZ::u32>(ShapeComponentModeRequests::SubMode::TranslationOffset)] =
            RegisterClusterButton(ViewportUi::DefaultViewportId, m_clusterId, "Move", TranslationOffsetTooltip);

        const auto onButtonClicked = [this](ViewportUi::ButtonId buttonId)
        {
            if (buttonId == m_buttonIds[static_cast<AZ::u32>(ShapeComponentModeRequests::SubMode::Dimensions)])
            {
                SetCurrentMode(ShapeComponentModeRequests::SubMode::Dimensions);
            }
            else if (buttonId == m_buttonIds[static_cast<AZ::u32>(ShapeComponentModeRequests::SubMode::TranslationOffset)])
            {
                SetCurrentMode(ShapeComponentModeRequests::SubMode::TranslationOffset);
            }
        };

        m_modeSelectionHandler = AZ::Event<ViewportUi::ButtonId>::Handler(onButtonClicked);
        ViewportUi::ViewportUiRequestBus::Event(
            ViewportUi::DefaultViewportId,
            &ViewportUi::ViewportUiRequestBus::Events::RegisterClusterEventHandler,
            m_clusterId,
            m_modeSelectionHandler);
    }

    ShapeComponentModeRequests::SubMode BoxComponentMode::GetCurrentMode()
    {
        return m_subMode;
    }

    void BoxComponentMode::SetCurrentMode(ShapeComponentModeRequests::SubMode mode)
    {
        AZ_Assert(mode < ShapeComponentModeRequests::SubMode::NumModes, "Submode not found:%d", static_cast<AZ::u32>(mode));
        m_subModes[static_cast<AZ::u32>(m_subMode)]->Teardown();
        const auto modeIndex = static_cast<AZ::u32>(mode);
        AZ_Assert(modeIndex < m_buttonIds.size(), "Invalid mode index %i.", modeIndex);
        m_subMode = mode;
        m_subModes[modeIndex]->Setup(m_entityComponentIdPair);

        ViewportUi::ViewportUiRequestBus::Event(
            ViewportUi::DefaultViewportId, &ViewportUi::ViewportUiRequestBus::Events::ClearClusterActiveButton, m_clusterId);

        AzToolsFramework::ViewportUi::ViewportUiRequestBus::Event(
            ViewportUi::DefaultViewportId,
            &AzToolsFramework::ViewportUi::ViewportUiRequestBus::Events::SetClusterActiveButton,
            m_clusterId,
            m_buttonIds[modeIndex]);
    }

    void BoxComponentMode::ResetCurrentMode()
    {
        UndoSystem::URSequencePoint* undoBatch = nullptr;
        ToolsApplicationRequests::Bus::BroadcastResult(
            undoBatch, &ToolsApplicationRequests::Bus::Events::BeginUndoBatch, "Reset box component sub mode");
        ToolsApplicationRequests::Bus::Broadcast(
            &ToolsApplicationRequests::Bus::Events::AddDirtyEntity, m_entityComponentIdPair.GetEntityId());
        m_subModes[static_cast<AZ::u32>(m_subMode)]->ResetValues();
        m_subModes[static_cast<AZ::u32>(m_subMode)]->UpdateManipulators();
        AzToolsFramework::ToolsApplicationNotificationBus::Broadcast(
            &AzToolsFramework::ToolsApplicationNotificationBus::Events::InvalidatePropertyDisplay, AzToolsFramework::Refresh_Values);
        ToolsApplicationRequests::Bus::Broadcast(&ToolsApplicationRequests::Bus::Events::EndUndoBatch);
    }

    bool BoxComponentMode::HandleMouseInteraction(const AzToolsFramework::ViewportInteraction::MouseInteractionEvent& mouseInteraction)
    {
        if (mouseInteraction.m_mouseEvent == AzToolsFramework::ViewportInteraction::MouseEvent::Wheel &&
            mouseInteraction.m_mouseInteraction.m_keyboardModifiers.Ctrl())
        {
            const int direction = MouseWheelDelta(mouseInteraction) > 0.0f ? -1 : 1;
            AZ::u32 currentModeIndex = static_cast<AZ::u32>(m_subMode);
            AZ::u32 numSubModes = static_cast<AZ::u32>(ShapeComponentModeRequests::SubMode::NumModes);
            AZ::u32 nextModeIndex = (currentModeIndex + numSubModes + direction) % m_subModes.size();
            ShapeComponentModeRequests::SubMode nextMode = static_cast<ShapeComponentModeRequests::SubMode>(nextModeIndex);
            SetCurrentMode(nextMode);
            return true;
        }
        return false;
    }
} // namespace AzToolsFramework
