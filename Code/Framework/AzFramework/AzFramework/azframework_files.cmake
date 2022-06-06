#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Asset/AssetSystemComponentHelper.cpp
    AzFrameworkModule.h
    AzFrameworkModule.cpp
    API/ApplicationAPI.h
    Application/Application.cpp
    Application/Application.h
    Archive/Archive.cpp
    Archive/Archive.h
    Archive/ArchiveBus.h
    Archive/ArchiveFileIO.cpp
    Archive/ArchiveFileIO.h
    Archive/ArchiveFindData.cpp
    Archive/ArchiveFindData.h
    Archive/ArchiveVars.h
    Archive/ArchiveVars.cpp
    Archive/Codec.h
    Archive/IArchive.h
    Archive/INestedArchive.h
    Archive/MissingFileReport.h
    Archive/MissingFileReport.cpp
    Archive/NestedArchive.cpp
    Archive/NestedArchive.h
    Archive/ZipDirCache.cpp
    Archive/ZipDirCacheFactory.cpp
    Archive/ZipDirFind.cpp
    Archive/ZipDirList.cpp
    Archive/ZipDirStructures.cpp
    Archive/ZipDirTree.cpp
    Archive/ZipDirCache.h
    Archive/ZipDirCacheFactory.h
    Archive/ZipDirFind.h
    Archive/ZipDirList.h
    Archive/ZipDirStructures.h
    Archive/ZipDirTree.h
    Archive/ZipFileFormat.h
    Asset/SimpleAsset.cpp
    Asset/SimpleAsset.h
    Asset/AssetCatalogBus.h
    Asset/AssetSystemBus.h
    Asset/AssetSystemTypes.h
    Asset/AssetCatalog.h
    Asset/AssetCatalog.cpp
    Asset/AssetCatalogComponent.h
    Asset/AssetCatalogComponent.cpp
    Asset/AssetProcessorMessages.cpp
    Asset/AssetProcessorMessages.h
    Asset/AssetRegistry.h
    Asset/AssetRegistry.cpp
    Asset/AssetSeedList.cpp
    Asset/AssetSeedList.h
    Asset/AssetSystemComponent.cpp
    Asset/AssetSystemComponent.h
    Asset/GenericAssetHandler.h
    Asset/AssetBundleManifest.cpp
    Asset/AssetBundleManifest.h
    Asset/CustomAssetTypeComponent.cpp
    Asset/CustomAssetTypeComponent.h
    Asset/FileTagAsset.cpp
    Asset/FileTagAsset.h
    Asset/NetworkAssetNotification_private.h
    Asset/XmlSchemaAsset.cpp
    Asset/XmlSchemaAsset.h
    Asset/Benchmark/BenchmarkAsset.cpp
    Asset/Benchmark/BenchmarkAsset.h
    Asset/Benchmark/BenchmarkCommands.cpp
    Asset/Benchmark/BenchmarkCommands.h
    Asset/Benchmark/BenchmarkSettingsAsset.cpp
    Asset/Benchmark/BenchmarkSettingsAsset.h
    AutoGen/AzFramework.AutoPackets.xml
    CommandLine/CommandLine.h
    CommandLine/CommandRegistrationBus.h
    Viewport/ViewportBus.h
    Viewport/ViewportBus.cpp
    Viewport/ViewportColors.h
    Viewport/ViewportColors.cpp
    Viewport/ViewportConstants.h
    Viewport/ViewportConstants.cpp
    Viewport/MultiViewportController.h
    Viewport/MultiViewportController.inl
    Viewport/SingleViewportController.cpp
    Viewport/SingleViewportController.h
    Viewport/ViewportControllerInterface.h
    Viewport/ViewportControllerList.cpp
    Viewport/ViewportControllerList.h
    Viewport/ViewportId.h
    Viewport/ViewportScreen.h
    Viewport/ViewportScreen.cpp
    Viewport/ScreenGeometry.h
    Viewport/ScreenGeometry.cpp
    Viewport/CameraState.h
    Viewport/CameraState.cpp
    Viewport/CameraInput.h
    Viewport/CameraInput.cpp
    Viewport/ClickDetector.h
    Viewport/ClickDetector.cpp
    Viewport/CursorState.h
    Viewport/DisplayContextRequestBus.h
    Entity/BehaviorEntity.cpp
    Entity/BehaviorEntity.h
    Entity/GameEntityContextComponent.cpp
    Entity/GameEntityContextComponent.h
    Entity/GameEntityContextBus.h
    Entity/EntityContext.cpp
    Entity/EntityContext.h
    Entity/EntityContextBus.h
    Entity/EntityDebugDisplayBus.h
    Entity/EntityOwnershipService.h
    Entity/EntityOwnershipServiceBus.h
    Entity/SliceEntityOwnershipServiceBus.h
    Entity/SliceEntityOwnershipService.h
    Entity/SliceEntityOwnershipService.cpp
    Entity/SliceGameEntityOwnershipService.h
    Entity/SliceGameEntityOwnershipService.cpp
    Entity/SliceGameEntityOwnershipServiceBus.h
    Entity/PrefabEntityOwnershipService.h
    Components/ComponentAdapter.h
    Components/ComponentAdapter.inl
    Components/ComponentAdapterHelpers.h
    Components/EditorEntityEvents.h
    Components/TransformComponent.cpp
    Components/TransformComponent.h
    Components/CameraBus.h
    Components/ConsoleBus.h
    Components/ConsoleBus.cpp
    Components/DeprecatedComponentsBus.h
    Components/AzFrameworkConfigurationSystemComponent.h
    Components/AzFrameworkConfigurationSystemComponent.cpp
    Components/NonUniformScaleComponent.h
    Components/NonUniformScaleComponent.cpp
    DocumentPropertyEditor/AdapterBuilder.cpp
    DocumentPropertyEditor/AdapterBuilder.h
    DocumentPropertyEditor/BasicAdapter.cpp
    DocumentPropertyEditor/BasicAdapter.h
    DocumentPropertyEditor/DocumentAdapter.cpp
    DocumentPropertyEditor/DocumentAdapter.h
    DocumentPropertyEditor/DocumentSchema.h
    DocumentPropertyEditor/PropertyEditorNodes.cpp
    DocumentPropertyEditor/PropertyEditorNodes.h
    DocumentPropertyEditor/PropertyEditorSystem.cpp
    DocumentPropertyEditor/PropertyEditorSystem.h
    DocumentPropertyEditor/PropertyEditorSystemInterface.h
    DocumentPropertyEditor/CvarAdapter.cpp
    DocumentPropertyEditor/CvarAdapter.h
    DocumentPropertyEditor/RoutingAdapter.cpp
    DocumentPropertyEditor/RoutingAdapter.h
    DocumentPropertyEditor/ReflectionAdapter.cpp
    DocumentPropertyEditor/ReflectionAdapter.h
    DocumentPropertyEditor/Reflection/Attribute.h
    DocumentPropertyEditor/Reflection/LegacyReflectionBridge.cpp
    DocumentPropertyEditor/Reflection/LegacyReflectionBridge.h
    DocumentPropertyEditor/Reflection/Visitor.cpp
    DocumentPropertyEditor/Reflection/Visitor.h
    FileFunc/FileFunc.h
    FileFunc/FileFunc.cpp
    Font/FontInterface.h
    Gem/GemInfo.cpp
    Gem/GemInfo.h
    StringFunc/StringFunc.h
    InGameUI/UiFrameworkBus.h
    IO/LocalFileIO.cpp
    IO/LocalFileIO.h
    IO/FileOperations.h
    IO/FileOperations.cpp
    IO/RemoteFileIO.cpp
    IO/RemoteFileIO.h
    IO/RemoteStorageDrive.h
    IO/RemoteStorageDrive.cpp
    Math/InterpolationSample.h
    Metrics/MetricsPlainTextNameRegistration.h
    Network/AssetProcessorConnection.cpp
    Network/AssetProcessorConnection.h
    Network/IRemoteTools.h
    Network/IRemoteTools.inl
    Network/SocketConnection.cpp
    Network/SocketConnection.h
    Logging/LogFile.cpp
    Logging/LogFile.h
    Logging/LoggingComponent.cpp
    Logging/LoggingComponent.h
    Logging/StartupLogSinkReporter.cpp
    Logging/StartupLogSinkReporter.h
    Logging/MissingAssetLogger.cpp
    Logging/MissingAssetLogger.h
    Logging/MissingAssetNotificationBus.h
    Scene/Scene.h
    Scene/Scene.inl
    Scene/Scene.cpp
    Scene/SceneSystemComponent.h
    Scene/SceneSystemComponent.cpp
    Scene/SceneSystemInterface.h
    Script/ScriptComponent.h
    Script/ScriptComponent.cpp
    Script/ScriptDebugAgentBus.h
    Script/ScriptDebugMsgReflection.cpp
    Script/ScriptDebugMsgReflection.h
    Script/ScriptRemoteDebugging.cpp
    Script/ScriptRemoteDebugging.h
    StreamingInstall/StreamingInstall.h
    StreamingInstall/StreamingInstall.cpp
    StreamingInstall/StreamingInstallRequests.h
    StreamingInstall/StreamingInstallNotifications.h
    Physics/Collision/CollisionEvents.h
    Physics/Collision/CollisionEvents.cpp
    Physics/Collision/CollisionLayers.h
    Physics/Collision/CollisionLayers.cpp
    Physics/Collision/CollisionGroups.h
    Physics/Collision/CollisionGroups.cpp
    Physics/Common/PhysicsJoint.h
    Physics/Common/PhysicsJoint.cpp
    Physics/Common/PhysicsSceneQueries.h
    Physics/Common/PhysicsSceneQueries.cpp
    Physics/Common/PhysicsEvents.h
    Physics/Common/PhysicsSimulatedBody.h
    Physics/Common/PhysicsSimulatedBody.cpp
    Physics/Common/PhysicsSimulatedBodyAutomation.h
    Physics/Common/PhysicsSimulatedBodyAutomation.cpp
    Physics/Common/PhysicsSimulatedBodyEvents.h
    Physics/Common/PhysicsSimulatedBodyEvents.cpp
    Physics/Common/PhysicsTypes.h
    Physics/Components/SimulatedBodyComponentBus.h
    Physics/Configuration/JointConfiguration.h
    Physics/Configuration/JointConfiguration.cpp
    Physics/Configuration/CollisionConfiguration.h
    Physics/Configuration/CollisionConfiguration.cpp
    Physics/Configuration/RigidBodyConfiguration.h
    Physics/Configuration/RigidBodyConfiguration.cpp
    Physics/Configuration/StaticRigidBodyConfiguration.h
    Physics/Configuration/StaticRigidBodyConfiguration.cpp
    Physics/Configuration/SceneConfiguration.h
    Physics/Configuration/SceneConfiguration.cpp
    Physics/Configuration/SimulatedBodyConfiguration.h
    Physics/Configuration/SimulatedBodyConfiguration.cpp
    Physics/Configuration/SystemConfiguration.h
    Physics/Configuration/SystemConfiguration.cpp
    Physics/DebugDraw/CharacterPhysicsDebugDraw.h
    Physics/DebugDraw/CharacterPhysicsDebugDraw.cpp
    Physics/HeightfieldProviderBus.h
    Physics/HeightfieldProviderBus.cpp
    Physics/SimulatedBodies/RigidBody.h
    Physics/SimulatedBodies/RigidBody.cpp
    Physics/SimulatedBodies/StaticRigidBody.h
    Physics/SimulatedBodies/StaticRigidBody.cpp
    Physics/PhysicsSystem.h
    Physics/PhysicsSystem.cpp
    Physics/PhysicsScene.cpp
    Physics/PhysicsScene.h
    Physics/AnimationConfiguration.cpp
    Physics/AnimationConfiguration.h
    Physics/Character.cpp
    Physics/Character.h
    Physics/CollisionBus.h
    Physics/CollisionBus.cpp
    Physics/Material.cpp
    Physics/Material.h
    Physics/NameConstants.cpp
    Physics/NameConstants.h
    Physics/RigidBodyBus.h
    Physics/Shape.cpp
    Physics/Shape.h
    Physics/ShapeConfiguration.h
    Physics/ShapeConfiguration.cpp
    Physics/SystemBus.h
    Physics/ColliderComponentBus.h
    Physics/RagdollPhysicsBus.h
    Physics/CharacterPhysicsDataBus.h
    Physics/CharacterBus.h
    Physics/Ragdoll.cpp
    Physics/Ragdoll.h
    Physics/Utils.h
    Physics/Utils.cpp
    Physics/ClassConverters.cpp
    Physics/ClassConverters.h
    Physics/MaterialBus.h
    Physics/WindBus.h
    Process/ProcessCommunicator.cpp
    Process/ProcessCommunicator.h
    Process/ProcessCommon_fwd.h
    Process/ProcessCommunicator.h
    Process/ProcessWatcher.cpp
    Process/ProcessWatcher.h
    Process/ProcessCommon_fwd.h
    Process/ProcessCommunicatorTracePrinter.cpp
    Process/ProcessCommunicatorTracePrinter.h
    ProjectManager/ProjectManager.h
    ProjectManager/ProjectManager.cpp
    Render/GameIntersectorComponent.h
    Render/GameIntersectorComponent.cpp
    Render/GeometryIntersectionBus.h
    Render/GeometryIntersectionStructures.h
    Render/Intersector.cpp
    Render/Intersector.h
    Render/IntersectorInterface.h
    Spawnable/RootSpawnableInterface.h
    Spawnable/Script/SpawnableScriptAssetRef.cpp
    Spawnable/Script/SpawnableScriptAssetRef.h
    Spawnable/Script/SpawnableScriptBus.h
    Spawnable/Script/SpawnableScriptMediator.cpp
    Spawnable/Script/SpawnableScriptMediator.h
    Spawnable/Script/SpawnableScriptNotificationsHandler.h
    Spawnable/InMemorySpawnableAssetContainer.cpp
    Spawnable/InMemorySpawnableAssetContainer.h
    Spawnable/Spawnable.cpp
    Spawnable/Spawnable.h
    Spawnable/SpawnableAssetBus.h
    Spawnable/SpawnableAssetHandler.h
    Spawnable/SpawnableAssetHandler.cpp
    Spawnable/SpawnableAssetUtils.h
    Spawnable/SpawnableAssetUtils.cpp
    Spawnable/SpawnableEntitiesContainer.h
    Spawnable/SpawnableEntitiesContainer.cpp
    Spawnable/SpawnableEntitiesInterface.h
    Spawnable/SpawnableEntitiesInterface.cpp
    Spawnable/SpawnableEntitiesManager.h
    Spawnable/SpawnableEntitiesManager.cpp
    Spawnable/SpawnableMetaData.cpp
    Spawnable/SpawnableMetaData.h
    Spawnable/SpawnableMonitor.h
    Spawnable/SpawnableMonitor.cpp
    Spawnable/SpawnableMonitor.cpp
    Spawnable/SpawnableSystemComponent.h
    Spawnable/SpawnableSystemComponent.cpp
    SurfaceData/SurfaceData.h
    SurfaceData/SurfaceData.cpp
    Terrain/TerrainDataRequestBus.h
    Terrain/TerrainDataRequestBus.cpp
    Thermal/ThermalInfo.h
    Platform/PlatformDefaults.h
    Windowing/WindowBus.h
    Windowing/NativeWindow.cpp
    Windowing/NativeWindow.h
    Input/Buses/Notifications/InputChannelNotificationBus.h
    Input/Buses/Notifications/InputDeviceNotificationBus.h
    Input/Buses/Notifications/InputSystemNotificationBus.h
    Input/Buses/Notifications/InputTextNotificationBus.h
    Input/Buses/Requests/InputChannelRequestBus.h
    Input/Buses/Requests/InputDeviceRequestBus.h
    Input/Buses/Requests/InputHapticFeedbackRequestBus.h
    Input/Buses/Requests/InputLightBarRequestBus.h
    Input/Buses/Requests/InputMotionSensorRequestBus.h
    Input/Buses/Requests/InputSystemRequestBus.h
    Input/Buses/Requests/InputSystemCursorRequestBus.h
    Input/Buses/Requests/InputTextEntryRequestBus.h
    Input/Channels/InputChannel.cpp
    Input/Channels/InputChannel.h
    Input/Channels/InputChannelAnalog.cpp
    Input/Channels/InputChannelAnalog.h
    Input/Channels/InputChannelAnalogWithPosition2D.cpp
    Input/Channels/InputChannelAnalogWithPosition2D.h
    Input/Channels/InputChannelAxis1D.cpp
    Input/Channels/InputChannelAxis1D.h
    Input/Channels/InputChannelAxis2D.cpp
    Input/Channels/InputChannelAxis2D.h
    Input/Channels/InputChannelAxis3D.cpp
    Input/Channels/InputChannelAxis3D.h
    Input/Channels/InputChannelDelta.cpp
    Input/Channels/InputChannelDelta.h
    Input/Channels/InputChannelDeltaWithSharedPosition2D.cpp
    Input/Channels/InputChannelDeltaWithSharedPosition2D.h
    Input/Channels/InputChannelDigital.cpp
    Input/Channels/InputChannelDigital.h
    Input/Channels/InputChannelDigitalWithPosition2D.cpp
    Input/Channels/InputChannelDigitalWithPosition2D.h
    Input/Channels/InputChannelDigitalWithSharedModifierKeyStates.cpp
    Input/Channels/InputChannelDigitalWithSharedModifierKeyStates.h
    Input/Channels/InputChannelDigitalWithSharedPosition2D.cpp
    Input/Channels/InputChannelDigitalWithSharedPosition2D.h
    Input/Channels/InputChannelId.cpp
    Input/Channels/InputChannelId.h
    Input/Channels/InputChannelQuaternion.cpp
    Input/Channels/InputChannelQuaternion.h
    Input/Contexts/InputContext.cpp
    Input/Contexts/InputContext.h
    Input/Contexts/InputContextComponent.cpp
    Input/Contexts/InputContextComponent.h
    Input/Devices/InputDevice.cpp
    Input/Devices/InputDevice.h
    Input/Devices/InputDeviceId.cpp
    Input/Devices/InputDeviceId.h
    Input/Devices/Gamepad/InputDeviceGamepad.cpp
    Input/Devices/Gamepad/InputDeviceGamepad.h
    Input/Devices/Keyboard/InputDeviceKeyboard.cpp
    Input/Devices/Keyboard/InputDeviceKeyboard.h
    Input/Devices/Keyboard/InputDeviceKeyboardWindowsScanCodes.h
    Input/Devices/Motion/InputDeviceMotion.cpp
    Input/Devices/Motion/InputDeviceMotion.h
    Input/Devices/Mouse/InputDeviceMouse.cpp
    Input/Devices/Mouse/InputDeviceMouse.h
    Input/Devices/VirtualKeyboard/InputDeviceVirtualKeyboard.cpp
    Input/Devices/VirtualKeyboard/InputDeviceVirtualKeyboard.h
    Input/Devices/Touch/InputDeviceTouch.cpp
    Input/Devices/Touch/InputDeviceTouch.h
    Input/Events/InputChannelEventFilter.cpp
    Input/Events/InputChannelEventFilter.h
    Input/Events/InputChannelEventListener.cpp
    Input/Events/InputChannelEventListener.h
    Input/Events/InputChannelEventSink.cpp
    Input/Events/InputChannelEventSink.h
    Input/Events/InputTextEventListener.cpp
    Input/Events/InputTextEventListener.h
    Input/Mappings/InputMapping.cpp
    Input/Mappings/InputMapping.h
    Input/Mappings/InputMappingAnd.cpp
    Input/Mappings/InputMappingAnd.h
    Input/Mappings/InputMappingOr.cpp
    Input/Mappings/InputMappingOr.h
    Input/System/InputSystemComponent.cpp
    Input/System/InputSystemComponent.h
    Input/User/LocalUserId.h
    Input/Utils/AdjustAnalogInputForDeadZone.h
    Input/Utils/IsAnyKeyOrButton.h
    Input/Utils/ProcessRawInputEventQueues.h
    TargetManagement/NeighborhoodAPI.h
    TargetManagement/TargetManagementAPI.h
    TargetManagement/TargetManagementComponent.cpp
    TargetManagement/TargetManagementComponent.h
    FileTag/FileTag.h
    FileTag/FileTag.cpp
    FileTag/FileTagBus.h
    FileTag/FileTagComponent.h
    FileTag/FileTagComponent.cpp
    UnitTest/FrameworkTestTypes.h
    UnitTest/TestDebugDisplayRequests.h
    UnitTest/TestDebugDisplayRequests.cpp
    Slice/SliceEntityBus.h
    Slice/SliceInstantiationBus.h
    Slice/SliceInstantiationTicket.h
    Slice/SliceInstantiationTicket.cpp
    Visibility/IVisibilitySystem.h
    Visibility/OctreeSystemComponent.h
    Visibility/OctreeSystemComponent.cpp
    Visibility/BoundsBus.h
    Visibility/BoundsBus.cpp
    Visibility/VisibilityDebug.h
    Visibility/VisibilityDebug.cpp
    Visibility/EntityBoundsUnionBus.h
    Visibility/EntityVisibilityBoundsUnionSystem.h
    Visibility/EntityVisibilityBoundsUnionSystem.cpp
    Visibility/EntityVisibilityQuery.h
    Visibility/EntityVisibilityQuery.cpp
    Dependency/Dependency.h
    Dependency/Dependency.inl
    Dependency/Version.h
)
