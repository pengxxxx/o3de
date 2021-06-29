#
# Copyright (c) Contributors to the Open 3D Engine Project
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Tests/BlendTreeParameterNodeTests.cpp
    Tests/CommandRemoveMotionTests.cpp
    Tests/EMotionFXTest.cpp
    Tests/InitSceneAPIFixture.h
    Tests/MetaDataRuleTests.cpp
    Tests/MorphTargetPipelineTests.cpp
    Tests/Printers.cpp
    Tests/PhysicsSetupUtils.h
    Tests/PhysicsSetupUtils.cpp
    Tests/SimulatedObjectModelTests.cpp
    Tests/SimulatedObjectPipelineTests.cpp
    Tests/SystemComponentFixture.h
    Tests/Bugs/CanDeleteMotionSetWhenSameMotionInTwoMotionSets.cpp
    Tests/UI/CanAddSimulatedObject.cpp
    Tests/UI/CanAddToSimulatedObject.cpp
    Tests/UI/CanAddAnimGraph.cpp
    Tests/UI/CanAddReferenceNode.cpp
    Tests/UI/CanAddMotionToAnimGraphNode.cpp
    Tests/UI/CanAddMotionToMotionSet.cpp
    Tests/UI/CanChangeParametersInSimulatedObject.cpp
    Tests/UI/CanEditParameters.cpp
    Tests/UI/CanMorphManyShapes.cpp
    Tests/UI/CanSeeJoints.cpp
    Tests/UI/CanAutoSaveFile.cpp
    Tests/UI/CanRemoveMotionFromMotionSet.cpp
    Tests/UI/CanUseLayoutMenu.cpp
    Tests/UI/CanUseViewMenu.cpp
    Tests/UI/CanUseFileMenu.cpp
    Tests/UI/ClothColliderTests.cpp
    Tests/UI/CanUseEditMenu.cpp
    Tests/UI/CanOpenWorkspace.cpp
    Tests/UI/CanUseHelpMenu.cpp
    Tests/UI/CommandRunnerFixture.cpp
    Tests/UI/CommandRunnerFixture.h
    Tests/UI/LODSkinnedMeshTests.cpp
    Tests/UI/RagdollEditTests.cpp
    Tests/UI/UIFixture.cpp
    Tests/UI/UIFixture.h
    Tests/UI/ModalPopupHandler.cpp
    Tests/UI/ModalPopupHandler.h
    Tests/UI/AnimGraphUIFixture.cpp
    Tests/UI/AnimGraphUIFixture.h
    Tests/UI/MenuUIFixture.cpp
    Tests/UI/MenuUIFixture.h
    Tests/UI/CanRenameParameter_ParameterNodeUpdates.cpp
    Tests/UI/CanDeleteAnimGraphNode_AnimGraphModelUpdates.cpp
    Tests/UI/CanAdjustGroupParameter.cpp
    Tests/UI/CanAddJointAndChildren.cpp
    Tests/Integration/CanAddActor.cpp
    Tests/Integration/CanAddSimpleMotionComponent.cpp
    Tests/Integration/CanDeleteJackEntity.cpp
    Tests/Bugs/CanDeleteMotionWhenMotionIsBeingBlended.cpp
    Tests/Bugs/CanUndoParameterDeletionAndRestoreBlendTreeConnections.cpp
    Tests/Editor/FileManagerTests.cpp
    Tests/Editor/ParametersGroupDefaultValues.cpp
    Tests/Editor/MotionSetLoadEscalation.cpp
    Tests/ProvidesUI/AnimGraph/AnimGraphActivateTests.cpp
    Tests/ProvidesUI/AnimGraph/SimpleAnimGraphUIFixture.cpp
    Tests/ProvidesUI/AnimGraph/SimpleAnimGraphUIFixture.h
    Tests/ProvidesUI/AnimGraph/AnimGraphModelTests.cpp
    Tests/ProvidesUI/AnimGraph/ParameterWindowTests.cpp
    Tests/ProvidesUI/AnimGraph/AnimGraphPreviewMotionTests.cpp
    Tests/ProvidesUI/AnimGraph/CanDeleteAnimGraphNode.cpp
    Tests/ProvidesUI/AnimGraph/AnimGraphNodeTests.cpp
    Tests/ProvidesUI/AnimGraph/CanEditAnimGraphNode.cpp
    Tests/ProvidesUI/AnimGraph/PreviewMotionFixture.cpp
    Tests/ProvidesUI/AnimGraph/PreviewMotionFixture.h
    Tests/ProvidesUI/AnimGraph/Parameters/AddGroup.cpp
    Tests/ProvidesUI/AnimGraph/Parameters/AddParameter.cpp
    Tests/ProvidesUI/AnimGraph/Parameters/CannotAssignGroupsParentAsChild.cpp
    Tests/ProvidesUI/AnimGraph/Parameters/RemoveParameter.cpp
    Tests/ProvidesUI/AnimGraph/Parameters/RemoveGroup.cpp
    Tests/ProvidesUI/AnimGraph/Parameters/ParameterGroupEdit.cpp
    Tests/ProvidesUI/AnimGraph/Transitions/RemoveTransition.cpp
    Tests/ProvidesUI/AnimGraph/Transitions/AddTransition.cpp
    Tests/ProvidesUI/AnimGraph/Transitions/AddTransitionCondition.cpp
    Tests/ProvidesUI/AnimGraph/Transitions/EditTransition.cpp
    Tests/ProvidesUI/AnimGraph/Transitions/RemoveTransitionCondition.cpp
    Tests/ProvidesUI/AnimGraph/StateMachine/EntryStateTests.cpp
    Tests/ProvidesUI/Menus/FileMenu/CanReset.cpp
    Tests/ProvidesUI/MotionSet/CanCreateMotionSet.cpp
    Tests/ProvidesUI/MotionSet/CanRemoveMotionSet.cpp
    Tests/ProvidesUI/Motions/CanAddMotions.cpp
    Tests/ProvidesUI/Motions/CanRemoveMotions.cpp
    Tests/ProvidesUI/Motions/MotionPlaybacksTests.cpp
    Tests/ProvidesUI/Ragdoll/CanCopyPasteJointLimits.cpp
    Tests/ProvidesUI/Ragdoll/CanCopyPasteColliders.cpp
    Tests/EMotionFXBuilderTests.cpp
    Tests/EMotionFXBuilderFixture.h
    Tests/EMotionFXBuilderFixture.cpp
    Tests/TestAssetCode/TestActorAssets.h
    Tests/TestAssetCode/TestActorAssets.cpp
    Tests/Mocks/PhysicsSystem.h
)
