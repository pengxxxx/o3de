#
# Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Include/GraphCanvas/Widgets/RootGraphicsItem.h
    Include/GraphCanvas/tools.h
    Include/GraphCanvas/Components/Connections/ConnectionFilters/ConnectionFilterBus.h
    Include/GraphCanvas/Components/Connections/ConnectionFilters/ConnectionFilters.h
    Include/GraphCanvas/Components/Connections/ConnectionFilters/DataConnectionFilters.h
    Source/GraphCanvas.cpp
    Source/GraphCanvas.h
    Source/GraphCanvasModule.h
    Source/GraphCanvasEditorModule.cpp
    Source/tools.cpp
    Source/Components/BookmarkManagerComponent.cpp
    Source/Components/BookmarkManagerComponent.h
    Source/Components/GeometryComponent.cpp
    Source/Components/GeometryComponent.h
    Source/Components/GridComponent.cpp
    Source/Components/GridComponent.h
    Source/Components/GridVisualComponent.cpp
    Source/Components/GridVisualComponent.h
    Source/Components/LayerControllerComponent.cpp
    Source/Components/LayerControllerComponent.h
    Source/Components/PersistentIdComponent.cpp
    Source/Components/PersistentIdComponent.h
    Source/Components/SceneComponent.cpp
    Source/Components/SceneComponent.h
    Source/Components/SceneMemberComponent.cpp
    Source/Components/SceneMemberComponent.h
    Source/Components/StylingComponent.cpp
    Source/Components/StylingComponent.h
    Source/Components/BookmarkAnchor/BookmarkAnchorComponent.cpp
    Source/Components/BookmarkAnchor/BookmarkAnchorComponent.h
    Source/Components/BookmarkAnchor/BookmarkAnchorLayerControllerComponent.h
    Source/Components/BookmarkAnchor/BookmarkAnchorVisualComponent.cpp
    Source/Components/BookmarkAnchor/BookmarkAnchorVisualComponent.h
    Source/Components/Connections/ConnectionComponent.cpp
    Source/Components/Connections/ConnectionComponent.h
    Source/Components/Connections/ConnectionLayerControllerComponent.cpp
    Source/Components/Connections/ConnectionLayerControllerComponent.h
    Source/Components/Connections/ConnectionVisualComponent.cpp
    Source/Components/Connections/ConnectionVisualComponent.h
    Source/Components/Connections/DataConnections/DataConnectionComponent.cpp
    Source/Components/Connections/DataConnections/DataConnectionComponent.h
    Source/Components/Connections/DataConnections/DataConnectionGraphicsItem.cpp
    Source/Components/Connections/DataConnections/DataConnectionGraphicsItem.h
    Source/Components/Connections/DataConnections/DataConnectionVisualComponent.cpp
    Source/Components/Connections/DataConnections/DataConnectionVisualComponent.h
    Source/Components/NodePropertyDisplays/AssetIdNodePropertyDisplay.cpp
    Source/Components/NodePropertyDisplays/AssetIdNodePropertyDisplay.h
    Source/Components/NodePropertyDisplays/BooleanNodePropertyDisplay.cpp
    Source/Components/NodePropertyDisplays/BooleanNodePropertyDisplay.h
    Source/Components/NodePropertyDisplays/ComboBoxNodePropertyDisplay.cpp
    Source/Components/NodePropertyDisplays/ComboBoxNodePropertyDisplay.h
    Source/Components/NodePropertyDisplays/NumericNodePropertyDisplay.cpp
    Source/Components/NodePropertyDisplays/NumericNodePropertyDisplay.h
    Source/Components/NodePropertyDisplays/EntityIdNodePropertyDisplay.cpp
    Source/Components/NodePropertyDisplays/EntityIdNodePropertyDisplay.h
    Source/Components/NodePropertyDisplays/ReadOnlyNodePropertyDisplay.cpp
    Source/Components/NodePropertyDisplays/ReadOnlyNodePropertyDisplay.h
    Source/Components/NodePropertyDisplays/StringNodePropertyDisplay.cpp
    Source/Components/NodePropertyDisplays/StringNodePropertyDisplay.h
    Source/Components/NodePropertyDisplays/VectorNodePropertyDisplay.cpp
    Source/Components/NodePropertyDisplays/VectorNodePropertyDisplay.h
    Source/Components/Nodes/NodeComponent.cpp
    Source/Components/Nodes/NodeComponent.h
    Source/Components/Nodes/NodeFrameGraphicsWidget.cpp
    Source/Components/Nodes/NodeFrameGraphicsWidget.h
    Source/Components/Nodes/NodeLayerControllerComponent.h
    Source/Components/Nodes/NodeLayoutComponent.h
    Source/Components/Nodes/Comment/CommentLayerControllerComponent.h
    Source/Components/Nodes/Comment/CommentNodeFrameComponent.cpp
    Source/Components/Nodes/Comment/CommentNodeFrameComponent.h
    Source/Components/Nodes/Comment/CommentNodeLayoutComponent.cpp
    Source/Components/Nodes/Comment/CommentNodeLayoutComponent.h
    Source/Components/Nodes/Comment/CommentNodeTextComponent.cpp
    Source/Components/Nodes/Comment/CommentNodeTextComponent.h
    Source/Components/Nodes/Comment/CommentTextGraphicsWidget.cpp
    Source/Components/Nodes/Comment/CommentTextGraphicsWidget.h
    Source/Components/Nodes/General/GeneralNodeFrameComponent.cpp
    Source/Components/Nodes/General/GeneralNodeFrameComponent.h
    Source/Components/Nodes/General/GeneralNodeLayoutComponent.cpp
    Source/Components/Nodes/General/GeneralNodeLayoutComponent.h
    Source/Components/Nodes/General/GeneralNodeTitleComponent.cpp
    Source/Components/Nodes/General/GeneralNodeTitleComponent.h
    Source/Components/Nodes/General/GeneralSlotLayoutComponent.cpp
    Source/Components/Nodes/General/GeneralSlotLayoutComponent.h
    Source/Components/Nodes/Group/CollapsedNodeGroupComponent.cpp
    Source/Components/Nodes/Group/CollapsedNodeGroupComponent.h
    Source/Components/Nodes/Group/NodeGroupFrameComponent.cpp
    Source/Components/Nodes/Group/NodeGroupFrameComponent.h
    Source/Components/Nodes/Group/NodeGroupLayerControllerComponent.h
    Source/Components/Nodes/Group/NodeGroupLayoutComponent.cpp
    Source/Components/Nodes/Group/NodeGroupLayoutComponent.h
    Source/Components/Nodes/Wrapper/WrapperNodeLayoutComponent.cpp
    Source/Components/Nodes/Wrapper/WrapperNodeLayoutComponent.h
    Source/Components/Slots/SlotComponent.cpp
    Source/Components/Slots/SlotComponent.h
    Source/Components/Slots/SlotConnectionFilterComponent.cpp
    Source/Components/Slots/SlotConnectionFilterComponent.h
    Source/Components/Slots/SlotConnectionPin.cpp
    Source/Components/Slots/SlotConnectionPin.h
    Source/Components/Slots/SlotLayoutComponent.cpp
    Source/Components/Slots/SlotLayoutComponent.h
    Source/Components/Slots/SlotLayoutItem.h
    Source/Components/Slots/Data/DataSlotComponent.cpp
    Source/Components/Slots/Data/DataSlotComponent.h
    Source/Components/Slots/Data/DataSlotConnectionPin.cpp
    Source/Components/Slots/Data/DataSlotConnectionPin.h
    Source/Components/Slots/Data/DataSlotLayoutComponent.cpp
    Source/Components/Slots/Data/DataSlotLayoutComponent.h
    Source/Components/Slots/Default/DefaultSlotLayoutComponent.cpp
    Source/Components/Slots/Default/DefaultSlotLayoutComponent.h
    Source/Components/Slots/Execution/ExecutionSlotComponent.cpp
    Source/Components/Slots/Execution/ExecutionSlotComponent.h
    Source/Components/Slots/Execution/ExecutionSlotConnectionPin.cpp
    Source/Components/Slots/Execution/ExecutionSlotConnectionPin.h
    Source/Components/Slots/Execution/ExecutionSlotLayoutComponent.cpp
    Source/Components/Slots/Execution/ExecutionSlotLayoutComponent.h
    Source/Components/Slots/Extender/ExtenderSlotComponent.cpp
    Source/Components/Slots/Extender/ExtenderSlotComponent.h
    Source/Components/Slots/Extender/ExtenderSlotConnectionPin.cpp
    Source/Components/Slots/Extender/ExtenderSlotConnectionPin.h
    Source/Components/Slots/Extender/ExtenderSlotLayoutComponent.cpp
    Source/Components/Slots/Extender/ExtenderSlotLayoutComponent.h
    Source/Components/Slots/Property/PropertySlotComponent.cpp
    Source/Components/Slots/Property/PropertySlotComponent.h
    Source/Components/Slots/Property/PropertySlotLayoutComponent.cpp
    Source/Components/Slots/Property/PropertySlotLayoutComponent.h
    Source/Widgets/GraphCanvasCheckBox.cpp
    Source/Widgets/GraphCanvasCheckBox.h
    Source/Widgets/GraphCanvasComboBox.cpp
    Source/Widgets/GraphCanvasComboBox.h
    Source/Widgets/GraphCanvasLabel.cpp
    Source/Widgets/GraphCanvasLabel.h
    Source/Widgets/NodePropertyDisplayWidget.cpp
    Source/Widgets/NodePropertyDisplayWidget.h
    Source/Translation/TranslationAsset.h
    Source/Translation/TranslationAsset.cpp
    Source/Translation/TranslationBuilder.h
    Source/Translation/TranslationBuilder.cpp
    Source/Translation/TranslationBus.h
    Source/Translation/TranslationDatabase.h
    Source/Translation/TranslationDatabase.cpp
    Source/Translation/TranslationSerializer.h
    Source/Translation/TranslationSerializer.cpp

)
