#
# Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Include/WhiteBox/EditorWhiteBoxBus.h
    Include/WhiteBox/EditorWhiteBoxComponentBus.h
    Include/WhiteBox/WhiteBoxToolApi.h
    Include/WhiteBox/EditorWhiteBoxColliderBus.h
    Source/EditorWhiteBoxComponent.cpp
    Source/EditorWhiteBoxComponent.h
    Source/EditorWhiteBoxComponentMode.cpp
    Source/EditorWhiteBoxComponentMode.h
    Source/EditorWhiteBoxComponentModeBus.h
    Source/EditorWhiteBoxComponentModeTypes.cpp
    Source/EditorWhiteBoxComponentModeTypes.h
    Source/EditorWhiteBoxEdgeModifierBus.h
    Source/EditorWhiteBoxPolygonModifierBus.h
    Source/EditorWhiteBoxDefaultShapeTypes.h
    Source/WhiteBoxToolApiReflection.cpp
    Source/WhiteBoxToolApiReflection.h
    Source/Core/WhiteBoxToolApi.cpp
    Source/Components/EditorWhiteBoxColliderComponent.cpp
    Source/Components/EditorWhiteBoxColliderComponent.h
    Source/Util/WhiteBoxTextureUtil.cpp
    Source/Util/WhiteBoxTextureUtil.h
    Source/Util/WhiteBoxEditorUtil.cpp
    Source/Util/WhiteBoxEditorUtil.h
    Source/Viewport/WhiteBoxEdgeScaleModifier.cpp
    Source/Viewport/WhiteBoxEdgeScaleModifier.h
    Source/Viewport/WhiteBoxEdgeTranslationModifier.cpp
    Source/Viewport/WhiteBoxEdgeTranslationModifier.h
    Source/Viewport/WhiteBoxPolygonScaleModifier.cpp
    Source/Viewport/WhiteBoxPolygonScaleModifier.h
    Source/Viewport/WhiteBoxPolygonTranslationModifier.cpp
    Source/Viewport/WhiteBoxPolygonTranslationModifier.h
    Source/Viewport/WhiteBoxManipulatorViews.cpp
    Source/Viewport/WhiteBoxManipulatorViews.h
    Source/Viewport/WhiteBoxManipulatorBounds.cpp
    Source/Viewport/WhiteBoxManipulatorBounds.h
    Source/Viewport/WhiteBoxModifierUtil.cpp
    Source/Viewport/WhiteBoxModifierUtil.h
    Source/Viewport/WhiteBoxViewportConstants.cpp
    Source/Viewport/WhiteBoxViewportConstants.h
    Source/Viewport/WhiteBoxVertexTranslationModifier.cpp
    Source/Viewport/WhiteBoxVertexTranslationModifier.h
    Source/Asset/EditorWhiteBoxMeshAsset.cpp
    Source/Asset/EditorWhiteBoxMeshAsset.h
    Source/Asset/WhiteBoxMeshAsset.h
    Source/Asset/WhiteBoxMeshAssetHandler.h
    Source/Asset/WhiteBoxMeshAssetHandler.cpp
    Source/Asset/WhiteBoxMeshAssetUndoCommand.h
    Source/Asset/WhiteBoxMeshAssetUndoCommand.cpp
    Source/Asset/WhiteBoxMeshAssetBus.h
    Source/SubComponentModes/EditorWhiteBoxEdgeRestoreMode.cpp
    Source/SubComponentModes/EditorWhiteBoxEdgeRestoreMode.h
    Source/SubComponentModes/EditorWhiteBoxDefaultMode.cpp
    Source/SubComponentModes/EditorWhiteBoxDefaultMode.h
    Source/SubComponentModes/EditorWhiteBoxDefaultModeBus.h
    Source/SubComponentModes/EditorWhiteBoxComponentModeCommon.cpp
    Source/SubComponentModes/EditorWhiteBoxComponentModeCommon.h
    Source/EditorWhiteBoxSystemComponent.cpp
    Source/EditorWhiteBoxSystemComponent.h
)
