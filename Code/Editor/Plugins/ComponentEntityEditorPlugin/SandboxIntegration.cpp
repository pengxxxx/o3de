/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include "SandboxIntegration.h"

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Debug/Profiler.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/RTTI/AttributeReader.h>
#include <AzCore/Slice/SliceComponent.h>
#include <AzCore/std/functional.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/std/numeric.h>
#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Outcome/Outcome.h>
#include <AzCore/Interface/Interface.h>
#include <AzFramework/API/ApplicationAPI.h>
#include <AzFramework/Asset/AssetSystemBus.h>
#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/StringFunc/StringFunc.h>
#include <AzFramework/Visibility/BoundsBus.h>
#include <AzToolsFramework/ActionManager/Action/ActionManagerInterface.h>
#include <AzToolsFramework/ActionManager/Menu/MenuManagerInterface.h>
#include <AzToolsFramework/ActionManager/HotKey/HotKeyManagerInterface.h>
#include <AzToolsFramework/API/ComponentEntityObjectBus.h>
#include <AzToolsFramework/API/EditorAssetSystemAPI.h>
#include <AzToolsFramework/API/EditorEntityAPI.h>
#include <AzToolsFramework/API/EntityCompositionRequestBus.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/AssetBrowser/AssetBrowserBus.h>
#include <AzToolsFramework/AssetBrowser/AssetBrowserEntry.h>
#include <AzToolsFramework/AssetBrowser/AssetSelectionModel.h>
#include <AzToolsFramework/Commands/EntityStateCommand.h>
#include <AzToolsFramework/Commands/SelectionCommand.h>
#include <AzToolsFramework/Commands/SliceDetachEntityCommand.h>
#include <AzToolsFramework/ContainerEntity/ContainerEntityInterface.h>
#include <AzToolsFramework/Editor/ActionManagerIdentifiers/EditorContextIdentifiers.h>
#include <AzToolsFramework/Editor/ActionManagerIdentifiers/EditorMenuIdentifiers.h>
#include <AzToolsFramework/Editor/ActionManagerIdentifiers/EditorActionUpdaterIdentifiers.h>
#include <AzToolsFramework/Editor/ActionManagerUtils.h>
#include <AzToolsFramework/Editor/EditorContextMenuBus.h>
#include <AzToolsFramework/Entity/EditorEntityHelpers.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>
#include <AzToolsFramework/Entity/ReadOnly/ReadOnlyEntityInterface.h>
#include <AzToolsFramework/Entity/SliceEditorEntityOwnershipServiceBus.h>
#include <AzToolsFramework/Slice/SliceRequestBus.h>
#include <AzToolsFramework/Slice/SliceUtilities.h>
#include <AzToolsFramework/ToolsComponents/EditorVisibilityComponent.h>
#include <AzToolsFramework/ToolsComponents/GenericComponentWrapper.h>
#include <AzToolsFramework/Undo/UndoSystem.h>
#include <AzToolsFramework/UI/EditorEntityUi/EditorEntityUiInterface.h>
#include <AzToolsFramework/UI/Prefab/PrefabIntegrationInterface.h>
#include <AzToolsFramework/UI/PropertyEditor/InstanceDataHierarchy.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include <AzToolsFramework/UI/PropertyEditor/EntityPropertyEditor.hxx>
#include <AzToolsFramework/ViewportSelection/EditorHelpers.h>
#include <AzToolsFramework/ViewportSelection/EditorSelectionUtil.h>
#include <MathConversion.h>

#include <Atom/ImageProcessing/ImageProcessingDefines.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <AtomToolsFramework/Viewport/ModularViewportCameraControllerRequestBus.h>

#include "ISourceControl.h"
#include "UI/QComponentEntityEditorMainWindow.h"

#include <LmbrCentral/Scripting/TagComponentBus.h>
#include <LmbrCentral/Scripting/EditorTagComponentBus.h>

// Sandbox imports.
#include <Editor/CryEditDoc.h>
#include <Editor/GameEngine.h>
#include <Editor/DisplaySettings.h>
#include <Editor/Settings.h>
#include <Editor/QtViewPaneManager.h>
#include <Editor/EditorViewportSettings.h>
#include <Editor/EditorViewportCamera.h>
#include <Editor/Util/PathUtil.h>
#include <Editor/Objects/EntityObject.h>
#include "CryEdit.h"
#include "Undo/Undo.h"

#include <QMenu>
#include <QAction>
#include <QWidgetAction>
#include <QHBoxLayout>
#include "MainWindow.h"

#include "Include/IObjectManager.h"

#include <AzCore/std/algorithm.h>

#ifdef CreateDirectory
#undef CreateDirectory
#endif

//////////////////////////////////////////////////////////////////////////
// Gathers all selected entities, culling any that have an ancestor in the selection.
void GetSelectedEntitiesSetWithFlattenedHierarchy(AzToolsFramework::EntityIdSet& out)
{
    AzToolsFramework::EntityIdList entities;
    AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(
        entities,
        &AzToolsFramework::ToolsApplicationRequests::GetSelectedEntities);

    for (const AZ::EntityId& entityId : entities)
    {
        bool selectionIncludesTransformHeritage = false;
        AZ::EntityId parent = entityId;
        do
        {
            AZ::EntityId nextParentId;
            AZ::TransformBus::EventResult(
                /*result*/ nextParentId,
                /*address*/ parent,
                &AZ::TransformBus::Events::GetParentId);
            parent = nextParentId;
            if (!parent.IsValid())
            {
                break;
            }
            for (const AZ::EntityId& parentCheck : entities)
            {
                if (parentCheck == parent)
                {
                    selectionIncludesTransformHeritage = true;
                    break;
                }
            }
        } while (parent.IsValid() && !selectionIncludesTransformHeritage);

        if (!selectionIncludesTransformHeritage)
        {
            out.insert(entityId);
        }
    }
}

SandboxIntegrationManager::SandboxIntegrationManager()
    : m_startedUndoRecordingNestingLevel(0)
    , m_dc(nullptr)
    , m_notificationWindowManager(new AzToolsFramework::SliceOverridesNotificationWindowManager())
{
    // Required to receive events from the Cry Engine undo system
    GetIEditor()->GetUndoManager()->AddListener(this);

    // Only create the PrefabIntegrationManager if prefabs are enabled
    bool prefabSystemEnabled = false;
    AzFramework::ApplicationRequests::Bus::BroadcastResult(
        prefabSystemEnabled, &AzFramework::ApplicationRequests::IsPrefabSystemEnabled);
    if (prefabSystemEnabled)
    {
        m_prefabIntegrationManager = aznew AzToolsFramework::Prefab::PrefabIntegrationManager();
    }

    AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler::BusConnect();
}

SandboxIntegrationManager::~SandboxIntegrationManager()
{
    AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler::BusDisconnect();

    GetIEditor()->GetUndoManager()->RemoveListener(this);

    delete m_prefabIntegrationManager;
    m_prefabIntegrationManager = nullptr;
}

void SandboxIntegrationManager::Setup()
{
    AzFramework::AssetCatalogEventBus::Handler::BusConnect();
    AzToolsFramework::ToolsApplicationEvents::Bus::Handler::BusConnect();
    AzToolsFramework::EditorRequests::Bus::Handler::BusConnect();
    AzToolsFramework::EditorWindowRequests::Bus::Handler::BusConnect();
    AzToolsFramework::EditorContextMenuBus::Handler::BusConnect();
    AzToolsFramework::EditorEntityContextNotificationBus::Handler::BusConnect();

    AzFramework::DisplayContextRequestBus::Handler::BusConnect();

    // Keep a reference to the interface EditorEntityUiInterface.
    // This is used to register layer entities to their UI handler when the layer component is activated.
    m_editorEntityUiInterface = AZ::Interface<AzToolsFramework::EditorEntityUiInterface>::Get();

    AZ_Assert((m_editorEntityUiInterface != nullptr),
        "SandboxIntegrationManager requires a EditorEntityUiInterface instance to be present on Setup().");

    bool prefabSystemEnabled = false;
    AzFramework::ApplicationRequests::Bus::BroadcastResult(
        prefabSystemEnabled, &AzFramework::ApplicationRequests::IsPrefabSystemEnabled);
    if (prefabSystemEnabled)
    {
        m_prefabIntegrationInterface = AZ::Interface<AzToolsFramework::Prefab::PrefabIntegrationInterface>::Get();
        AZ_Assert(
            (m_prefabIntegrationInterface != nullptr),
            "SandboxIntegrationManager requires a PrefabIntegrationInterface instance to be present on Setup().");
    }

    m_editorEntityAPI = AZ::Interface<AzToolsFramework::EditorEntityAPI>::Get();
    AZ_Assert(m_editorEntityAPI, "SandboxIntegrationManager requires an EditorEntityAPI instance to be present on Setup().");

    m_readOnlyEntityPublicInterface = AZ::Interface<AzToolsFramework::ReadOnlyEntityPublicInterface>::Get();
    AZ_Assert(m_readOnlyEntityPublicInterface, "SandboxIntegrationManager requires an ReadOnlyEntityPublicInterface instance to be present on Setup().");

    m_contextMenuBottomHandler.Setup();
}

void SandboxIntegrationManager::SaveSlice(const bool& QuickPushToFirstLevel)
{
    AzToolsFramework::EntityIdList selectedEntities;
    GetSelectedEntities(selectedEntities);
    if (selectedEntities.size() == 0)
    {
        m_notificationWindowManager->CreateNotificationWindow(AzToolsFramework::SliceOverridesNotificationWindow::EType::TypeError, "Nothing selected - Select a slice entity with overrides and try again");
        return;
    }

    AzToolsFramework::EntityIdList relevantEntities;
    AZ::u32 entitiesInSlices;
    AZStd::vector<AZ::SliceComponent::SliceInstanceAddress> sliceInstances;
    GetEntitiesInSlices(selectedEntities, entitiesInSlices, sliceInstances);
    if (entitiesInSlices > 0)
    {
        // Gather the set of relevant entities from the selected entities and all descendants
        AzToolsFramework::EntityIdSet relevantEntitiesSet;
        AzToolsFramework::ToolsApplicationRequestBus::BroadcastResult(relevantEntitiesSet,
            &AzToolsFramework::ToolsApplicationRequestBus::Events::GatherEntitiesAndAllDescendents, selectedEntities);

        relevantEntities.reserve(relevantEntitiesSet.size());
        for (AZ::EntityId& id : relevantEntitiesSet)
        {
            relevantEntities.push_back(id);
        }
    }

    int numEntitiesToAdd = 0;
    int numEntitiesToRemove = 0;
    int numEntitiesToUpdate = 0;
    if (AzToolsFramework::SliceUtilities::SaveSlice(relevantEntities, numEntitiesToAdd, numEntitiesToRemove, numEntitiesToUpdate, QuickPushToFirstLevel))
    {
        if (numEntitiesToAdd > 0 || numEntitiesToRemove > 0 || numEntitiesToUpdate > 0)
        {
            m_notificationWindowManager->CreateNotificationWindow(
                AzToolsFramework::SliceOverridesNotificationWindow::EType::TypeSuccess,
                QString("Save slice to parent - %1 saved successfully").arg(numEntitiesToUpdate + numEntitiesToAdd + numEntitiesToRemove));
        }
        else
        {
            m_notificationWindowManager->CreateNotificationWindow(
                AzToolsFramework::SliceOverridesNotificationWindow::EType::TypeError,
                "Selected has no overrides - Select a slice entity with overrides and try again");
        }
    }
    else
    {
        m_notificationWindowManager->CreateNotificationWindow(
            AzToolsFramework::SliceOverridesNotificationWindow::EType::TypeError,
            "Save slice to parent - Failed");
    }
}

// This event handler is queued on main thread.
void SandboxIntegrationManager::OnCatalogAssetAdded(const AZ::Data::AssetId& assetId)
{
    bool prefabSystemEnabled = false;
    AzFramework::ApplicationRequests::Bus::BroadcastResult(
        prefabSystemEnabled, &AzFramework::ApplicationRequests::IsPrefabSystemEnabled);

    if (!prefabSystemEnabled)
    {
        AZ::SliceComponent* editorRootSlice = nullptr;
        AzToolsFramework::SliceEditorEntityOwnershipServiceRequestBus::BroadcastResult(
            editorRootSlice, &AzToolsFramework::SliceEditorEntityOwnershipServiceRequestBus::Events::GetEditorRootSlice);
        AZ_Assert(editorRootSlice, "Editor root slice missing!");

        for (auto restoreItr = m_sliceAssetDeletionErrorRestoreInfos.begin(); restoreItr != m_sliceAssetDeletionErrorRestoreInfos.end();)
        {
            if (restoreItr->m_assetId == assetId)
            {
                for (const auto& entityRestore : restoreItr->m_entityRestoreInfos)
                {
                    const AZ::EntityId& entityId = entityRestore.first;
                    const AZ::SliceComponent::EntityRestoreInfo& restoreInfo = entityRestore.second;

                    AZ::Entity* entity = editorRootSlice->FindEntity(entityId);
                    if (entity)
                    {
                        AzToolsFramework::SliceEditorEntityOwnershipServiceRequestBus::Broadcast(
                            &AzToolsFramework::SliceEditorEntityOwnershipServiceRequestBus::Events::RestoreSliceEntity, entity, restoreInfo,
                            AzToolsFramework::SliceEntityRestoreType::Detached);
                    }
                    else
                    {
                        AZ_Error(
                            "DetachSliceEntity", entity, "Unable to find previous detached entity of Id %s. Cannot undo \"Detach\" action.",
                            entityId.ToString().c_str());
                    }
                }

                restoreItr = m_sliceAssetDeletionErrorRestoreInfos.erase(restoreItr);
            }
            else
            {
                restoreItr++;
            }
        }
    }
}

// No mutex is used for now because the only 
// operation writing to shared resource is queued on main thread.
void SandboxIntegrationManager::OnCatalogAssetRemoved(const AZ::Data::AssetId& assetId, const AZ::Data::AssetInfo& assetInfo)
{
    bool isPrefabSystemEnabled = false;
    AzFramework::ApplicationRequests::Bus::BroadcastResult(isPrefabSystemEnabled, &AzFramework::ApplicationRequests::IsPrefabSystemEnabled);

    // Check to see if the removed slice asset has any instance in the level, then check if 
    // those dangling instances are directly under the root slice (not sub-slices). If yes,
    // detach them and save necessary information so they can be restored when their slice asset
    // comes back.

    if (!isPrefabSystemEnabled && assetInfo.m_assetType == AZ::AzTypeInfo<AZ::SliceAsset>::Uuid())
    {
        AZ::SliceComponent* rootSlice = nullptr;
        AzToolsFramework::SliceEditorEntityOwnershipServiceRequestBus::BroadcastResult(rootSlice,
            &AzToolsFramework::SliceEditorEntityOwnershipServiceRequestBus::Events::GetEditorRootSlice);
        AZ_Assert(rootSlice, "Editor root slice missing!");

        AZStd::vector<AZ::EntityId> entitiesToDetach;
        const AZ::SliceComponent::SliceList& subSlices = rootSlice->GetSlices();
        for (const AZ::SliceComponent::SliceReference& subSliceRef : subSlices)
        {
            if (subSliceRef.GetSliceAsset().GetId() == assetId)
            {
                for (const AZ::SliceComponent::SliceInstance& sliceInst : subSliceRef.GetInstances())
                {
                    const AZ::SliceComponent::InstantiatedContainer* instContainer = sliceInst.GetInstantiated();
                    for (AZ::Entity* entity : instContainer->m_entities)
                    {
                        entitiesToDetach.emplace_back(entity->GetId());
                    }
                }
            }
        }

        AZ_Error("Editor", false, "The slice asset %s is deleted from disk, to prevent further data corruption, all of its root level slice instances are detached. "
            "Restoring the slice asset on disk will revert the detaching operation.", assetInfo.m_relativePath.c_str());

        AZ::SystemTickBus::QueueFunction([this, assetId, entitiesToDetach]() {
            AZStd::vector<AZStd::pair<AZ::EntityId, AZ::SliceComponent::EntityRestoreInfo>> restoreInfos;
            bool detachSuccess = false;
            AzToolsFramework::ToolsApplicationRequestBus::BroadcastResult(
                detachSuccess, &AzToolsFramework::ToolsApplicationRequestBus::Events::DetachEntities, entitiesToDetach, restoreInfos);
            if (detachSuccess)
            {
                m_sliceAssetDeletionErrorRestoreInfos.emplace_back(assetId, AZStd::move(restoreInfos));
            }
        });
    }
}

void SandboxIntegrationManager::GetEntitiesInSlices(
    const AzToolsFramework::EntityIdList& selectedEntities,
    AZ::u32& entitiesInSlices,
    AZStd::vector<AZ::SliceComponent::SliceInstanceAddress>& sliceInstances)
{
    // Identify all slice instances affected by the selected entity set.
    entitiesInSlices = 0;
    for (const AZ::EntityId& entityId : selectedEntities)
    {
        AZ::SliceComponent::SliceInstanceAddress sliceAddress;
        AzFramework::SliceEntityRequestBus::EventResult(sliceAddress, entityId,
            &AzFramework::SliceEntityRequestBus::Events::GetOwningSlice);

        if (sliceAddress.IsValid())
        {
            ++entitiesInSlices;

            if (sliceInstances.end() == AZStd::find(sliceInstances.begin(), sliceInstances.end(), sliceAddress))
            {
                sliceInstances.push_back(sliceAddress);
            }
        }
    }
}

void SandboxIntegrationManager::Teardown()
{
    m_contextMenuBottomHandler.Teardown();

    AzFramework::DisplayContextRequestBus::Handler::BusDisconnect();
    AzToolsFramework::EditorEntityContextNotificationBus::Handler::BusDisconnect();
    AzToolsFramework::EditorContextMenuBus::Handler::BusDisconnect();
    AzToolsFramework::EditorWindowRequests::Bus::Handler::BusDisconnect();
    AzToolsFramework::EditorRequests::Bus::Handler::BusDisconnect();
    AzToolsFramework::ToolsApplicationEvents::Bus::Handler::BusDisconnect();
}

void SandboxIntegrationManager::SetDC(DisplayContext* dc)
{
    m_dc = dc;
}

DisplayContext* SandboxIntegrationManager::GetDC()
{
    return m_dc;
}

void SandboxIntegrationManager::OnBeginUndo([[maybe_unused]] const char* label)
{
    AzToolsFramework::UndoSystem::URSequencePoint* currentBatch = nullptr;
    AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(
        currentBatch, &AzToolsFramework::ToolsApplicationRequests::Bus::Events::GetCurrentUndoBatch);

    AZ_Assert(currentBatch, "No undo batch is active.");

    // Only generate a Sandbox placeholder for root-level undo batches.
    if (nullptr == currentBatch->GetParent())
    {
        // start Cry Undo
        if (!CUndo::IsRecording())
        {
            GetIEditor()->BeginUndo();
            // flag that we started recording the undo batch
            m_startedUndoRecordingNestingLevel = 1;
        }

    }
    else
    {
        if (m_startedUndoRecordingNestingLevel)
        {
            // if we previously started recording the undo, increment the nesting level so we can
            // detect when we need to accept the undo in OnEndUndo()
            m_startedUndoRecordingNestingLevel++;
        }
    }
}

void SandboxIntegrationManager::OnEndUndo(const char* label, bool changed)
{
    // Add the undo only after we know it's got a legit change, we can't remove undos from the cry undo system so we do it here instead of OnBeginUndo
    if (changed && CUndo::IsRecording())
    {
        CUndo::Record(new CToolsApplicationUndoLink());
    }
    if (m_startedUndoRecordingNestingLevel)
    {
        m_startedUndoRecordingNestingLevel--;
        if (m_startedUndoRecordingNestingLevel == 0)
        {
            if (changed)
            {
                // only accept the undo batch that we initially started undo recording on
                GetIEditor()->AcceptUndo(label);
            }
            else
            {
                GetIEditor()->CancelUndo();
            }
        }
    }
}

int SandboxIntegrationManager::GetMenuPosition() const
{
    return aznumeric_cast<int>(AzToolsFramework::EditorContextMenuOrdering::TOP);
}

void SandboxIntegrationManager::PopulateEditorGlobalContextMenu(
    QMenu* menu, const AZStd::optional<AzFramework::ScreenPoint>& point, int flags)
{
    if (!IsLevelDocumentOpen())
    {
        return;
    }

    if (flags & AzToolsFramework::EditorEvents::eECMF_USE_VIEWPORT_CENTER)
    {
        int width = 0;
        int height = 0;
        // If there is no 3D Viewport active to aid in the positioning of context menu
        // operations, we don't need to store anything but default values here. Any code
        // using these numbers for placement should default to the origin when there's
        // no 3D viewport to raycast into.
        if (const CViewport* view = GetIEditor()->GetViewManager()->GetGameViewport())
        {
            view->GetDimensions(&width, &height);
        }

        m_contextMenuViewPoint = AzFramework::ScreenPoint{ width / 2, height / 2 };
    }
    else
    {
        m_contextMenuViewPoint = point;
    }

    CGameEngine* gameEngine = GetIEditor()->GetGameEngine();
    if (!gameEngine || !gameEngine->IsLevelLoaded())
    {
        return;
    }

    menu->setToolTipsVisible(true);

    AzToolsFramework::EntityIdList selected;
    GetSelectedOrHighlightedEntities(selected);

    bool prefabSystemEnabled = false;
    AzFramework::ApplicationRequests::Bus::BroadcastResult(prefabSystemEnabled, &AzFramework::ApplicationRequests::IsPrefabSystemEnabled);

    QAction* action = nullptr;

    // when nothing is selected, entity is created at root level
    if (selected.size() == 0)
    {
        action = menu->addAction(QObject::tr("Create entity"));
        action->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_N));
        QObject::connect(
            action, &QAction::triggered, action,
            [this]
            {
                ContextMenu_NewEntity();
            }
        );
    }
    // when a single entity is selected, entity is created as its child
    else if (selected.size() == 1)
    {
        AZ::EntityId selectedEntityId = selected.front();
        bool selectedEntityIsReadOnly = m_readOnlyEntityPublicInterface->IsReadOnly(selectedEntityId);
        auto containerEntityInterface = AZ::Interface<AzToolsFramework::ContainerEntityInterface>::Get();
        if (!prefabSystemEnabled || (containerEntityInterface && containerEntityInterface->IsContainerOpen(selectedEntityId) && !selectedEntityIsReadOnly))
        {
            action = menu->addAction(QObject::tr("Create entity"));
            action->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_N));
            QObject::connect(
                action, &QAction::triggered, action,
                [selectedEntityId]
                {
                    AzToolsFramework::EditorRequestBus::Broadcast(&AzToolsFramework::EditorRequestBus::Handler::CreateNewEntityAsChild, selectedEntityId);
                }
            );
        }
    }

    if (!prefabSystemEnabled)
    {
        menu->addSeparator();

        AzToolsFramework::EntityIdList entities;
        AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(
            entities,
            &AzToolsFramework::ToolsApplicationRequests::GetSelectedEntities);

        AzToolsFramework::EntityIdSet flattenedSelection = AzToolsFramework::GetCulledEntityHierarchy(entities);

        SetupSliceContextMenu(menu);

        if (!selected.empty())
        {
            // Don't allow duplication if any of the selected entities are direct descendants of a read-only entity
            bool selectionContainsDescendantOfReadOnlyEntity = false;
            for (const auto& entityId : selected)
            {
                AZ::EntityId parentEntityId;
                AZ::TransformBus::EventResult(parentEntityId, entityId, &AZ::TransformBus::Events::GetParentId);

                if (parentEntityId.IsValid() && m_readOnlyEntityPublicInterface->IsReadOnly(parentEntityId))
                {
                    selectionContainsDescendantOfReadOnlyEntity = true;
                    break;
                }
            }

            if (!selectionContainsDescendantOfReadOnlyEntity)
            {
                action = menu->addAction(QObject::tr("Duplicate"));
                QObject::connect(action, &QAction::triggered, action, [this] { ContextMenu_Duplicate(); });
            }
        }

        action = menu->addAction(QObject::tr("Delete"));
        QObject::connect(action, &QAction::triggered, action, [this] { ContextMenu_DeleteSelected(); });
        if (selected.size() == 0)
        {
            action->setDisabled(true);
        }
    }

    menu->addSeparator();

    if (selected.size() > 0)
    {
        action = menu->addAction(QObject::tr("Find in Entity Outliner"));
        QObject::connect(
            action, &QAction::triggered,
            [selected]
            {
                AzToolsFramework::EditorEntityContextNotificationBus::Broadcast(
                    &EditorEntityContextNotification::OnFocusInEntityOutliner, selected);
            });
        menu->addSeparator();
    }
}

void SandboxIntegrationManager::OpenPinnedInspector(const AzToolsFramework::EntityIdSet& entities)
{
    QDockWidget* dockWidget = InstanceViewPane(LyViewPane::EntityInspectorPinned);
    if (dockWidget)
    {
        QComponentEntityEditorInspectorWindow* editor = static_cast<QComponentEntityEditorInspectorWindow*>(dockWidget->widget());
        if (editor && editor->GetPropertyEditor())
        {
            editor->GetPropertyEditor()->SetOverrideEntityIds(entities);

            AZStd::string widgetTitle;
            if (entities.size() == 1)
            {
                AZStd::string entityName;
                AZ::ComponentApplicationBus::BroadcastResult(entityName, &AZ::ComponentApplicationBus::Events::GetEntityName, *entities.begin());
                widgetTitle = AZStd::string::format("%s Inspector", entityName.c_str());

                QObject::connect(editor->GetPropertyEditor(), &AzToolsFramework::EntityPropertyEditor::SelectedEntityNameChanged, [dockWidget](const AZ::EntityId& /*entityId*/, const AZStd::string& name)
                {
                    AZStd::string newTitle = AZStd::string::format("%s Inspector", name.c_str());
                    dockWidget->setWindowTitle(newTitle.c_str());
                });
            }
            else
            {
                widgetTitle = AZStd::string::format("%zu Entities - Inspector", entities.size());
            }

            dockWidget->setWindowTitle(widgetTitle.c_str());
        }
    }
}

void SandboxIntegrationManager::ClosePinnedInspector(AzToolsFramework::EntityPropertyEditor* editor)
{
    QWidget* currentWidget = editor->parentWidget();
    while (currentWidget)
    {
        QDockWidget* dockWidget = qobject_cast<QDockWidget*>(currentWidget);
        if (dockWidget)
        {
            QtViewPaneManager::instance()->ClosePaneInstance(LyViewPane::EntityInspectorPinned, dockWidget);
            return;
        }
        currentWidget = currentWidget->parentWidget();
    }
}

void SandboxIntegrationManager::SetupSliceContextMenu(QMenu* menu)
{
    AZ_PROFILE_FUNCTION(Editor);
    AzToolsFramework::EntityIdList selectedEntities;
    GetSelectedOrHighlightedEntities(selectedEntities);

    menu->addSeparator();

    if (!selectedEntities.empty())
    {
        bool anySelectedEntityFromExistingSlice = false;
        for (AZ::EntityId entityId : selectedEntities)
        {
            if (!anySelectedEntityFromExistingSlice)
            {
                AZ::SliceComponent::SliceInstanceAddress sliceAddress(nullptr, nullptr);
                AzFramework::SliceEntityRequestBus::EventResult(sliceAddress, entityId,
                    &AzFramework::SliceEntityRequestBus::Events::GetOwningSlice);
                if (sliceAddress.GetReference())
                {
                    anySelectedEntityFromExistingSlice = true;
                }
            }
        }
    }

    QAction* instantiateAction = menu->addAction(QObject::tr("Instantiate slice..."));
    instantiateAction->setToolTip(QObject::tr("Instantiates a pre-existing slice asset into the level"));
    QObject::connect(instantiateAction, &QAction::triggered, instantiateAction, [this] { ContextMenu_InstantiateSlice(); });

    AzToolsFramework::EditorEvents::Bus::Broadcast(&AzToolsFramework::EditorEvents::Bus::Events::PopulateEditorGlobalContextMenu_SliceSection, menu, AZ::Vector2::CreateZero(),
        AzToolsFramework::EditorEvents::eECMF_HIDE_ENTITY_CREATION | AzToolsFramework::EditorEvents::eECMF_USE_VIEWPORT_CENTER);

    if (selectedEntities.empty())
    {
        return;
    }

    AZ::u32 entitiesInSlices;
    AZStd::vector<AZ::SliceComponent::SliceInstanceAddress> sliceInstances;
    GetEntitiesInSlices(selectedEntities, entitiesInSlices, sliceInstances);
    // Offer slice-related options if any selected entities belong to slice instances.
    if (0 == entitiesInSlices)
    {
        return;
    }

    // Setup push and revert options (quick push and 'advanced' push UI).
    SetupSliceContextMenu_Modify(menu, selectedEntities, entitiesInSlices);

    menu->addSeparator();

    // PopulateFindSliceMenu takes a callback to run when a slice is selected, which is called before the slice is selected in the asset browser.
    // This is so the AssetBrowser can be opened first, which can only be done from a Sandbox module. PopulateFindSliceMenu exists in the AzToolsFramework
    // module in SliceUtilities, so it can share logic with similar menus, like Quick Push.
    // Similarly, it takes a callback for the SliceRelationshipView.
    AzToolsFramework::SliceUtilities::PopulateSliceSubMenus(*menu, selectedEntities,
        []
    {
        // This will open the AssetBrowser if it's not open, and do nothing if it's already opened.
        QtViewPaneManager::instance()->OpenPane(LyViewPane::AssetBrowser);
    },
        []
    {
        //open SliceRelationshipView if necessary, and populate it
        QtViewPaneManager::instance()->OpenPane(LyViewPane::SliceRelationships);
    });
}

void SandboxIntegrationManager::SetupSliceContextMenu_Modify(QMenu* menu, const AzToolsFramework::EntityIdList& selectedEntities, [[maybe_unused]] const AZ::u32 numEntitiesInSlices)
{
    AZ_PROFILE_FUNCTION(Editor);
    using namespace AzToolsFramework;

    // Gather the set of relevant entities from the selected entities and all descendants
    AzToolsFramework::EntityIdSet relevantEntitiesSet;
    AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(
        relevantEntitiesSet, &AzToolsFramework::ToolsApplicationRequests::Bus::Events::GatherEntitiesAndAllDescendents, selectedEntities);
    AzToolsFramework::EntityIdList relevantEntities;
    relevantEntities.reserve(relevantEntitiesSet.size());
    for (AZ::EntityId& id : relevantEntitiesSet)
    {
        relevantEntities.push_back(id);
    }

    SliceUtilities::PopulateQuickPushMenu(*menu, relevantEntities);

    SliceUtilities::PopulateDetachMenu(*menu, selectedEntities, relevantEntitiesSet);

    bool canRevert = false;

    for (AZ::EntityId id : relevantEntitiesSet)
    {
        bool entityHasOverrides = false;
        AzToolsFramework::EditorEntityInfoRequestBus::EventResult(entityHasOverrides, id, &AzToolsFramework::EditorEntityInfoRequestBus::Events::HasSliceEntityOverrides);
        if (entityHasOverrides)
        {
            canRevert = true;
            break;
        }
    }

    QAction* revertAction = menu->addAction(QObject::tr("Revert overrides"));
    revertAction->setToolTip(QObject::tr("Revert all slice entities and their children to their last saved state."));

    QObject::connect(revertAction, &QAction::triggered, [this, relevantEntities]
    {
        ContextMenu_ResetToSliceDefaults(relevantEntities);
    });

    revertAction->setEnabled(canRevert);
}

void SandboxIntegrationManager::CreateEditorRepresentation(AZ::Entity* entity)
{
    // Add required editor components to the entity.
    AzToolsFramework::EditorEntityContextRequestBus::Broadcast(
        &AzToolsFramework::EditorEntityContextRequestBus::Events::AddRequiredComponents, *entity);
}

//////////////////////////////////////////////////////////////////////////
void SandboxIntegrationManager::GoToSelectedOrHighlightedEntitiesInViewports()
{
    AzToolsFramework::EntityIdList entityIds;
    GetSelectedOrHighlightedEntities(entityIds);
    GoToEntitiesInViewports(entityIds);
}

//////////////////////////////////////////////////////////////////////////
void SandboxIntegrationManager::GoToSelectedEntitiesInViewports()
{
    AzToolsFramework::EntityIdList entityIds;
    GetSelectedEntities(entityIds);
    GoToEntitiesInViewports(entityIds);
}

bool SandboxIntegrationManager::CanGoToSelectedEntitiesInViewports()
{
    AzToolsFramework::EntityIdList entityIds;
    GetSelectedEntities(entityIds);

    if (entityIds.size() == 0)
    {
        return false;
    }

    for (const AZ::EntityId& entityId : entityIds)
    {
        if (CanGoToEntityOrChildren(entityId))
        {
            return true;
        }
    }

    return false;
}

bool SandboxIntegrationManager::CanGoToEntityOrChildren(const AZ::EntityId& entityId) const
{
    AZ::Entity* entity = nullptr;
    AZ::ComponentApplicationBus::BroadcastResult(entity, &AZ::ComponentApplicationRequests::FindEntity, entityId);
    if (entity == nullptr)
    {
        return false;
    }

    // Skip if this entity doesn't have a transform;
    // UI entities and system components don't have transforms and thus aren't visible in the Editor viewport
    return entity->GetTransform() != nullptr;
}

AZ::Vector3 SandboxIntegrationManager::GetWorldPositionAtViewportCenter()
{
    if (GetIEditor() && GetIEditor()->GetViewManager())
    {
        CViewport* view = GetIEditor()->GetViewManager()->GetGameViewport();
        if (view)
        {
            int width, height;
            view->GetDimensions(&width, &height);
            return LYVec3ToAZVec3(view->ViewToWorld(QPoint(width / 2, height / 2)));
        }
    }

    return AZ::Vector3::CreateZero();
}

void SandboxIntegrationManager::ClearRedoStack()
{
    // We have two separate undo systems that are assumed to be kept in sync,
    // So here we tell the legacy Undo system to clear the redo stack and at the same time
    // tell the new AZ undo system to clear redo stack ("slice" the stack)

    // Clear legacy redo stack
    GetIEditor()->ClearRedoStack();

    // Clear AZ redo stack
    AzToolsFramework::UndoSystem::UndoStack* undoStack = nullptr;
    AzToolsFramework::ToolsApplicationRequestBus::BroadcastResult(undoStack, &AzToolsFramework::ToolsApplicationRequestBus::Events::GetUndoStack);
    if (undoStack)
    {
        undoStack->Slice();
    }
}

//////////////////////////////////////////////////////////////////////////

void SandboxIntegrationManager::CloneSelection(bool& handled)
{
    AZ_PROFILE_FUNCTION(AzToolsFramework);

    AzToolsFramework::EntityIdList entities;
    AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(
        entities,
        &AzToolsFramework::ToolsApplicationRequests::GetSelectedEntities);

    AzToolsFramework::EntityIdSet duplicationSet = AzToolsFramework::GetCulledEntityHierarchy(entities);

    if (!duplicationSet.empty())
    {
        m_editorEntityAPI->DuplicateSelected();
        handled = true;
    }
    else
    {
        handled = false;
    }
}

void SandboxIntegrationManager::DeleteSelectedEntities([[maybe_unused]] const bool includeDescendants)
{
    AzToolsFramework::EntityIdList selectedEntityIds;
    AzToolsFramework::ToolsApplicationRequestBus::BroadcastResult(
        selectedEntityIds, &AzToolsFramework::ToolsApplicationRequests::GetSelectedEntities);

    AzToolsFramework::ToolsApplicationRequestBus::Broadcast(
        &AzToolsFramework::ToolsApplicationRequests::DeleteEntitiesAndAllDescendants, selectedEntityIds);
}

AZ::EntityId SandboxIntegrationManager::CreateNewEntity(AZ::EntityId parentId)
{
    AZ::Vector3 position = AZ::Vector3::CreateZero();

    // If we have an invalid parent, base new entity's position on the viewport.
    if (!parentId.IsValid())
    {
        position = GetWorldPositionAtViewportCenter();
    }
    return CreateNewEntityAtPosition(position, parentId);
}

AZ::EntityId SandboxIntegrationManager::CreateNewEntityAsChild(AZ::EntityId parentId)
{
    AZ_Assert(parentId.IsValid(), "Entity created as a child of an invalid parent entity.");
    auto newEntityId = CreateNewEntity(parentId);

    // Some modules need to know that this special action has taken place; broadcast an event.
    ToolsApplicationEvents::Bus::Broadcast(&ToolsApplicationEvents::EntityCreatedAsChild, newEntityId, parentId);
    return newEntityId;
}

AZ::EntityId SandboxIntegrationManager::CreateNewEntityAtPosition(const AZ::Vector3& pos, AZ::EntityId parentId)
{
    using namespace AzToolsFramework;

    bool prefabSystemEnabled = false;
    AzFramework::ApplicationRequests::Bus::BroadcastResult(prefabSystemEnabled, &AzFramework::ApplicationRequests::IsPrefabSystemEnabled);

    AZ::EntityId newEntityId;

    if (!prefabSystemEnabled)
    {
        const AZStd::string name = AZStd::string::format("Entity%d", GetIEditor()->GetObjectManager()->GetObjectCount() + 1);
        EditorEntityContextRequestBus::BroadcastResult(newEntityId, &EditorEntityContextRequests::CreateNewEditorEntity, name.c_str());

        if (newEntityId.IsValid())
        {
            AZ::Transform transform = AZ::Transform::CreateIdentity();
            transform.SetTranslation(pos);
            if (parentId.IsValid())
            {
                AZ::TransformBus::Event(newEntityId, &AZ::TransformInterface::SetParent, parentId);
                AZ::TransformBus::Event(newEntityId, &AZ::TransformInterface::SetLocalTM, transform);
            }
            else
            {
                AZ::TransformBus::Event(newEntityId, &AZ::TransformInterface::SetWorldTM, transform);
            }

            // Select the new entity (and deselect others).
            AzToolsFramework::EntityIdList selection = {newEntityId};

            ScopedUndoBatch undo("New Entity");
            auto selectionCommand = AZStd::make_unique<AzToolsFramework::SelectionCommand>(selection, "");
            selectionCommand->SetParent(undo.GetUndoBatch());
            selectionCommand.release();

            AzToolsFramework::ToolsApplicationRequests::Bus::Broadcast(
                &AzToolsFramework::ToolsApplicationRequests::Bus::Events::SetSelectedEntities, selection);
        }
    }
    else
    {
        newEntityId =  m_prefabIntegrationInterface->CreateNewEntityAtPosition(pos, parentId);
    }
    return newEntityId;
}

AzFramework::EntityContextId SandboxIntegrationManager::GetEntityContextId()
{
    AzFramework::EntityContextId editorEntityContextId = AzFramework::EntityContextId::CreateNull();
    AzToolsFramework::EditorEntityContextRequestBus::BroadcastResult(editorEntityContextId, &AzToolsFramework::EditorEntityContextRequestBus::Events::GetEditorEntityContextId);

    return editorEntityContextId;
}

//////////////////////////////////////////////////////////////////////////
QWidget* SandboxIntegrationManager::GetAppMainWindow()
{
    return MainWindow::instance();
}

QWidget* SandboxIntegrationManager::GetMainWindow()
{
    return MainWindow::instance();
}

IEditor* SandboxIntegrationManager::GetEditor()
{
    return GetIEditor();
}

//////////////////////////////////////////////////////////////////////////
bool SandboxIntegrationManager::GetUndoSliceOverrideSaveValue()
{
    return false;
}

//////////////////////////////////////////////////////////////////////////
bool SandboxIntegrationManager::GetShowCircularDependencyError()
{
    return true;
}

void SandboxIntegrationManager::SetShowCircularDependencyError(const bool& /* showCircularDependencyError */)
{
}

//////////////////////////////////////////////////////////////////////////
void SandboxIntegrationManager::LaunchLuaEditor(const char* files)
{
    CCryEditApp::instance()->OpenLUAEditor(files);
}

bool SandboxIntegrationManager::IsLevelDocumentOpen()
{
    return (GetIEditor() && GetIEditor()->GetDocument() && GetIEditor()->GetDocument()->IsDocumentReady());
}

AZStd::string SandboxIntegrationManager::GetLevelName()
{
    return AZStd::string(GetIEditor()->GetGameEngine()->GetLevelName().toUtf8().constData());
}

void SandboxIntegrationManager::OnPrepareForContextReset()
{
    // Deselect everything.
    AzToolsFramework::ToolsApplicationRequests::Bus::Broadcast(
        &AzToolsFramework::ToolsApplicationRequests::Bus::Events::SetSelectedEntities, AzToolsFramework::EntityIdList());

    std::vector<CBaseObject*> objects;
    objects.reserve(128);
    IObjectManager* objectManager = GetIEditor()->GetObjectManager();
    objectManager->FindObjectsOfType(OBJTYPE_AZENTITY, objects);
}

void SandboxIntegrationManager::OnActionRegistrationHook()
{
    auto actionManagerInterface = AZ::Interface<AzToolsFramework::ActionManagerInterface>::Get();
    auto hotKeyManagerInterface = AZ::Interface<AzToolsFramework::HotKeyManagerInterface>::Get();

    if (!actionManagerInterface || !hotKeyManagerInterface)
    {
        return;
    }

    // Create entity
    {
        const AZStd::string_view actionIdentifier = "o3de.action.sandbox.createEntity";
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "Create entity";
        actionProperties.m_description = "Creates an entity under the current selection";
        actionProperties.m_category = "Entity";
        actionProperties.m_menuVisibility = AzToolsFramework::ActionVisibility::HideWhenDisabled;

        actionManagerInterface->RegisterAction(
            EditorIdentifiers::MainWindowActionContextIdentifier,
            actionIdentifier,
            actionProperties,
            [this]()
            {
                AzToolsFramework::EntityIdList selectedEntities;
                AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(
                    selectedEntities, &AzToolsFramework::ToolsApplicationRequests::Bus::Events::GetSelectedEntities);

                // when nothing is selected, entity is created at root level.
                if (selectedEntities.empty())
                {
                    ContextMenu_NewEntity();
                }
                // when a single entity is selected, entity is created as its child.
                else if (selectedEntities.size() == 1)
                {
                    AZ::EntityId selectedEntityId = selectedEntities.front();
                    bool selectedEntityIsReadOnly = m_readOnlyEntityPublicInterface->IsReadOnly(selectedEntityId);
                    auto containerEntityInterface = AZ::Interface<AzToolsFramework::ContainerEntityInterface>::Get();
                    bool prefabSystemEnabled = false;
                    AzFramework::ApplicationRequests::Bus::BroadcastResult(
                        prefabSystemEnabled, &AzFramework::ApplicationRequests::IsPrefabSystemEnabled);

                    if (!prefabSystemEnabled ||
                        (containerEntityInterface && containerEntityInterface->IsContainerOpen(selectedEntityId) && !selectedEntityIsReadOnly))
                    {
                        AzToolsFramework::EditorRequestBus::Broadcast(
                            &AzToolsFramework::EditorRequestBus::Handler::CreateNewEntityAsChild, selectedEntityId);
                    }
                }
            }
        );

        actionManagerInterface->InstallEnabledStateCallback(
            actionIdentifier,
            [readOnlyEntityPublicInterface = m_readOnlyEntityPublicInterface]() -> bool
            {
                AzToolsFramework::EntityIdList selectedEntities;
                AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(
                    selectedEntities, &AzToolsFramework::ToolsApplicationRequests::Bus::Events::GetSelectedEntities);

                if (selectedEntities.size() == 0)
                {
                    return true;
                }
                else if (selectedEntities.size() == 1)
                {
                    AZ::EntityId selectedEntityId = selectedEntities.front();
                    bool selectedEntityIsReadOnly = readOnlyEntityPublicInterface->IsReadOnly(selectedEntityId);
                    auto containerEntityInterface = AZ::Interface<AzToolsFramework::ContainerEntityInterface>::Get();
                    bool prefabSystemEnabled = false;
                    AzFramework::ApplicationRequests::Bus::BroadcastResult(
                        prefabSystemEnabled, &AzFramework::ApplicationRequests::IsPrefabSystemEnabled);

                    return (!prefabSystemEnabled ||
                        (containerEntityInterface && containerEntityInterface->IsContainerOpen(selectedEntityId) && !selectedEntityIsReadOnly));
                }

                return false;
            }
        );

        // Trigger update whenever entity selection changes.
        actionManagerInterface->AddActionToUpdater(EditorIdentifiers::EntitySelectionChangedUpdaterIdentifier, actionIdentifier);

        hotKeyManagerInterface->SetActionHotKey(actionIdentifier, "Ctrl+Alt+N");
    }

}

void SandboxIntegrationManager::OnMenuBindingHook()
{
    auto menuManagerInterface = AZ::Interface<AzToolsFramework::MenuManagerInterface>::Get();

    if (!menuManagerInterface)
    {
        return;
    }

    // Entity Outliner Context Menu
    auto outcome = menuManagerInterface->AddActionToMenu(EditorIdentifiers::EntityOutlinerContextMenuIdentifier, "o3de.action.sandbox.createEntity", 100);

    // Viewport Context Menu
    menuManagerInterface->AddActionToMenu(EditorIdentifiers::ViewportContextMenuIdentifier, "o3de.action.sandbox.createEntity", 100);
}

void SandboxIntegrationManager::ContextMenu_NewEntity()
{
    AZ::Vector3 worldPosition = AZ::Vector3::CreateZero();

    // If we don't have a viewport active to aid in placement, the object
    // will be created at the origin.
    if (CViewport* view = GetIEditor()->GetViewManager()->GetGameViewport();
        view && m_contextMenuViewPoint.has_value())
    {
        worldPosition = AzToolsFramework::FindClosestPickIntersection(
            view->GetViewportId(), m_contextMenuViewPoint.value(), AzToolsFramework::EditorPickRayLength,
            AzToolsFramework::GetDefaultEntityPlacementDistance());
    }

    CreateNewEntityAtPosition(worldPosition);
}

void SandboxIntegrationManager::ContextMenu_MakeSlice(AzToolsFramework::EntityIdList entities)
{
    QChar bulletChar(0x2022);

    QMessageBox createSliceBox(GetMainWindow());
    createSliceBox.setWindowTitle(QObject::tr("Create Slice"));
    createSliceBox.setText(QString(QObject::tr("Your selection contains slice instances. What kind of slice do you want to create?"))
        + "\n\n" + QString(bulletChar) + " " + "Fresh slice that doesn't inherit existing slice references."
        + "\n" + QString(bulletChar) + " " + "Nested slice that inherits existing slice references."
        + "\n\n");
    createSliceBox.setIcon(QMessageBox::Warning);
    
    QPushButton* freshSliceButton = createSliceBox.addButton(QObject::tr("Fresh Slice"), QMessageBox::ActionRole);
    QPushButton* nestedSliceButton = createSliceBox.addButton(QObject::tr("Nested Slice"), QMessageBox::ActionRole);
    createSliceBox.addButton(QMessageBox::Cancel);

    createSliceBox.exec();

    if (createSliceBox.clickedButton() == freshSliceButton)
    {
        MakeSliceFromEntities(entities, false, GetIEditor()->GetEditorSettings()->sliceSettings.dynamicByDefault);
    }
    else if (createSliceBox.clickedButton() == nestedSliceButton)
    {
        ContextMenu_InheritSlice(entities);
    }
}

void SandboxIntegrationManager::ContextMenu_InheritSlice(AzToolsFramework::EntityIdList entities)
{
    MakeSliceFromEntities(entities, true, GetIEditor()->GetEditorSettings()->sliceSettings.dynamicByDefault);
}
void SandboxIntegrationManager::ContextMenu_InstantiateSlice()
{
    AssetSelectionModel selection = AssetSelectionModel::AssetTypeSelection("Slice");
    BrowseForAssets(selection);

    if (selection.IsValid())
    {
        auto product = azrtti_cast<const ProductAssetBrowserEntry*>(selection.GetResult());
        AZ_Assert(product, "Incorrect entry type selected. Expected product.");
     
        InstantiateSliceFromAssetId(product->GetAssetId());
    }
}

void SandboxIntegrationManager::InstantiateSliceFromAssetId(const AZ::Data::AssetId& assetId)
{
    AZ::Transform sliceWorldTransform = AZ::Transform::CreateIdentity();

    // If we don't have a viewport active to aid in placement, the slice
    // will be instantiated at the origin.
    if (CViewport* view = GetIEditor()->GetViewManager()->GetGameViewport())
    {
        const QPoint viewPoint =
            AzToolsFramework::ViewportInteraction::QPointFromScreenPoint(m_contextMenuViewPoint.value_or(AzFramework::ScreenPoint{}));
        sliceWorldTransform = AZ::Transform::CreateTranslation(LYVec3ToAZVec3(view->SnapToGrid(view->ViewToWorld(viewPoint))));
    }

    AzToolsFramework::SliceRequestBus::Broadcast(&AzToolsFramework::SliceRequests::InstantiateSliceFromAssetId, assetId, sliceWorldTransform);
}

//////////////////////////////////////////////////////////////////////////
// Returns true if at least one non-layer entity was found.
bool CollectEntityBoundingBoxesForZoom(const AZ::EntityId& entityId, AABB& selectionBounds)
{
    AABB entityBoundingBox;
    CEntityObject* componentEntityObject = nullptr;
    AzToolsFramework::ComponentEntityEditorRequestBus::EventResult(
        /*result*/ componentEntityObject,
        /*address*/ entityId,
        &AzToolsFramework::ComponentEntityEditorRequestBus::Events::GetSandboxObject);

    if (componentEntityObject)
    {
        componentEntityObject->GetBoundBox(entityBoundingBox);
        selectionBounds.Add(entityBoundingBox.min);
        selectionBounds.Add(entityBoundingBox.max);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
void SandboxIntegrationManager::GoToEntitiesInViewports(const AzToolsFramework::EntityIdList& entityIds)
{
    if (entityIds.empty())
    {
        return;
    }

    const AZ::Aabb aabb = AZStd::accumulate(
        AZStd::begin(entityIds), AZStd::end(entityIds), AZ::Aabb::CreateNull(), [](AZ::Aabb acc, const AZ::EntityId entityId) {
            const AZ::Aabb aabb = AzFramework::CalculateEntityWorldBoundsUnion(AzToolsFramework::GetEntityById(entityId));
            acc.AddAabb(aabb);
            return acc;
        });

    float radius;
    AZ::Vector3 center;
    aabb.GetAsSphere(center, radius);

    auto viewportContextManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
    const int viewCount = GetIEditor()->GetViewManager()->GetViewCount(); // legacy call
    for (int viewIndex = 0; viewIndex < viewCount; ++viewIndex)
    {
        if (auto viewportContext = viewportContextManager->GetViewportContextById(viewIndex))
        {
            if (const AZStd::optional<AZ::Transform> nextCameraTransform = SandboxEditor::CalculateGoToEntityTransform(
                    viewportContext->GetCameraTransform(),
                    AzFramework::RetrieveFov(viewportContext->GetCameraProjectionMatrix()),
                    center,
                    radius);
                nextCameraTransform.has_value())
            {
                SandboxEditor::HandleDefaultViewportCameraTransitionFromSetting(*nextCameraTransform);
            }
        }
    }
}

void SandboxIntegrationManager::ContextMenu_SelectSlice()
{
    AzToolsFramework::EntityIdList selectedEntities;
    GetSelectedOrHighlightedEntities(selectedEntities);

    AzToolsFramework::EntityIdList newSelectedEntities;

    for (const AZ::EntityId& entityId : selectedEntities)
    {
        AZ::SliceComponent::SliceInstanceAddress sliceAddress;
        AzFramework::SliceEntityRequestBus::EventResult(sliceAddress, entityId,
            &AzFramework::SliceEntityRequestBus::Events::GetOwningSlice);

        if (sliceAddress.IsValid())
        {
            const AZ::SliceComponent::InstantiatedContainer* instantiated = sliceAddress.GetInstance()->GetInstantiated();

            if (instantiated)
            {
                for (AZ::Entity* entityInSlice : instantiated->m_entities)
                {
                    newSelectedEntities.push_back(entityInSlice->GetId());
                }
            }
        }
    }

    AzToolsFramework::ToolsApplicationRequests::Bus::Broadcast(
        &AzToolsFramework::ToolsApplicationRequests::Bus::Events::SetSelectedEntities, newSelectedEntities);
}

void SandboxIntegrationManager::ContextMenu_PushEntitiesToSlice(AzToolsFramework::EntityIdList entities,
    AZ::SliceComponent::EntityAncestorList ancestors,
    AZ::Data::AssetId targetAncestorId,
    bool affectEntireHierarchy)
{
    (void)ancestors;
    (void)targetAncestorId;
    (void)affectEntireHierarchy;

    AZ::SerializeContext* serializeContext = nullptr;
    AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);
    AZ_Assert(serializeContext, "No serialize context");

    AzToolsFramework::SliceUtilities::PushEntitiesModal(GetMainWindow(), entities, serializeContext);
}

void SandboxIntegrationManager::ContextMenu_Duplicate()
{
    bool handled = true;
    AzToolsFramework::EditorRequestBus::Broadcast(&AzToolsFramework::EditorRequests::CloneSelection, handled);
}

void SandboxIntegrationManager::ContextMenu_DeleteSelected()
{
    DeleteSelectedEntities(true);
}

void SandboxIntegrationManager::ContextMenu_ResetToSliceDefaults(AzToolsFramework::EntityIdList entities)
{
    AzToolsFramework::SliceEditorEntityOwnershipServiceRequestBus::Broadcast(
        &AzToolsFramework::SliceEditorEntityOwnershipServiceRequests::ResetEntitiesToSliceDefaults, entities);
}

void SandboxIntegrationManager::GetSelectedEntities(AzToolsFramework::EntityIdList& entities)
{
    AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(
        entities, &AzToolsFramework::ToolsApplicationRequests::Bus::Events::GetSelectedEntities);
}

void SandboxIntegrationManager::GetSelectedOrHighlightedEntities(AzToolsFramework::EntityIdList& entities)
{
    AzToolsFramework::EntityIdList selectedEntities;
    AzToolsFramework::EntityIdList highlightedEntities;

    AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(
        selectedEntities, &AzToolsFramework::ToolsApplicationRequests::Bus::Events::GetSelectedEntities);

    AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(
        highlightedEntities, &AzToolsFramework::ToolsApplicationRequests::Bus::Events::GetHighlightedEntities);

    entities = AZStd::move(selectedEntities);

    for (AZ::EntityId highlightedId : highlightedEntities)
    {
        if (entities.end() == AZStd::find(entities.begin(), entities.end(), highlightedId))
        {
            entities.push_back(highlightedId);
        }
    }
}

AZStd::string SandboxIntegrationManager::GetComponentEditorIcon(const AZ::Uuid& componentType, const AZ::Component* component)
{
    AZStd::string iconPath = GetComponentIconPath(componentType, AZ::Edit::Attributes::Icon, component);
    return iconPath;
}

AZStd::string SandboxIntegrationManager::GetComponentTypeEditorIcon(const AZ::Uuid& componentType)
{
    return GetComponentEditorIcon(componentType, nullptr);
}

AZStd::string SandboxIntegrationManager::GetComponentIconPath(const AZ::Uuid& componentType,
    AZ::Crc32 componentIconAttrib, const AZ::Component* component)
{
    AZ_PROFILE_FUNCTION(AzToolsFramework);
    if (componentIconAttrib != AZ::Edit::Attributes::Icon
        && componentIconAttrib != AZ::Edit::Attributes::ViewportIcon
        && componentIconAttrib != AZ::Edit::Attributes::HideIcon)
    {
        AZ_Warning("SandboxIntegration", false, "Unrecognized component icon attribute!");
    }

    // return blank path if component shouldn't have icon at all
    AZStd::string iconPath;

    AZ::SerializeContext* serializeContext = nullptr;
    AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);
    AZ_Assert(serializeContext, "No serialize context");

    auto classData = serializeContext->FindClassData(componentType);
    if (classData && classData->m_editData)
    {
        // check if component icon should be hidden
        bool hideIcon = false;

        auto editorElementData = classData->m_editData->FindElementData(AZ::Edit::ClassElements::EditorData);
        if (editorElementData)
        {
            if (auto hideIconAttribute = editorElementData->FindAttribute(AZ::Edit::Attributes::HideIcon))
            {
                auto hideIconAttributeData = azdynamic_cast<const AZ::Edit::AttributeData<bool>*>(hideIconAttribute);
                if (hideIconAttributeData)
                {
                    hideIcon = hideIconAttributeData->Get(nullptr);
                }
            }
        }

        if (!hideIcon)
        {
            // component should have icon. start with default
            iconPath = GetDefaultComponentEditorIcon();

            // check for specific icon
            if (editorElementData)
            {
                if (auto iconAttribute = editorElementData->FindAttribute(componentIconAttrib))
                {
                    if (auto iconAttributeData = azdynamic_cast<const AZ::Edit::AttributeData<const char*>*>(iconAttribute))
                    {
                        AZStd::string iconAttributeValue = iconAttributeData->Get(component);
                        if (!iconAttributeValue.empty())
                        {
                            iconPath = AZStd::move(iconAttributeValue);
                        }
                    }

                    auto iconOverrideAttribute = editorElementData->FindAttribute(AZ::Edit::Attributes::DynamicIconOverride);

                    // If it has an override and we're given an instance, then get any potential override from the instance here
                    if (component &&
                        (componentIconAttrib == AZ::Edit::Attributes::Icon || componentIconAttrib == AZ::Edit::Attributes::ViewportIcon) &&
                        iconOverrideAttribute)
                    {
                        AZStd::string iconValue;
                        AZ::AttributeReader iconReader(const_cast<AZ::Component*>(component), iconOverrideAttribute);
                        iconReader.Read<AZStd::string>(iconValue);

                        if (!iconValue.empty())
                        {
                            iconPath = AZStd::move(iconValue);
                        }
                    }
                }
            }
            // If Qt doesn't know where the relative path is we have to use the more expensive full path
            if (!QFile::exists(QString(iconPath.c_str())))
            {
                // use absolute path if possible
                AZStd::string iconFullPath;
                bool pathFound = false;
                using AssetSysReqBus = AzToolsFramework::AssetSystemRequestBus;
                AssetSysReqBus::BroadcastResult(
                    pathFound, &AssetSysReqBus::Events::GetFullSourcePathFromRelativeProductPath,
                    iconPath, iconFullPath);
                if (pathFound)
                {
                    iconPath = AZStd::move(iconFullPath);
                }
                else
                {
                    // If we couldn't find the full source path, try appending the product extension if the icon
                    // source asset is one of the supported image assets. Most component icons are in .svg format,
                    // which isn't actually consumed by the Asset Processor so the GetFullSourcePathFromRelativeProductPath
                    // API can find the source asset without needing the product extension as well. So this edge case is
                    // to cover any component icons that are still using other formats (e.g. png), that haven't been converted
                    // to .svg yet, or for customers that prefer to use image formats besides .svg.
                    AZStd::string extension;
                    AzFramework::StringFunc::Path::GetExtension(iconPath.c_str(), extension);
                    bool supportedStreamingImage = false;
                    for (int i = 0; i < ImageProcessingAtom::s_TotalSupportedImageExtensions; ++i)
                    {
                        if (AZStd::wildcard_match(ImageProcessingAtom::s_SupportedImageExtensions[i], extension.c_str()))
                        {
                            supportedStreamingImage = true;
                            break;
                        }
                    }
                    if (supportedStreamingImage)
                    {
                        iconPath = AZStd::string::format("%s.%s", iconPath.c_str(), AZ::RPI::StreamingImageAsset::Extension);

                        AssetSysReqBus::BroadcastResult(
                            pathFound, &AssetSysReqBus::Events::GetFullSourcePathFromRelativeProductPath,
                            iconPath, iconFullPath);
                    }

                    if (pathFound)
                    {
                        iconPath = AZStd::move(iconFullPath);
                    }
                    else
                    {
                        AZ_Warning(
                            "SandboxIntegration",
                            false,
                            "Unable to find icon path \"%s\" for component type: %s",
                            iconPath.c_str(),
                            classData->m_editData->m_name);
                    }
                }
            }
        }
    }

    return iconPath;
}

void SandboxIntegrationManager::UndoStackFlushed()
{
    AzToolsFramework::ToolsApplicationRequestBus::Broadcast(&AzToolsFramework::ToolsApplicationRequestBus::Events::FlushUndo);
}

void SandboxIntegrationManager::MakeSliceFromEntities(const AzToolsFramework::EntityIdList& entities, bool inheritSlices, bool setAsDynamic)
{
    // expand the list of entities to include all transform descendant entities
    AzToolsFramework::EntityIdSet entitiesAndDescendants;
    AzToolsFramework::ToolsApplicationRequestBus::BroadcastResult(entitiesAndDescendants,
        &AzToolsFramework::ToolsApplicationRequestBus::Events::GatherEntitiesAndAllDescendents, entities);

    const AZStd::string slicesAssetsPath = "@projectroot@/Slices";

    if (!gEnv->pFileIO->Exists(slicesAssetsPath.c_str()))
    {
        gEnv->pFileIO->CreatePath(slicesAssetsPath.c_str());
    }

    char path[AZ_MAX_PATH_LEN] = { 0 };
    gEnv->pFileIO->ResolvePath(slicesAssetsPath.c_str(), path, AZ_MAX_PATH_LEN);
    AzToolsFramework::SliceUtilities::MakeNewSlice(entitiesAndDescendants, path, inheritSlices, setAsDynamic);
}

void SandboxIntegrationManager::RegisterViewPane(const char* name, const char* category, const AzToolsFramework::ViewPaneOptions& viewOptions, const WidgetCreationFunc& widgetCreationFunc)
{
    QtViewPaneManager::instance()->RegisterPane(name, category, widgetCreationFunc, viewOptions);
}

void SandboxIntegrationManager::RegisterCustomViewPane(const char* name, const char* category, const AzToolsFramework::ViewPaneOptions& viewOptions)
{
    QtViewPaneManager::instance()->RegisterPane(name, category, nullptr, viewOptions);
}

void SandboxIntegrationManager::UnregisterViewPane(const char* name)
{
    QtViewPaneManager::instance()->UnregisterPane(name);
}

QWidget* SandboxIntegrationManager::GetViewPaneWidget(const char* viewPaneName)
{
    return FindViewPane<QWidget>(viewPaneName);
}

void SandboxIntegrationManager::OpenViewPane(const char* paneName)
{
    const QtViewPane* pane = QtViewPaneManager::instance()->OpenPane(paneName);
    if (pane)
    {
        pane->m_dockWidget->raise();
        pane->m_dockWidget->activateWindow();
    }
}

QDockWidget* SandboxIntegrationManager::InstanceViewPane(const char* paneName)
{
    return QtViewPaneManager::instance()->InstancePane(paneName);
}

void SandboxIntegrationManager::CloseViewPane(const char* paneName)
{
    QtViewPaneManager::instance()->ClosePane(paneName);
}

void SandboxIntegrationManager::BrowseForAssets(AssetSelectionModel& selection)
{
    AssetBrowserComponentRequestBus::Broadcast(&AssetBrowserComponentRequests::PickAssets, selection, GetMainWindow());
}
