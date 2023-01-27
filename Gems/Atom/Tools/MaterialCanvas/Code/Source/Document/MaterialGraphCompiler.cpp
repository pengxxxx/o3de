/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RHI.Reflect/SamplerState.h>
#include <Atom/RPI.Edit/Common/AssetUtils.h>
#include <Atom/RPI.Edit/Common/JsonUtils.h>
#include <Atom/RPI.Edit/Material/MaterialTypeSourceData.h>
#include <Atom/RPI.Edit/Material/MaterialUtils.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <AtomToolsFramework/Graph/DynamicNode/DynamicNode.h>
#include <AtomToolsFramework/Graph/DynamicNode/DynamicNodeUtil.h>
#include <AtomToolsFramework/Graph/GraphTemplateFileDataCacheRequestBus.h>
#include <AtomToolsFramework/Graph/GraphUtil.h>
#include <AtomToolsFramework/Util/MaterialPropertyUtil.h>
#include <AtomToolsFramework/Util/Util.h>
#include <AzCore/Jobs/Algorithms.h>
#include <AzCore/Math/Color.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Vector4.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/ObjectStream.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/Utils.h>
#include <AzCore/Utils/Utils.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/sort.h>
#include <AzCore/std/string/regex.h>
#include <Document/MaterialGraphCompiler.h>
#include <GraphModel/Model/Connection.h>

namespace MaterialCanvas
{
    void MaterialGraphCompiler::Reflect(AZ::ReflectContext* context)
    {
        if (auto serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MaterialGraphCompiler, AtomToolsFramework::GraphCompiler>()
                ->Version(0)
                ;
        }
    }

    MaterialGraphCompiler::MaterialGraphCompiler(const AZ::Crc32& toolId)
        : AtomToolsFramework::GraphCompiler(toolId)
    {
    }

    MaterialGraphCompiler::~MaterialGraphCompiler()
    {
    }

    AZStd::string MaterialGraphCompiler::GetGraphPath() const
    {
        if (const auto& graphPath = AtomToolsFramework::GraphCompiler::GetGraphPath(); graphPath.ends_with(".materialgraph"))
        {
            return graphPath;
        }

        return AZStd::string::format("%s/Assets/Materials/Generated/untitled.materialgraph", AZ::Utils::GetProjectPath().c_str());
    }

    bool MaterialGraphCompiler::CompileGraph(GraphModel::GraphPtr graph, const AZStd::string& graphName, const AZStd::string& graphPath)
    {
        if (!AtomToolsFramework::GraphCompiler::CompileGraph(graph, graphName, graphPath))
        {
            return false;
        }

        m_includePaths.clear();
        m_classDefinitions.clear();
        m_functionDefinitions.clear();
        m_configIdsVisited.clear();
        m_slotValueTable.clear();
        m_templatePathsForCurrentNode.clear();
        m_templateFileDataVecForCurrentNode.clear();
        m_instructionNodesForCurrentNode.clear();
        BuildSlotValueTable();
        BuildDependencyTables();

        // Traverse all graph nodes and slots searching for settings to generate files from templates
        for (const auto& currentNode : GetAllNodesInExecutionOrder())
        {
            // Search this node for any template path settings that describe files that need to be generated from the graph.
            BuildTemplatePathsForCurrentNode(currentNode);

            // If no template files were specified for this node then skip additional processing and continue to the next one.
            if (m_templatePathsForCurrentNode.empty())
            {
                continue;
            }

            // Attempt to load all of the template files referenced by this node. All of the template data will be tokenized into individual
            // lines and stored in a container so then multiple passes can be made on each file, substituting tokens and filling in
            // details provided by the graph. None of the files generated from this node will be saved until they have all been processed.
            // Template files for material types will be processed in their own pass Because they require special handling and need to be
            // saved before material file templates to not trigger asset processor dependency errors.
            if (!LoadTemplatesForCurrentNode())
            {
                SetState(State::Failed);
                return false;
            }

            // Perform an initial pass over all template files, injecting include files, class definitions, function definitions, simple
            // things that don't require much processing.
            PreprocessTemplatesForCurrentNode();

            // The next phase injects shader code instructions assembled by traversing the graph from each of the input slots on the current
            // node. The O3DE_GENERATED_INSTRUCTIONS_BEGIN marker will be followed by a list of input slot names corresponding to required
            // variables in the shader. Instructions will only be generated for the current node and nodes connected to the specified
            // inputs. This will allow multiple O3DE_GENERATED_INSTRUCTIONS blocks with different inputs to be specified in multiple
            // locations across multiple files from a single graph.

            // This will also keep track of nodes with instructions and data that contribute to the final shader code. The list of
            // contributing nodes will be used to exclude unused material inputs from generated SRGs and material types.
            BuildInstructionsForCurrentNode(currentNode);

            // At this point, all of the instructions have been generated for all of the template files used by this node. We now also have
            // a complete list of all nodes that contributed instructions to the final shader code across all of the files. Now, we can
            // safely generate the material SRG and material type that only contain variables referenced in the shaders. Without tracking
            // this, all variables would be included in the SRG and material type. The shader compiler would eliminate unused variables from
            // the compiled shader code. The material type would fail to build if it referenced any of the eliminated variables.
            BuildMaterialSrgForCurrentNode();

            // Save all of the generated files except for materials and material types. Generated material type files must be saved after
            // generated shader files to prevent AP errors because of missing dependencies.
            if (!ExportTemplatesMatchingRegex(".*\\.lua\\b") ||
                !ExportTemplatesMatchingRegex(".*\\.azsli\\b") ||
                !ExportTemplatesMatchingRegex(".*\\.azsl\\b") ||
                !ExportTemplatesMatchingRegex(".*\\.shader\\b"))
            {
                SetState(State::Failed);
                return false;
            }

            // Process material type template files, injecting properties from material input nodes.
            if (!BuildMaterialTypeForCurrentNode(currentNode))
            {
                SetState(State::Failed);
                return false;
            }

            // After the material types have been processed and saved, save the materials that reference them.
            if (!ExportTemplatesMatchingRegex(".*\\.material\\b"))
            {
                SetState(State::Failed);
                return false;
            }
        }

        if (!ReportGeneratedFileStatus())
        {
            SetState(State::Failed);
            return false;
        }

        SetState(State::Complete);
        return true;
    }

    void MaterialGraphCompiler::BuildSlotValueTable()
    {
        // Build a table of all values for every slot in the graph.
        m_slotValueTable.clear();
        for (const auto& currentNode : GetAllNodesInExecutionOrder())
        {
            for (const auto& currentSlotPair : currentNode->GetSlots())
            {
                const auto& currentSlot = currentSlotPair.second;
                m_slotValueTable[currentSlot] = currentSlot->GetValue();
            }

            // If this is a dynamic node with slot data type groups, we will search for the largest vector or other data type and convert
            // all of the values in the group to the same type.
            if (auto dynamicNode = azrtti_cast<const AtomToolsFramework::DynamicNode*>(currentNode.get()))
            {
                const auto& nodeConfig = dynamicNode->GetConfig();
                for (const auto& slotDataTypeGroup : nodeConfig.m_slotDataTypeGroups)
                {
                    unsigned int vectorSize = 0;

                    // The slot data group string is separated by vertical bars and can be treated like a regular expression to compare
                    // against slot names. The largest vector size is recorded for each slot group.
                    const AZStd::regex slotDataTypeGroupRegex(slotDataTypeGroup, AZStd::regex::flag_type::icase);
                    for (const auto& currentSlotPair : currentNode->GetSlots())
                    {
                        const auto& currentSlot = currentSlotPair.second;
                        if (currentSlot->GetSlotDirection() == GraphModel::SlotDirection::Input &&
                            AZStd::regex_match(currentSlot->GetName(), slotDataTypeGroupRegex))
                        {
                            const auto& currentSlotValue = GetValueFromSlotOrConnection(currentSlot);
                            vectorSize = AZStd::max(vectorSize, GetVectorSize(currentSlotValue));
                        }
                    }

                    // Once all of the container sizes have been recorded for each slot data group, iterate over all of these slot values
                    // and upgrade entries in the map to the bigger type.
                    for (const auto& currentSlotPair : currentNode->GetSlots())
                    {
                        const auto& currentSlot = currentSlotPair.second;
                        if (AZStd::regex_match(currentSlot->GetName(), slotDataTypeGroupRegex))
                        {
                            const auto& currentSlotValue = GetValueFromSlot(currentSlot);
                            m_slotValueTable[currentSlot] = ConvertToVector(currentSlotValue, vectorSize);
                        }
                    }
                }
            }
        }
    }

    void MaterialGraphCompiler::BuildDependencyTables()
    {
        for (const auto& nodePair : m_graph->GetNodes())
        {
            const auto& currentNode = nodePair.second;

            if (auto dynamicNode = azrtti_cast<const AtomToolsFramework::DynamicNode*>(currentNode.get()))
            {
                if (!m_configIdsVisited.contains(dynamicNode->GetConfig().m_id))
                {
                    m_configIdsVisited.insert(dynamicNode->GetConfig().m_id);
                    AtomToolsFramework::VisitDynamicNodeSettings(
                        dynamicNode->GetConfig(),
                        [&](const AtomToolsFramework::DynamicNodeSettingsMap& settings)
                        {
                            AtomToolsFramework::CollectDynamicNodeSettings(settings, "includePaths", m_includePaths);
                            AtomToolsFramework::CollectDynamicNodeSettings(settings, "classDefinitions", m_classDefinitions);
                            AtomToolsFramework::CollectDynamicNodeSettings(settings, "functionDefinitions", m_functionDefinitions);
                        });
                }
            }
        }
    }

    void MaterialGraphCompiler::BuildTemplatePathsForCurrentNode(const GraphModel::ConstNodePtr& currentNode)
    {
        m_templatePathsForCurrentNode.clear();
        if (auto dynamicNode = azrtti_cast<const AtomToolsFramework::DynamicNode*>(currentNode.get()))
        {
            AtomToolsFramework::VisitDynamicNodeSettings(
                dynamicNode->GetConfig(),
                [&](const AtomToolsFramework::DynamicNodeSettingsMap& settings)
                {
                    AtomToolsFramework::CollectDynamicNodeSettings(settings, "templatePaths", m_templatePathsForCurrentNode);
                });
        }
    }

    bool MaterialGraphCompiler::LoadTemplatesForCurrentNode()
    {
        m_templateFileDataVecForCurrentNode.clear();

        for (const auto& templatePath : m_templatePathsForCurrentNode)
        {
            if (!templatePath.ends_with(".materialtype"))
            {
                // Load the unmodified, template source file data, which will be copied and used for insertions, substitutions, and
                // code generation.
                AtomToolsFramework::GraphTemplateFileData templateFileData;
                AtomToolsFramework::GraphTemplateFileDataCacheRequestBus::EventResult(
                    templateFileData,
                    m_toolId,
                    &AtomToolsFramework::GraphTemplateFileDataCacheRequestBus::Events::Load,
                    AtomToolsFramework::GetPathWithoutAlias(templatePath));

                if (!templateFileData.IsLoaded())
                {
                    m_templateFileDataVecForCurrentNode.clear();
                    return false;
                }

                m_templateFileDataVecForCurrentNode.emplace_back(AZStd::move(templateFileData));
            }
        }
        return true;
    }

    void MaterialGraphCompiler::PreprocessTemplatesForCurrentNode()
    {
        AZ::parallel_for_each(
            m_templateFileDataVecForCurrentNode.begin(),
            m_templateFileDataVecForCurrentNode.end(),
            [&](auto& templateFileData)
            {
                // Substitute all references to the placeholder graph name with one generated from the document name
                templateFileData.ReplaceSymbol("MaterialGraphName", m_graphName);

                // Inject include files found while traversing the graph into any include file blocks in the template.
                templateFileData.ReplaceLinesInBlock(
                    "O3DE_GENERATED_INCLUDES_BEGIN",
                    "O3DE_GENERATED_INCLUDES_END",
                    [&, this]([[maybe_unused]] const AZStd::string& blockHeader)
                    {
                        // Include file paths will need to be converted to include statements.
                        AZStd::vector<AZStd::string> includeStatements;
                        includeStatements.reserve(m_includePaths.size());

                        for (const auto& path : m_includePaths)
                        {
                            bool relativePathFound = false;
                            AZStd::string relativePath;
                            AZStd::string relativePathFolder;

                            AzToolsFramework::AssetSystemRequestBus::BroadcastResult(
                                relativePathFound,
                                &AzToolsFramework::AssetSystem::AssetSystemRequest::GenerateRelativeSourcePath,
                                AtomToolsFramework::GetPathWithoutAlias(path),
                                relativePath,
                                relativePathFolder);

                            if (relativePathFound)
                            {
                                includeStatements.push_back(AZStd::string::format("#include <%s>", relativePath.c_str()));
                            }
                        }
                        return includeStatements;
                    });

                // Inject class definitions found while traversing the graph.
                templateFileData.ReplaceLinesInBlock(
                    "O3DE_GENERATED_CLASSES_BEGIN",
                    "O3DE_GENERATED_CLASSES_END",
                    [&]([[maybe_unused]] const AZStd::string& blockHeader)
                    {
                        return m_classDefinitions;
                    });

                // Inject function definitions found while traversing the graph.
                templateFileData.ReplaceLinesInBlock(
                    "O3DE_GENERATED_FUNCTIONS_BEGIN",
                    "O3DE_GENERATED_FUNCTIONS_END",
                    [&]([[maybe_unused]] const AZStd::string& blockHeader)
                    {
                        return m_functionDefinitions;
                    });
            });
    }

    void MaterialGraphCompiler::BuildInstructionsForCurrentNode(const GraphModel::ConstNodePtr& currentNode)
    {
        m_instructionNodesForCurrentNode.clear();
        m_instructionNodesForCurrentNode.reserve(m_graph->GetNodeCount());

        AZ::parallel_for_each(
            m_templateFileDataVecForCurrentNode.begin(),
            m_templateFileDataVecForCurrentNode.end(),
            [&](auto& templateFileData)
            {
                templateFileData.ReplaceLinesInBlock(
                    "O3DE_GENERATED_INSTRUCTIONS_BEGIN",
                    "O3DE_GENERATED_INSTRUCTIONS_END",
                    [&]([[maybe_unused]] const AZStd::string& blockHeader)
                    {
                        AZStd::vector<AZStd::string> inputSlotNames;
                        AZ::StringFunc::Tokenize(blockHeader, inputSlotNames, ";:, \t\r\n\\/", false, false);

                        AZStd::vector<GraphModel::ConstNodePtr> instructionNodesForBlock;
                        instructionNodesForBlock.reserve(m_graph->GetNodeCount());
                        const auto& lines = GetInstructionsFromConnectedNodes(currentNode, inputSlotNames, instructionNodesForBlock);

                        // Adding all of the contributing notes from this blog to the set of all nodes for all blocks.
                        AZStd::scoped_lock lock(m_instructionNodesForCurrentNodeMutex);
                        m_instructionNodesForCurrentNode.insert(
                            m_instructionNodesForCurrentNode.end(), instructionNodesForBlock.begin(), instructionNodesForBlock.end());
                        return lines;
                    });
            });

        // All of the instruction nodes are gathered in temporary vectors and the results concatenated. The vector needs to be reduced
        // to only contain unique nodes and then resorted by depth.
        AZStd::sort(m_instructionNodesForCurrentNode.begin(), m_instructionNodesForCurrentNode.end());
        m_instructionNodesForCurrentNode.erase(
            AZStd::unique(m_instructionNodesForCurrentNode.begin(), m_instructionNodesForCurrentNode.end()),
            m_instructionNodesForCurrentNode.end());
        AtomToolsFramework::SortNodesInExecutionOrder(m_instructionNodesForCurrentNode);
    }

    void MaterialGraphCompiler::BuildMaterialSrgForCurrentNode()
    {
        AZ::parallel_for_each(
            m_templateFileDataVecForCurrentNode.begin(),
            m_templateFileDataVecForCurrentNode.end(),
            [&](auto& templateFileData)
            {
                templateFileData.ReplaceLinesInBlock(
                    "O3DE_GENERATED_MATERIAL_SRG_BEGIN",
                    "O3DE_GENERATED_MATERIAL_SRG_END",
                    [&]([[maybe_unused]] const AZStd::string& blockHeader)
                    {
                        return GetMaterialPropertySrgMemberFromNodes(m_instructionNodesForCurrentNode);
                    });
            });
    }

    bool MaterialGraphCompiler::BuildMaterialTypeForCurrentNode(const GraphModel::ConstNodePtr& currentNode)
    {
        for (const auto& templatePath : m_templatePathsForCurrentNode)
        {
            if (!templatePath.ends_with(".materialtype"))
            {
                continue;
            }

            // Remove any aliases to resolve the absolute path to the template file
            const auto& templateInputPath = AtomToolsFramework::GetPathWithoutAlias(templatePath);
            const auto& templateOutputPath = GetOutputPathFromTemplatePath(templateInputPath);
            if (!BuildMaterialTypeFromTemplate(currentNode, m_instructionNodesForCurrentNode, templateInputPath, templateOutputPath))
            {
                return false;
            }

            AzFramework::AssetSystemRequestBus::Broadcast(
                &AzFramework::AssetSystem::AssetSystemRequests::EscalateAssetBySearchTerm, templateOutputPath);
            m_generatedFiles.push_back(templateOutputPath);
        }
        return true;
    }

    bool MaterialGraphCompiler::ExportTemplatesMatchingRegex(const AZStd::string& pattern)
    {
        const AZStd::regex patternRegex(pattern, AZStd::regex::flag_type::icase);
        for (const auto& templateFileData : m_templateFileDataVecForCurrentNode)
        {
            if (AZStd::regex_match(templateFileData.GetPath(), patternRegex))
            {
                const auto& templateOutputPath = GetOutputPathFromTemplatePath(templateFileData.GetPath());
                if (!templateFileData.Save(templateOutputPath))
                {
                    return false;
                }

                AzFramework::AssetSystemRequestBus::Broadcast(
                    &AzFramework::AssetSystem::AssetSystemRequests::EscalateAssetBySearchTerm, templateOutputPath);
                m_generatedFiles.push_back(templateOutputPath);
            }
        }
        return true;
    }

    AZStd::string MaterialGraphCompiler::GetOutputPathFromTemplatePath(const AZStd::string& templateInputPath) const
    {
        AZStd::string templateInputFileName;
        AZ::StringFunc::Path::GetFullFileName(templateInputPath.c_str(), templateInputFileName);

        AZStd::string templateOutputPath = GetGraphPath();
        AZ::StringFunc::Path::ReplaceFullName(templateOutputPath, templateInputFileName.c_str());

        AZ::StringFunc::Replace(templateOutputPath, "MaterialGraphName", m_graphName.c_str());
        return templateOutputPath;
    }

    unsigned int MaterialGraphCompiler::GetVectorSize(const AZStd::any& slotValue) const
    {
        if (slotValue.is<AZ::Color>())
        {
            return 4;
        }
        if (slotValue.is<AZ::Vector4>())
        {
            return 4;
        }
        if (slotValue.is<AZ::Vector3>())
        {
            return 3;
        }
        if (slotValue.is<AZ::Vector2>())
        {
            return 2;
        }
        if (slotValue.is<bool>() || slotValue.is<int>() || slotValue.is<unsigned int>() || slotValue.is<float>())
        {
            return 1;
        }
        return 0;
    }

    AZStd::any MaterialGraphCompiler::ConvertToScalar(const AZStd::any& slotValue) const
    {
        if (auto v = AZStd::any_cast<const AZ::Color>(&slotValue))
        {
            return AZStd::any(v->GetR());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector4>(&slotValue))
        {
            return AZStd::any(v->GetX());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector3>(&slotValue))
        {
            return AZStd::any(v->GetX());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector2>(&slotValue))
        {
            return AZStd::any(v->GetX());
        }
        return slotValue;
    }

    template<typename T>
    AZStd::any MaterialGraphCompiler::ConvertToVector(const AZStd::any& slotValue) const
    {
        if (auto v = AZStd::any_cast<const AZ::Color>(&slotValue))
        {
            return AZStd::any(T(v->GetAsVector4()));
        }
        if (auto v = AZStd::any_cast<const AZ::Vector4>(&slotValue))
        {
            return AZStd::any(T(*v));
        }
        if (auto v = AZStd::any_cast<const AZ::Vector3>(&slotValue))
        {
            return AZStd::any(T(*v));
        }
        if (auto v = AZStd::any_cast<const AZ::Vector2>(&slotValue))
        {
            return AZStd::any(T(*v));
        }
        return slotValue;
    }

    AZStd::any MaterialGraphCompiler::ConvertToVector(const AZStd::any& slotValue, unsigned int score) const
    {
        switch (score)
        {
        case 4:
            // Skipping color to vector conversions so that they export as the correct type with the material type.
            return slotValue.is<AZ::Color>() ? slotValue : ConvertToVector<AZ::Vector4>(slotValue);
        case 3:
            // Skipping color to vector conversions so that they export as the correct type with the material type.
            return slotValue.is<AZ::Color>() ? slotValue : ConvertToVector<AZ::Vector3>(slotValue);
        case 2:
            return ConvertToVector<AZ::Vector2>(slotValue);
        case 1:
            return ConvertToScalar(slotValue);
        default:
            return slotValue;
        }
    }

    AZStd::any MaterialGraphCompiler::GetValueFromSlot(GraphModel::ConstSlotPtr slot) const
    {
        const auto& slotItr = m_slotValueTable.find(slot);
        return slotItr != m_slotValueTable.end() ? slotItr->second : slot->GetValue();
    }

    AZStd::any MaterialGraphCompiler::GetValueFromSlotOrConnection(GraphModel::ConstSlotPtr slot) const
    {
         for (const auto& connection : slot->GetConnections())
        {
             auto sourceSlot = connection->GetSourceSlot();
             auto targetSlot = connection->GetTargetSlot();
             if (targetSlot == slot)
             {
                return GetValueFromSlotOrConnection(sourceSlot);
            }
        }

        return GetValueFromSlot(slot);
    }

    AZStd::string MaterialGraphCompiler::GetAzslTypeFromSlot(GraphModel::ConstSlotPtr slot) const
    {
        const auto& slotValue = GetValueFromSlot(slot);
        const auto& slotDataType = slot->GetGraphContext()->GetDataTypeForValue(slotValue);
        const auto& slotDataTypeName = slotDataType ? slotDataType->GetDisplayName() : AZStd::string{};

        if (AZ::StringFunc::Equal(slotDataTypeName, "color"))
        {
            return "float4";
        }

        return slotDataTypeName;
    }

    AZStd::string MaterialGraphCompiler::GetAzslValueFromSlot(GraphModel::ConstSlotPtr slot) const
    {
        const auto& slotValue = GetValueFromSlot(slot);

        // This code and some of these rules will be refactored and generalized after splitting this class into a document and builder or
        // compiler class. Once that is done, it will be easier to register types, conversions, substitutions with the system.
        for (const auto& connection : slot->GetConnections())
        {
            auto sourceSlot = connection->GetSourceSlot();
            auto targetSlot = connection->GetTargetSlot();
            if (targetSlot == slot)
            {
                // If there is an incoming connection to this slot, the name of the source slot from the incoming connection will be used as
                // part of the value for the slot. It must be cast to the correct vector type for generated code. These conversions will be
                // extended once the code generator is separated from the document class.
                const auto& sourceSlotValue = GetValueFromSlot(sourceSlot);
                const auto& sourceSlotSymbolName = GetSymbolNameFromSlot(sourceSlot);
                if (slotValue.is<AZ::Vector2>())
                {
                    if (sourceSlotValue.is<AZ::Vector3>() ||
                        sourceSlotValue.is<AZ::Vector4>() ||
                        sourceSlotValue.is<AZ::Color>())
                    {
                        return AZStd::string::format("(float2)%s", sourceSlotSymbolName.c_str());
                    }
                }
                if (slotValue.is<AZ::Vector3>())
                {
                    if (sourceSlotValue.is<AZ::Vector2>())
                    {
                        return AZStd::string::format("float3(%s, 0)", sourceSlotSymbolName.c_str());
                    }
                    if (sourceSlotValue.is<AZ::Vector4>() ||
                        sourceSlotValue.is<AZ::Color>())
                    {
                        return AZStd::string::format("(float3)%s", sourceSlotSymbolName.c_str());
                    }
                }
                if (slotValue.is<AZ::Vector4>() ||
                    slotValue.is<AZ::Color>())
                {
                    if (sourceSlotValue.is<AZ::Vector2>())
                    {
                        return AZStd::string::format("float4(%s, 0, 1)", sourceSlotSymbolName.c_str());
                    }
                    if (sourceSlotValue.is<AZ::Vector3>())
                    {
                        return AZStd::string::format("float4(%s, 1)", sourceSlotSymbolName.c_str());
                    }
                }
                return sourceSlotSymbolName;
            }
        }

        // If the slot's embedded value is being used then generate shader code to represent it. More generic options will be explored to
        // clean this code up, possibly storing numeric values in a two-dimensional floating point array with the layout corresponding to
        // most vector and matrix types.
        if (auto v = AZStd::any_cast<const AZ::Color>(&slotValue))
        {
            return AZStd::string::format("{%g, %g, %g, %g}", v->GetR(), v->GetG(), v->GetB(), v->GetA());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector4>(&slotValue))
        {
            return AZStd::string::format("{%g, %g, %g, %g}", v->GetX(), v->GetY(), v->GetZ(), v->GetW());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector3>(&slotValue))
        {
            return AZStd::string::format("{%g, %g, %g}", v->GetX(), v->GetY(), v->GetZ());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector2>(&slotValue))
        {
            return AZStd::string::format("{%g, %g}", v->GetX(), v->GetY());
        }
        if (auto v = AZStd::any_cast<const AZStd::array<AZ::Vector2, 2>>(&slotValue))
        {
            const auto& value = *v;
            return AZStd::string::format(
                "{%g, %g, %g, %g}",
                value[0].GetX(), value[0].GetY(),
                value[1].GetX(), value[1].GetY());
        }
        if (auto v = AZStd::any_cast<const AZStd::array<AZ::Vector3, 3>>(&slotValue))
        {
            const auto& value = *v;
            return AZStd::string::format(
                "{%g, %g, %g, %g, %g, %g, %g, %g, %g}",
                value[0].GetX(), value[0].GetY(), value[0].GetZ(),
                value[1].GetX(), value[1].GetY(), value[1].GetZ(),
                value[2].GetX(), value[2].GetY(), value[2].GetZ());
        }
        if (auto v = AZStd::any_cast<const AZStd::array<AZ::Vector4, 3>>(&slotValue))
        {
            const auto& value = *v;
            return AZStd::string::format(
                "{%g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g}",
                value[0].GetX(), value[0].GetY(), value[0].GetZ(), value[0].GetW(),
                value[1].GetX(), value[1].GetY(), value[1].GetZ(), value[1].GetW(),
                value[2].GetX(), value[2].GetY(), value[2].GetZ(), value[2].GetW());
        }
        if (auto v = AZStd::any_cast<const AZStd::array<AZ::Vector4, 4>>(&slotValue))
        {
            const auto& value = *v;
            return AZStd::string::format(
                "{%g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g}",
                value[0].GetX(), value[0].GetY(), value[0].GetZ(), value[0].GetW(),
                value[1].GetX(), value[1].GetY(), value[1].GetZ(), value[1].GetW(),
                value[2].GetX(), value[2].GetY(), value[2].GetZ(), value[2].GetW(),
                value[3].GetX(), value[3].GetY(), value[3].GetZ(), value[3].GetW());
        }
        if (auto v = AZStd::any_cast<const float>(&slotValue))
        {
            return AZStd::string::format("%g", *v);
        }
        if (auto v = AZStd::any_cast<const int>(&slotValue))
        {
            return AZStd::string::format("%i", *v);
        }
        if (auto v = AZStd::any_cast<const unsigned int>(&slotValue))
        {
            return AZStd::string::format("%u", *v);
        }
        if (auto v = AZStd::any_cast<const bool>(&slotValue))
        {
            return AZStd::string::format("%u", *v ? 1 : 0);
        }
        return AZStd::string();
    }

    AZStd::string MaterialGraphCompiler::GetAzslSrgMemberFromSlot(
        GraphModel::ConstNodePtr node, const AtomToolsFramework::DynamicNodeSlotConfig& slotConfig) const
    {
        if (const auto& slot = node->GetSlot(slotConfig.m_name))
        {
            const auto& slotValue = GetValueFromSlot(slot);
            if (auto v = AZStd::any_cast<const AZ::RHI::SamplerState>(&slotValue))
            {
                // The fields commented out below either cause errors or are not recognized by the shader compiler.
                AZStd::string srgMember;
                srgMember += AZStd::string::format("Sampler SLOTNAME\n");
                srgMember += AZStd::string::format("{\n");
                srgMember += AZStd::string::format("MaxAnisotropy = %u;\n", v->m_anisotropyMax);
                //srgMember += AZStd::string::format("AnisotropyEnable = %u;\n", v->m_anisotropyEnable);
                srgMember += AZStd::string::format("MinFilter = %s;\n", AZ::RHI::FilterModeNamespace::ToString(v->m_filterMin).data());
                srgMember += AZStd::string::format("MagFilter = %s;\n", AZ::RHI::FilterModeNamespace::ToString(v->m_filterMag).data());
                srgMember += AZStd::string::format("MipFilter = %s;\n", AZ::RHI::FilterModeNamespace::ToString(v->m_filterMip).data());
                srgMember += AZStd::string::format("ReductionType = %s;\n", AZ::RHI::ReductionTypeNamespace::ToString(v->m_reductionType).data());
                //srgMember += AZStd::string::format("ComparisonFunc = %s;\n", AZ::RHI::ComparisonFuncNamespace::ToString(v->m_comparisonFunc).data());
                srgMember += AZStd::string::format("AddressU = %s;\n", AZ::RHI::AddressModeNamespace::ToString(v->m_addressU).data());
                srgMember += AZStd::string::format("AddressV = %s;\n", AZ::RHI::AddressModeNamespace::ToString(v->m_addressV).data());
                srgMember += AZStd::string::format("AddressW = %s;\n", AZ::RHI::AddressModeNamespace::ToString(v->m_addressW).data());
                srgMember += AZStd::string::format("MinLOD = %f;\n", v->m_mipLodMin);
                srgMember += AZStd::string::format("MaxLOD = %f;\n", v->m_mipLodMax);
                srgMember += AZStd::string::format("MipLODBias = %f;\n", v->m_mipLodBias);
                srgMember += AZStd::string::format("BorderColor = %s;\n", AZ::RHI::BorderColorNamespace::ToString(v->m_borderColor).data());
                srgMember += "};\n";
                return srgMember;
            }

            if (auto v = AZStd::any_cast<const AZ::Data::Asset<AZ::RPI::StreamingImageAsset>>(&slotValue))
            {
                return AZStd::string::format("Texture2D SLOTNAME;\n");
            }

            return AZStd::string::format("SLOTTYPE SLOTNAME;\n");
        }

        return AZStd::string();
    }

    AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>> MaterialGraphCompiler::GetSubstitutionSymbolsFromNode(
        GraphModel::ConstNodePtr node) const
    {
        AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>> substitutionSymbols;

        // Reserving space for the number of elements added in this function. 
        substitutionSymbols.reserve(node->GetSlots().size() * 4 + 1);
        substitutionSymbols.emplace_back("NODEID", GetSymbolNameFromNode(node));

        for (const auto& slotPair : node->GetSlots())
        {
            const auto& slot = slotPair.second;

            // These substitutions will allow accessing the slot ID, type, value from anywhere in the node's shader code.
            substitutionSymbols.emplace_back(AZStd::string::format("SLOTTYPE\\(%s\\)", slot->GetName().c_str()), GetAzslTypeFromSlot(slot));
            substitutionSymbols.emplace_back(AZStd::string::format("SLOTVALUE\\(%s\\)", slot->GetName().c_str()), GetAzslValueFromSlot(slot));
            substitutionSymbols.emplace_back(AZStd::string::format("SLOTNAME\\(%s\\)", slot->GetName().c_str()), GetSymbolNameFromSlot(slot));

            // This expression will allow direct substitution of node variable names in node configurations with the decorated symbol name.
            // It will match whole words only. No additional decoration should be required on the node configuration side. However, support
            // for the older slot type, name, value substitutions are still supported as a convenience.
            substitutionSymbols.emplace_back(AZStd::string::format("\\b%s\\b", slot->GetName().c_str()), GetSymbolNameFromSlot(slot));
        }

        return substitutionSymbols;
    }

    AZStd::vector<AZStd::string> MaterialGraphCompiler::GetInstructionsFromSlot(
        GraphModel::ConstNodePtr node,
        const AtomToolsFramework::DynamicNodeSlotConfig& slotConfig,
        const AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>>& substitutionSymbols) const
    {
        AZStd::vector<AZStd::string> instructionsForSlot;

        auto slot = node->GetSlot(slotConfig.m_name);
        if (slot && (slot->GetSlotDirection() != GraphModel::SlotDirection::Output || !slot->GetConnections().empty()))
        {
            AtomToolsFramework::CollectDynamicNodeSettings(slotConfig.m_settings, "instructions", instructionsForSlot);

            AtomToolsFramework::ReplaceSymbolsInContainer(substitutionSymbols, instructionsForSlot);
            AtomToolsFramework::ReplaceSymbolsInContainer("SLOTNAME", GetSymbolNameFromSlot(slot), instructionsForSlot);
            AtomToolsFramework::ReplaceSymbolsInContainer("SLOTTYPE", GetAzslTypeFromSlot(slot), instructionsForSlot);
            AtomToolsFramework::ReplaceSymbolsInContainer("SLOTVALUE", GetAzslValueFromSlot(slot), instructionsForSlot);
        }

        return instructionsForSlot;
    }

    bool MaterialGraphCompiler::ShouldUseInstructionsFromInputNode(
        GraphModel::ConstNodePtr outputNode, GraphModel::ConstNodePtr inputNode, const AZStd::vector<AZStd::string>& inputSlotNames) const
    {
        if (inputNode == outputNode)
        {
            return true;
        }

        for (const auto& inputSlotName : inputSlotNames)
        {
            if (const auto slot = outputNode->GetSlot(inputSlotName))
            {
                if (slot->GetSlotDirection() == GraphModel::SlotDirection::Input)
                {
                    for (const auto& connection : slot->GetConnections())
                    {
                        AZ_Assert(connection->GetSourceNode() != outputNode, "This should never be the source node on an input connection.");
                        AZ_Assert(connection->GetTargetNode() == outputNode, "This should always be the target node on an input connection.");
                        if (connection->GetSourceNode() == inputNode || connection->GetSourceNode()->HasInputConnectionFromNode(inputNode))
                        {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    AZStd::vector<GraphModel::ConstNodePtr> MaterialGraphCompiler::GetAllNodesInExecutionOrder() const
    {
        AZStd::vector<GraphModel::ConstNodePtr> nodes;

        if (m_graph)
        {
            nodes.reserve(m_graph->GetNodes().size());
            for (const auto& nodePair : m_graph->GetNodes())
            {
                nodes.push_back(nodePair.second);
            }

            AtomToolsFramework::SortNodesInExecutionOrder(nodes);
        }

        return nodes;
    }

    AZStd::vector<GraphModel::ConstNodePtr> MaterialGraphCompiler::GetInstructionNodesInExecutionOrder(
        GraphModel::ConstNodePtr outputNode, const AZStd::vector<AZStd::string>& inputSlotNames) const
    {
        AZStd::vector<GraphModel::ConstNodePtr> nodes = GetAllNodesInExecutionOrder();
        AZStd::erase_if(nodes, [this, &outputNode, &inputSlotNames](const auto& node) {
            return !ShouldUseInstructionsFromInputNode(outputNode, node, inputSlotNames);
        });
        return nodes;
    }

    AZStd::vector<AZStd::string> MaterialGraphCompiler::GetInstructionsFromConnectedNodes(
        GraphModel::ConstNodePtr outputNode,
        const AZStd::vector<AZStd::string>& inputSlotNames,
        AZStd::vector<GraphModel::ConstNodePtr>& instructionNodes) const
    {
        AZStd::vector<AZStd::string> instructions;

        for (const auto& inputNode : GetInstructionNodesInExecutionOrder(outputNode, inputSlotNames))
        {
            // Build a list of all nodes that will contribute instructions for the output node
            if (AZStd::find(instructionNodes.begin(), instructionNodes.end(), inputNode) == instructionNodes.end())
            {
                instructionNodes.push_back(inputNode);
            }

            auto dynamicNode = azrtti_cast<const AtomToolsFramework::DynamicNode*>(inputNode.get());
            if (dynamicNode)
            {
                const auto& nodeConfig = dynamicNode->GetConfig();
                const auto& substitutionSymbols = GetSubstitutionSymbolsFromNode(inputNode);

                // Instructions are gathered separately for all of the slot categories because they need to be added in a specific order.

                // Gather and perform substitutions on instructions embedded directly in the node.
                AZStd::vector<AZStd::string> instructionsForNode;
                AtomToolsFramework::CollectDynamicNodeSettings(nodeConfig.m_settings, "instructions", instructionsForNode);
                AtomToolsFramework::ReplaceSymbolsInContainer(substitutionSymbols, instructionsForNode);

                // Gather and perform substitutions on instructions contained in property slots.
                AZStd::vector<AZStd::string> instructionsForPropertySlots;
                for (const auto& slotConfig : nodeConfig.m_propertySlots)
                {
                    const auto& instructionsForSlot = GetInstructionsFromSlot(inputNode, slotConfig, substitutionSymbols);
                    instructionsForPropertySlots.insert(instructionsForPropertySlots.end(), instructionsForSlot.begin(), instructionsForSlot.end());
                }

                // Gather and perform substitutions on instructions contained in input slots.
                AZStd::vector<AZStd::string> instructionsForInputSlots;
                for (const auto& slotConfig : nodeConfig.m_inputSlots)
                {
                    // If this is the output node, only gather instructions for requested input slots.
                    if (inputNode == outputNode &&
                        AZStd::find(inputSlotNames.begin(), inputSlotNames.end(), slotConfig.m_name) == inputSlotNames.end())
                    {
                        continue;
                    }

                    const auto& instructionsForSlot = GetInstructionsFromSlot(inputNode, slotConfig, substitutionSymbols);
                    instructionsForInputSlots.insert(instructionsForInputSlots.end(), instructionsForSlot.begin(), instructionsForSlot.end());
                }

                // Gather and perform substitutions on instructions contained in output slots.
                AZStd::vector<AZStd::string> instructionsForOutputSlots;
                for (const auto& slotConfig : nodeConfig.m_outputSlots)
                {
                    const auto& instructionsForSlot = GetInstructionsFromSlot(inputNode, slotConfig, substitutionSymbols);
                    instructionsForOutputSlots.insert(instructionsForOutputSlots.end(), instructionsForSlot.begin(), instructionsForSlot.end());
                }

                instructions.insert(instructions.end(), instructionsForPropertySlots.begin(), instructionsForPropertySlots.end());
                instructions.insert(instructions.end(), instructionsForInputSlots.begin(), instructionsForInputSlots.end());
                instructions.insert(instructions.end(), instructionsForNode.begin(), instructionsForNode.end());
                instructions.insert(instructions.end(), instructionsForOutputSlots.begin(), instructionsForOutputSlots.end());
            }
        }

        return instructions;
    }

    AZStd::string MaterialGraphCompiler::GetSymbolNameFromNode(GraphModel::ConstNodePtr node) const
    {
        return AtomToolsFramework::GetSymbolNameFromText(AZStd::string::format("node%u_%s", node->GetId(), node->GetTitle()));
    }

    AZStd::string MaterialGraphCompiler::GetSymbolNameFromSlot(GraphModel::ConstSlotPtr slot) const
    {
        bool allowNameSubstitution = true;
        if (auto dynamicNode = azrtti_cast<const AtomToolsFramework::DynamicNode*>(slot->GetParentNode().get()))
        {
            const auto& nodeConfig = dynamicNode->GetConfig();
            AtomToolsFramework::VisitDynamicNodeSlotConfigs(
                nodeConfig,
                [&](const AtomToolsFramework::DynamicNodeSlotConfig& slotConfig)
                {
                    if (slot->GetName() == slotConfig.m_name)
                    {
                        allowNameSubstitution = slotConfig.m_allowNameSubstitution;
                    }
                });
        }

        if (!allowNameSubstitution)
        {
            return slot->GetName();
        }

        if (slot->SupportsExtendability())
        {
            return AZStd::string::format(
                "%s_%s_%d", GetSymbolNameFromNode(slot->GetParentNode()).c_str(), slot->GetName().c_str(), slot->GetSlotSubId());
        }

        return AZStd::string::format("%s_%s", GetSymbolNameFromNode(slot->GetParentNode()).c_str(), slot->GetName().c_str());
    }

    AZStd::vector<AZStd::string> MaterialGraphCompiler::GetMaterialPropertySrgMemberFromSlot(
        GraphModel::ConstNodePtr node,
        const AtomToolsFramework::DynamicNodeSlotConfig& slotConfig,
        const AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>>& substitutionSymbols) const
    {
        AZStd::vector<AZStd::string> materialPropertySrgMemberForSlot;

        if (auto slot = node->GetSlot(slotConfig.m_name))
        {
            AtomToolsFramework::CollectDynamicNodeSettings(slotConfig.m_settings, "materialPropertySrgMember", materialPropertySrgMemberForSlot);

            AtomToolsFramework::ReplaceSymbolsInContainer(substitutionSymbols, materialPropertySrgMemberForSlot);
            AtomToolsFramework::ReplaceSymbolsInContainer("STANDARD_SRG_MEMBER", GetAzslSrgMemberFromSlot(node, slotConfig), materialPropertySrgMemberForSlot);
            AtomToolsFramework::ReplaceSymbolsInContainer("SLOTNAME", GetSymbolNameFromSlot(slot), materialPropertySrgMemberForSlot);
            AtomToolsFramework::ReplaceSymbolsInContainer("SLOTTYPE", GetAzslTypeFromSlot(slot), materialPropertySrgMemberForSlot);
            AtomToolsFramework::ReplaceSymbolsInContainer("SLOTVALUE", GetAzslValueFromSlot(slot), materialPropertySrgMemberForSlot);
        }

        return materialPropertySrgMemberForSlot;
    }

    AZStd::vector<AZStd::string> MaterialGraphCompiler::GetMaterialPropertySrgMemberFromNodes(
        const AZStd::vector<GraphModel::ConstNodePtr>& instructionNodes) const
    {
        if (!m_graph)
        {
            AZ_Assert(false, "Attempting to generate data from invalid graph object.");
            return {};
        }

        AZStd::vector<AZStd::string> materialPropertySrgMember;

        for (const auto& inputNode : instructionNodes)
        {
            auto dynamicNode = azrtti_cast<const AtomToolsFramework::DynamicNode*>(inputNode.get());
            if (dynamicNode)
            {
                const auto& nodeConfig = dynamicNode->GetConfig();
                const auto& substitutionSymbols = GetSubstitutionSymbolsFromNode(inputNode);

                AZStd::vector<AZStd::string> materialPropertySrgMembersForNode;
                AtomToolsFramework::CollectDynamicNodeSettings(
                    nodeConfig.m_settings, "materialPropertySrgMember", materialPropertySrgMembersForNode);
                AtomToolsFramework::ReplaceSymbolsInContainer(substitutionSymbols, materialPropertySrgMembersForNode);

                AtomToolsFramework::VisitDynamicNodeSlotConfigs(
                    nodeConfig,
                    [&](const AtomToolsFramework::DynamicNodeSlotConfig& slotConfig)
                    {
                        const auto& materialPropertySrgMemberForSlot =
                            GetMaterialPropertySrgMemberFromSlot(inputNode, slotConfig, substitutionSymbols);
                        materialPropertySrgMembersForNode.insert(
                            materialPropertySrgMembersForNode.end(),
                            materialPropertySrgMemberForSlot.begin(),
                            materialPropertySrgMemberForSlot.end());
                    });

                materialPropertySrgMember.insert(
                    materialPropertySrgMember.end(), materialPropertySrgMembersForNode.begin(), materialPropertySrgMembersForNode.end());
            }
        }

        return materialPropertySrgMember;
    }

    bool MaterialGraphCompiler::BuildMaterialTypeFromTemplate(
        GraphModel::ConstNodePtr templateNode,
        const AZStd::vector<GraphModel::ConstNodePtr>& instructionNodes,
        const AZStd::string& templateInputPath,
        const AZStd::string& templateOutputPath) const
    {
        using namespace AtomToolsFramework;

        if (!m_graph)
        {
            AZ_Assert(false, "Attempting to generate data from invalid graph object.");
            return false;
        }

        if (!templateNode)
        {
            AZ_Assert(false, "Attempting to generate data from invalid template node.");
            return false;
        }

        // Load the material type template file, which is the same format as MaterialTypeSourceData with a different extension
        auto materialTypeOutcome = AZ::RPI::MaterialUtils::LoadMaterialTypeSourceData(templateInputPath);
        if (!materialTypeOutcome.IsSuccess())
        {
            AZ_Error("MaterialGraphCompiler", false, "Material type template could not be loaded: '%s'.", templateInputPath.c_str());
            return false;
        }

        // Copy the material type source data from the template and begin populating it.
        AZ::RPI::MaterialTypeSourceData materialTypeSourceData = materialTypeOutcome.TakeValue();

        // If the node providing all the template information has a description then assign it to the material type source data.
        materialTypeSourceData.m_description = GetStringValueFromSlot(templateNode->GetSlot("inDescription"));

        // Search the graph for nodes defining material input properties that should be added to the material type and material SRG
        for (const auto& inputNode : instructionNodes)
        {
            // Search for all slots with settings indicating that material type properties should be generated. The settings can correspond
            // to shader inputs, shader options, and other material property values that may or may not have matching entries in the
            // material SRG.
            AZStd::vector<AZStd::pair<GraphModel::ConstSlotPtr, DynamicNodeSlotConfig>> materialPropertyValueSlots;
            if (auto dynamicNode = azrtti_cast<const DynamicNode*>(inputNode.get()))
            {
                VisitDynamicNodeSlotConfigs(
                    dynamicNode->GetConfig(),
                    [&](const DynamicNodeSlotConfig& slotConfig)
                    {
                        if (slotConfig.m_settings.contains("materialPropertyName") ||
                            slotConfig.m_settings.contains("materialPropertyDisplayName") ||
                            slotConfig.m_settings.contains("materialPropertyConnectionType") ||
                            slotConfig.m_settings.contains("materialPropertyConnectionName") ||
                            slotConfig.m_settings.contains("materialPropertyGroupName") ||
                            slotConfig.m_settings.contains("materialPropertyGroup"))
                        {
                            const auto materialPropertyValueSlot = inputNode->GetSlot(slotConfig.m_name);
                            materialPropertyValueSlots.emplace_back(materialPropertyValueSlot, slotConfig);
                        }
                    });
            }

            // Register all the properties that were parsed out of the slots with the material type.
            for (const auto& [materialPropertyValueSlot, materialPropertyValueSlotConfig] : materialPropertyValueSlots)
            {
                // Sampler states are currently not configurable and will not be added added to the material type, just the material SRG.
                if (!materialPropertyValueSlot || materialPropertyValueSlot->GetValue().empty() ||
                    materialPropertyValueSlot->GetValue().is<AZ::RHI::SamplerState>())
                {
                    continue;
                }

                // If the property represents a shader option, the connection name will be defined in a static setting. Otherwise, it will
                // be the slot symbol name which is the same as the variable name added to the SRG and referenced in code.
                const auto& materialPropertyConnectionName = GetFirstNonEmptyString({
                    GetSettingValueByName(materialPropertyValueSlotConfig.m_settings, "materialPropertyConnectionName"),
                    GetSymbolNameFromSlot(materialPropertyValueSlot)
                    });

                // The material property connection type determines if the connection represents a shader option, shader input, internal
                // value, or just a placeholder property.
                const auto& materialPropertyConnectionType = GetFirstNonEmptyString({
                    GetSettingValueByName(materialPropertyValueSlotConfig.m_settings, "materialPropertyConnectionType")
                    });

                // While this might change, material properties representing shader inputs generally have their name, display name,
                // description, and other details spread across multiple, user configurable slots on the same node. Shader options don't
                // need a user configurable name or description because they refer to a predefined option name that will always be used the
                // same way. Several shader options can be exposed on the same node. Because of that, shader options must specify their
                // connection name and copy the name and description directly from the slot instead of having the users enter one.
                const auto& materialPropertyUseSlotConfig = !AZ::StringFunc::Equal(materialPropertyConnectionType, "ShaderInput");

                // The material property name must be unique relative to its group. Material property names are used to read and write
                // property values through the material system API. These will be stored with default values in the material type and
                // overridden values per material. In material canvas, rather than overwhelming the user with Learning and managing the
                // differences between IDs, names, and display names, we will generate the values for symbol and display names Based on a
                // single user specified material input name, slot settings, or the symbol name generated from the node and slot IDs.

                // Find the most appropriate name to use for this property, prioritizing static settings for shader options first.
                const auto& materialPropertyName = GetFirstNonEmptyString({
                    GetSettingValueByName(materialPropertyValueSlotConfig.m_settings, "materialPropertyName"),
                    GetSettingValueByName(materialPropertyValueSlotConfig.m_settings, "materialPropertyDisplayName"),
                    materialPropertyUseSlotConfig ? materialPropertyValueSlotConfig.m_displayName : GetStringValueFromSlot(inputNode->GetSlot("inDisplayName")),
                    materialPropertyUseSlotConfig ? materialPropertyValueSlotConfig.m_name : GetStringValueFromSlot(inputNode->GetSlot("inName")),
                    GetSymbolNameFromSlot(materialPropertyValueSlot)
                    });

                // The symbol name used to uniquely identify the property in its group will be generated by transforming the above name to
                // lowercase and replacing all non word characters with underscores.
                const auto& materialPropertySymbolName = GetSymbolNameFromText(materialPropertyName);

                // The display name slot was removed from the original, experimental material output nodes but we are handling it for
                // backwards compatibility. The display name will otherwise be generated by sanitizing and camel casing the property name.
                const auto& materialPropertyDisplayName = GetDisplayNameFromText(GetFirstNonEmptyString({
                    GetSettingValueByName(materialPropertyValueSlotConfig.m_settings, "materialPropertyDisplayName"),
                    materialPropertyUseSlotConfig ? materialPropertyValueSlotConfig.m_displayName : GetStringValueFromSlot(inputNode->GetSlot("inDisplayName")),
                    materialPropertyName
                    }));

                if (materialPropertyName.empty() || materialPropertySymbolName.empty() || materialPropertyDisplayName.empty())
                {
                    AZ_Error(
                        "MaterialGraphCompiler",
                        false,
                        "Material property name could not be resolved for slot '%s' and template '%s'.",
                        GetSymbolNameFromSlot(materialPropertyValueSlot).c_str(),
                        templateOutputPath.c_str());
                    return false;
                }

                // The group name can be specified in a static setting for shader options or configured for material inputs. Properties that
                // do not explicitly define a group will fall back to the general group.
                const auto& materialPropertyGroupName = GetFirstNonEmptyString({
                    GetSettingValueByName(materialPropertyValueSlotConfig.m_settings, "materialPropertyGroup"),
                    GetSettingValueByName(materialPropertyValueSlotConfig.m_settings, "materialPropertyGroupName"),
                    GetStringValueFromSlot(inputNode->GetSlot("inGroup")),
                    "general"
                    });

                // Sanitize the symbol and display names for the group to force casing, spacing, and eliminate any potential erroneous input.
                const auto& materialPropertyGroupSymbolName = GetSymbolNameFromText(materialPropertyGroupName);
                const auto& materialPropertyGroupDisplayName = GetDisplayNameFromText(materialPropertyGroupName);
                if (materialPropertyGroupName.empty() || materialPropertyGroupDisplayName.empty())
                {
                    AZ_Error(
                        "MaterialGraphCompiler",
                        false,
                        "Material property group could not be resolved for slot '%s' and template '%s'.",
                        GetSymbolNameFromSlot(materialPropertyValueSlot).c_str(),
                        templateOutputPath.c_str());
                    return false;
                }


                // The property description can also be read from static settings for shader options or a user configurable slot
                // for material inputs. If no description is specified, it will fall back to using the material property display name.
                const auto& materialPropertyDescription = GetFirstNonEmptyString({
                    GetSettingValueByName(materialPropertyValueSlotConfig.m_settings, "materialPropertyDescription"),
                    materialPropertyUseSlotConfig ? materialPropertyValueSlotConfig.m_description : GetStringValueFromSlot(inputNode->GetSlot("inDescription")),
                    materialPropertyDisplayName
                    });

                // Lastly, the property value is read from the slot.
                const auto& materialPropertyValue = GetValueFromSlot(materialPropertyValueSlot);

                // The complete property ID is a combination of the group name and the property name.
                const AZ::Name materialPropertyId(materialPropertyGroupSymbolName + "." + materialPropertySymbolName);

                // Find or create a property group with the specified name
                auto propertyGroup = materialTypeSourceData.FindPropertyGroup(materialPropertyGroupSymbolName);
                if (!propertyGroup)
                {
                    // Add the property group to the material type if it was not already registered
                    propertyGroup = materialTypeSourceData.AddPropertyGroup(materialPropertyGroupSymbolName);

                    // The unmodified text value will be used as the display name and description for now
                    propertyGroup->SetDisplayName(materialPropertyGroupDisplayName);
                    propertyGroup->SetDescription(materialPropertyGroupDisplayName);
                }

                auto property = propertyGroup->AddProperty(materialPropertySymbolName);
                property->m_displayName = materialPropertyDisplayName;
                property->m_description = materialPropertyDescription;
                property->m_enumValues = materialPropertyValueSlotConfig.m_enumValues;
                property->m_value = AZ::RPI::MaterialPropertyValue::FromAny(materialPropertyValue);

                // The property definition requires an explicit type enum that's converted from the actual data type.
                property->m_dataType = GetMaterialPropertyDataTypeFromValue(property->m_value, !property->m_enumValues.empty());

                // Images and enums need additional conversion prior to being saved.
                ConvertToExportFormat(templateOutputPath, materialPropertyId, *property, property->m_value);

                // This property connects to the material SRG member with the same name. Shader options are not yet supported.
                if (!materialPropertyConnectionName.empty())
                {
                    if (AZ::StringFunc::Equal(materialPropertyConnectionType, "ShaderInput"))
                    {
                        property->m_outputConnections.emplace_back(
                            AZ::RPI::MaterialPropertyOutputType::ShaderInput, materialPropertyConnectionName);
                    }
                    if (AZ::StringFunc::Equal(materialPropertyConnectionType, "ShaderOption"))
                    {
                        property->m_outputConnections.emplace_back(
                            AZ::RPI::MaterialPropertyOutputType::ShaderOption, materialPropertyConnectionName);
                    }
                }
            }
        }

        // The file is written to an in memory buffer before saving to facilitate string substitutions.
        AZStd::string templateOutputText;
        if (!AZ::RPI::JsonUtils::SaveObjectToString(templateOutputText, materialTypeSourceData))
        {
            AZ_Error("MaterialGraphCompiler", false, "Material type template could not be saved: '%s'.", templateOutputPath.c_str());
            return false;
        }

        // Substitute the material graph name and any other Material Canvas specific tokens
        AZ::StringFunc::Replace(templateOutputText, "MaterialGraphName", m_graphName.c_str());

        AZ_TracePrintf_IfTrue(
            "MaterialGraphCompiler", IsCompileLoggingEnabled(), "Saving generated file: %s\n", templateOutputPath.c_str());

        // The material type is complete and can be saved to disk.
        const auto writeOutcome = AZ::Utils::WriteFile(templateOutputText, templateOutputPath);
        if (!writeOutcome)
        {
            AZ_Error("MaterialGraphCompiler", false, "Material type template could not be saved: '%s'.", templateOutputPath.c_str());
            return false;
        }

        return true;
    }
} // namespace MaterialCanvas
