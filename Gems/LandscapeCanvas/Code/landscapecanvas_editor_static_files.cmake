#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Include/LandscapeCanvas/LandscapeCanvasBus.h
    Source/LandscapeCanvasSystemComponent.cpp
    Source/LandscapeCanvasSystemComponent.h
    Source/Editor/MainWindow.cpp
    Source/Editor/MainWindow.h
    Source/Editor/Core/Core.h
    Source/Editor/Core/DataTypes.cpp
    Source/Editor/Core/DataTypes.h
    Source/Editor/Core/GraphContext.cpp
    Source/Editor/Core/GraphContext.h
    Source/Editor/Menus/LayerExtenderContextMenu.cpp
    Source/Editor/Menus/LayerExtenderContextMenu.h
    Source/Editor/Menus/NodeContextMenu.cpp
    Source/Editor/Menus/NodeContextMenu.h
    Source/Editor/Menus/SceneContextMenuActions.cpp
    Source/Editor/Menus/SceneContextMenuActions.h
    Source/Editor/Nodes/BaseNode.cpp
    Source/Editor/Nodes/BaseNode.h
    Source/Editor/Nodes/Areas/AreaBlenderNode.cpp
    Source/Editor/Nodes/Areas/AreaBlenderNode.h
    Source/Editor/Nodes/Areas/BaseAreaNode.cpp
    Source/Editor/Nodes/Areas/BaseAreaNode.h
    Source/Editor/Nodes/Areas/BlockerAreaNode.cpp
    Source/Editor/Nodes/Areas/BlockerAreaNode.h
    Source/Editor/Nodes/Areas/MeshBlockerAreaNode.cpp
    Source/Editor/Nodes/Areas/MeshBlockerAreaNode.h
    Source/Editor/Nodes/Areas/SpawnerAreaNode.cpp
    Source/Editor/Nodes/Areas/SpawnerAreaNode.h
    Source/Editor/Nodes/AreaFilters/AltitudeFilterNode.cpp
    Source/Editor/Nodes/AreaFilters/AltitudeFilterNode.h
    Source/Editor/Nodes/AreaFilters/BaseAreaFilterNode.cpp
    Source/Editor/Nodes/AreaFilters/BaseAreaFilterNode.h
    Source/Editor/Nodes/AreaFilters/DistanceBetweenFilterNode.cpp
    Source/Editor/Nodes/AreaFilters/DistanceBetweenFilterNode.h
    Source/Editor/Nodes/AreaFilters/DistributionFilterNode.cpp
    Source/Editor/Nodes/AreaFilters/DistributionFilterNode.h
    Source/Editor/Nodes/AreaFilters/ShapeIntersectionFilterNode.cpp
    Source/Editor/Nodes/AreaFilters/ShapeIntersectionFilterNode.h
    Source/Editor/Nodes/AreaFilters/SlopeFilterNode.cpp
    Source/Editor/Nodes/AreaFilters/SlopeFilterNode.h
    Source/Editor/Nodes/AreaFilters/SurfaceMaskDepthFilterNode.cpp
    Source/Editor/Nodes/AreaFilters/SurfaceMaskDepthFilterNode.h
    Source/Editor/Nodes/AreaFilters/SurfaceMaskFilterNode.cpp
    Source/Editor/Nodes/AreaFilters/SurfaceMaskFilterNode.h
    Source/Editor/Nodes/AreaModifiers/BaseAreaModifierNode.cpp
    Source/Editor/Nodes/AreaModifiers/BaseAreaModifierNode.h
    Source/Editor/Nodes/AreaModifiers/PositionModifierNode.cpp
    Source/Editor/Nodes/AreaModifiers/PositionModifierNode.h
    Source/Editor/Nodes/AreaModifiers/RotationModifierNode.cpp
    Source/Editor/Nodes/AreaModifiers/RotationModifierNode.h
    Source/Editor/Nodes/AreaModifiers/ScaleModifierNode.cpp
    Source/Editor/Nodes/AreaModifiers/ScaleModifierNode.h
    Source/Editor/Nodes/AreaModifiers/SlopeAlignmentModifierNode.cpp
    Source/Editor/Nodes/AreaModifiers/SlopeAlignmentModifierNode.h
    Source/Editor/Nodes/AreaSelectors/AssetWeightSelectorNode.cpp
    Source/Editor/Nodes/AreaSelectors/AssetWeightSelectorNode.h
    Source/Editor/Nodes/Gradients/AltitudeGradientNode.cpp
    Source/Editor/Nodes/Gradients/AltitudeGradientNode.h
    Source/Editor/Nodes/Gradients/BaseGradientNode.cpp
    Source/Editor/Nodes/Gradients/BaseGradientNode.h
    Source/Editor/Nodes/Gradients/ConstantGradientNode.cpp
    Source/Editor/Nodes/Gradients/ConstantGradientNode.h
    Source/Editor/Nodes/Gradients/FastNoiseGradientNode.cpp
    Source/Editor/Nodes/Gradients/FastNoiseGradientNode.h
    Source/Editor/Nodes/Gradients/ImageGradientNode.cpp
    Source/Editor/Nodes/Gradients/ImageGradientNode.h
    Source/Editor/Nodes/Gradients/PerlinNoiseGradientNode.cpp
    Source/Editor/Nodes/Gradients/PerlinNoiseGradientNode.h
    Source/Editor/Nodes/Gradients/RandomNoiseGradientNode.cpp
    Source/Editor/Nodes/Gradients/RandomNoiseGradientNode.h
    Source/Editor/Nodes/Gradients/ShapeAreaFalloffGradientNode.cpp
    Source/Editor/Nodes/Gradients/ShapeAreaFalloffGradientNode.h
    Source/Editor/Nodes/Gradients/SlopeGradientNode.cpp
    Source/Editor/Nodes/Gradients/SlopeGradientNode.h
    Source/Editor/Nodes/Gradients/SurfaceMaskGradientNode.cpp
    Source/Editor/Nodes/Gradients/SurfaceMaskGradientNode.h
    Source/Editor/Nodes/GradientModifiers/BaseGradientModifierNode.cpp
    Source/Editor/Nodes/GradientModifiers/BaseGradientModifierNode.h
    Source/Editor/Nodes/GradientModifiers/DitherGradientModifierNode.cpp
    Source/Editor/Nodes/GradientModifiers/DitherGradientModifierNode.h
    Source/Editor/Nodes/GradientModifiers/GradientMixerNode.cpp
    Source/Editor/Nodes/GradientModifiers/GradientMixerNode.h
    Source/Editor/Nodes/GradientModifiers/InvertGradientModifierNode.cpp
    Source/Editor/Nodes/GradientModifiers/InvertGradientModifierNode.h
    Source/Editor/Nodes/GradientModifiers/LevelsGradientModifierNode.cpp
    Source/Editor/Nodes/GradientModifiers/LevelsGradientModifierNode.h
    Source/Editor/Nodes/GradientModifiers/PosterizeGradientModifierNode.cpp
    Source/Editor/Nodes/GradientModifiers/PosterizeGradientModifierNode.h
    Source/Editor/Nodes/GradientModifiers/SmoothStepGradientModifierNode.cpp
    Source/Editor/Nodes/GradientModifiers/SmoothStepGradientModifierNode.h
    Source/Editor/Nodes/GradientModifiers/ThresholdGradientModifierNode.cpp
    Source/Editor/Nodes/GradientModifiers/ThresholdGradientModifierNode.h
    Source/Editor/Nodes/Shapes/BaseShapeNode.cpp
    Source/Editor/Nodes/Shapes/BaseShapeNode.h
    Source/Editor/Nodes/Shapes/BoxShapeNode.cpp
    Source/Editor/Nodes/Shapes/BoxShapeNode.h
    Source/Editor/Nodes/Shapes/CapsuleShapeNode.cpp
    Source/Editor/Nodes/Shapes/CapsuleShapeNode.h
    Source/Editor/Nodes/Shapes/CompoundShapeNode.cpp
    Source/Editor/Nodes/Shapes/CompoundShapeNode.h
    Source/Editor/Nodes/Shapes/CylinderShapeNode.cpp
    Source/Editor/Nodes/Shapes/CylinderShapeNode.h
    Source/Editor/Nodes/Shapes/DiskShapeNode.cpp
    Source/Editor/Nodes/Shapes/DiskShapeNode.h
    Source/Editor/Nodes/Shapes/PolygonPrismShapeNode.cpp
    Source/Editor/Nodes/Shapes/PolygonPrismShapeNode.h
    Source/Editor/Nodes/Shapes/SphereShapeNode.cpp
    Source/Editor/Nodes/Shapes/SphereShapeNode.h
    Source/Editor/Nodes/Shapes/TubeShapeNode.cpp
    Source/Editor/Nodes/Shapes/TubeShapeNode.h
    Source/Editor/Nodes/UI/GradientPreviewThumbnailItem.cpp
    Source/Editor/Nodes/UI/GradientPreviewThumbnailItem.h
)
