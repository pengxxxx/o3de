#
# Copyright (c) Contributors to the Open 3D Engine Project
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Source/PhysX_precompiled.h
    Source/SystemComponent.cpp
    Source/SystemComponent.h
    Include/PhysX/SystemComponentBus.h
    Include/PhysX/ColliderComponentBus.h
    Include/PhysX/NativeTypeIdentifiers.h
    Include/PhysX/MeshColliderComponentBus.h
    Include/PhysX/MeshAsset.h
    Include/PhysX/HeightFieldAsset.h
    Include/PhysX/HeightFieldAsset.cpp
    Include/PhysX/UserDataTypes.h
    Include/PhysX/UserDataTypes.inl
    Include/PhysX/Utils.h
    Include/PhysX/Utils.inl
    Include/PhysX/MathConversion.h
    Include/PhysX/ComponentTypeIds.h
    Include/PhysX/ForceRegionComponentBus.h
    Include/PhysX/ColliderShapeBus.h
    Include/PhysX/PhysXLocks.h
    Include/PhysX/CharacterControllerBus.h
    Include/PhysX/CharacterGameplayBus.h
    Source/RigidBodyComponent.cpp
    Source/RigidBodyComponent.h
    Source/BaseColliderComponent.cpp
    Source/BaseColliderComponent.h
    Source/MeshColliderComponent.cpp
    Source/MeshColliderComponent.h
    Source/BoxColliderComponent.h
    Source/BoxColliderComponent.cpp
    Source/SphereColliderComponent.h
    Source/SphereColliderComponent.cpp
    Source/CapsuleColliderComponent.h
    Source/CapsuleColliderComponent.cpp
    Source/ShapeColliderComponent.cpp
    Source/ShapeColliderComponent.h
    Source/ForceRegionComponent.cpp
    Source/ForceRegionComponent.h
    Source/StaticRigidBodyComponent.cpp
    Source/StaticRigidBodyComponent.h
    Source/BallJointComponent.cpp
    Source/BallJointComponent.h
    Source/FixedJointComponent.cpp
    Source/FixedJointComponent.h
    Source/HingeJointComponent.cpp
    Source/HingeJointComponent.h
    Source/JointComponent.cpp
    Source/JointComponent.h
    Source/Pipeline/MeshAssetHandler.cpp
    Source/Pipeline/MeshAssetHandler.h
    Source/Pipeline/HeightFieldAssetHandler.cpp
    Source/Pipeline/HeightFieldAssetHandler.h
    Source/Pipeline/StreamWrapper.cpp
    Source/Pipeline/StreamWrapper.h
    Source/Utils.h
    Source/Utils.cpp
    Source/RigidBody.cpp
    Source/RigidBody.h
    Source/RigidBodyStatic.cpp
    Source/RigidBodyStatic.h
    Source/Collision.cpp
    Source/Collision.h
    Source/Shape.h
    Source/Shape.cpp
    Source/Material.cpp
    Source/Material.h
    Source/ForceRegionForces.cpp
    Source/ForceRegionForces.h
    Source/ForceRegion.cpp
    Source/ForceRegion.h
    Source/NameConstants.cpp
    Source/NameConstants.h
    Source/DefaultWorldComponent.cpp
    Source/DefaultWorldComponent.h
    Source/WindProvider.cpp    
    Source/WindProvider.h
    Source/PhysXCharacters/API/CharacterController.cpp
    Source/PhysXCharacters/API/CharacterController.h
    Source/PhysXCharacters/API/Ragdoll.cpp
    Source/PhysXCharacters/API/Ragdoll.h
    Source/PhysXCharacters/API/RagdollNode.cpp
    Source/PhysXCharacters/API/RagdollNode.h
    Source/PhysXCharacters/API/CharacterUtils.cpp
    Source/PhysXCharacters/API/CharacterUtils.h
    Source/PhysXCharacters/Components/CharacterControllerComponent.cpp
    Source/PhysXCharacters/Components/CharacterControllerComponent.h
    Source/PhysXCharacters/Components/CharacterGameplayComponent.cpp
    Source/PhysXCharacters/Components/CharacterGameplayComponent.h
    Source/PhysXCharacters/Components/RagdollComponent.cpp
    Source/PhysXCharacters/Components/RagdollComponent.h
    Include/PhysX/Debug/PhysXDebugConfiguration.h
    Include/PhysX/Debug/PhysXDebugInterface.h
    Include/PhysX/Configuration/PhysXConfiguration.h
    Include/PhysX/Joint/Configuration/PhysXJointConfiguration.h
    Source/Common/PhysXSceneQueryHelpers.h
    Source/Common/PhysXSceneQueryHelpers.cpp
    Source/Configuration/PhysXConfiguration.cpp
    Source/Configuration/PhysXSettingsRegistryManager.h
    Source/Configuration/PhysXSettingsRegistryManager.cpp
    Source/Debug/PhysXDebug.h
    Source/Debug/PhysXDebug.cpp
    Source/Debug/Configuration/PhysXDebugConfiguration.cpp
    Source/Joint/PhysXJoint.h
    Source/Joint/PhysXJoint.cpp
    Source/Joint/PhysXJointUtils.h
    Source/Joint/PhysXJointUtils.cpp
    Source/Joint/Configuration/PhysXJointConfiguration.cpp
    Source/Scene/PhysXScene.h
    Source/Scene/PhysXScene.cpp
    Source/Scene/PhysXSceneInterface.h
    Source/Scene/PhysXSceneInterface.cpp
    Source/Scene/PhysXSceneSimulationEventCallback.h
    Source/Scene/PhysXSceneSimulationEventCallback.cpp
    Source/Scene/PhysXSceneSimulationFilterCallback.h
    Source/Scene/PhysXSceneSimulationFilterCallback.cpp
    Source/System/PhysXAllocator.h
    Source/System/PhysXAllocator.cpp
    Source/System/PhysXCookingParams.h
    Source/System/PhysXCookingParams.cpp
    Source/System/PhysXCpuDispatcher.cpp
    Source/System/PhysXCpuDispatcher.h
    Source/System/PhysXJob.cpp
    Source/System/PhysXJob.h
    Source/System/PhysXJointInterface.h
    Source/System/PhysXJointInterface.cpp
    Source/System/PhysXSdkCallbacks.h
    Source/System/PhysXSdkCallbacks.cpp
    Source/System/PhysXSystem.h
    Source/System/PhysXSystem.cpp
)
