/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RHI/RHISystemInterface.h>
#include <Atom/RHI/RHIUtils.h>
#include <Atom/RHI.Reflect/InputStreamLayoutBuilder.h>
#include <Atom/Feature/RenderCommon.h>
#include <Atom/Feature/Mesh/MeshCommon.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessor.h>
#include <Atom/Feature/Mesh/ModelReloaderSystemInterface.h>
#include <Atom/RPI.Public/Model/ModelLodUtils.h>
#include <Atom/RPI.Public/Model/ModelTagSystemComponent.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Culling.h>
#include <Atom/RPI.Public/RPIUtils.h>
#include <Atom/RPI.Public/AssetQuality.h>

#include <Atom/Utils/StableDynamicArray.h>
#include <ReflectionProbe/ReflectionProbeFeatureProcessor.h>

#include <Atom/RPI.Reflect/Model/ModelAssetCreator.h>

#include <AzFramework/Asset/AssetSystemBus.h>

#include <AtomCore/Instance/InstanceDatabase.h>

#include <AzCore/Console/IConsole.h>
#include <AzCore/Jobs/Algorithms.h>
#include <AzCore/Jobs/JobCompletion.h>
#include <AzCore/Jobs/JobFunction.h>
#include <AzCore/Math/ShapeIntersection.h>
#include <AzCore/Name/NameDictionary.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/RTTI/TypeInfo.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Name/NameDictionary.h>

namespace AZ
{
    namespace Render
    {
        static AZ::Name s_o_meshUseForwardPassIBLSpecular_Name =
            AZ::Name::FromStringLiteral("o_meshUseForwardPassIBLSpecular", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_Manual_Name = AZ::Name::FromStringLiteral("Manual", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_Multiply_Name = AZ::Name::FromStringLiteral("Multiply", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_BaseColorTint_Name = AZ::Name::FromStringLiteral("BaseColorTint", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_BaseColor_Name = AZ::Name::FromStringLiteral("BaseColor", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_baseColor_color_Name = AZ::Name::FromStringLiteral("baseColor.color", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_baseColor_factor_Name = AZ::Name::FromStringLiteral("baseColor.factor", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_baseColor_useTexture_Name =
            AZ::Name::FromStringLiteral("baseColor.useTexture", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_metallic_factor_Name = AZ::Name::FromStringLiteral("metallic.factor", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_roughness_factor_Name = AZ::Name::FromStringLiteral("roughness.factor", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_emissive_enable_Name = AZ::Name::FromStringLiteral("emissive.enable", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_emissive_color_Name = AZ::Name::FromStringLiteral("emissive.color", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_emissive_intensity_Name =
            AZ::Name::FromStringLiteral("emissive.intensity", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_baseColor_textureMap_Name =
            AZ::Name::FromStringLiteral("baseColor.textureMap", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_normal_textureMap_Name =
            AZ::Name::FromStringLiteral("normal.textureMap", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_metallic_textureMap_Name =
            AZ::Name::FromStringLiteral("metallic.textureMap", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_roughness_textureMap_Name =
            AZ::Name::FromStringLiteral("roughness.textureMap", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_irradiance_irradianceColorSource_Name =
            AZ::Name::FromStringLiteral("irradiance.irradianceColorSource", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_emissive_textureMap_Name =
            AZ::Name::FromStringLiteral("emissive.textureMap", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_irradiance_manualColor_Name =
            AZ::Name::FromStringLiteral("irradiance.manualColor", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_irradiance_color_Name = AZ::Name::FromStringLiteral("irradiance.color", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_baseColor_textureBlendMode_Name =
            AZ::Name::FromStringLiteral("baseColor.textureBlendMode", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_irradiance_factor_Name =
            AZ::Name::FromStringLiteral("irradiance.factor", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_opacity_mode_Name = AZ::Name::FromStringLiteral("opacity.mode", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_opacity_factor_Name = AZ::Name::FromStringLiteral("opacity.factor", AZ::Interface<AZ::NameDictionary>::Get());
        static AZ::Name s_m_rootConstantInstanceDataOffset_Name =
            AZ::Name::FromStringLiteral("m_rootConstantInstanceDataOffset", AZ::Interface<AZ::NameDictionary>::Get());

        static void CacheRootConstantInterval(MeshInstanceGroupData& meshInstanceGroupData)
        {
            meshInstanceGroupData.m_drawRootConstantInterval = RHI::Interval{};

            RHI::ConstPtr<RHI::ConstantsLayout> rootConstantsLayout = meshInstanceGroupData.m_drawPacket.GetRootConstantsLayout();
            if (rootConstantsLayout)
            {
                // Get the root constant layout
                RHI::ShaderInputConstantIndex shaderInputIndex =
                    rootConstantsLayout->FindShaderInputIndex(s_m_rootConstantInstanceDataOffset_Name);

                if (shaderInputIndex.IsValid())
                {
                    RHI::Interval interval = rootConstantsLayout->GetInterval(shaderInputIndex);
                    meshInstanceGroupData.m_drawRootConstantInterval = interval;
                }
            }
        }

        void MeshFeatureProcessor::Reflect(ReflectContext* context)
        {
            if (auto* serializeContext = azrtti_cast<SerializeContext*>(context))
            {
                serializeContext
                    ->Class<MeshFeatureProcessor, FeatureProcessor>()
                    ->Version(1);
            }
        }

        void MeshFeatureProcessor::Activate()
        {
            m_transformService = GetParentScene()->GetFeatureProcessor<TransformServiceFeatureProcessor>();
            AZ_Assert(m_transformService, "MeshFeatureProcessor requires a TransformServiceFeatureProcessor on its parent scene.");

            m_rayTracingFeatureProcessor = GetParentScene()->GetFeatureProcessor<RayTracingFeatureProcessor>();
            m_reflectionProbeFeatureProcessor = GetParentScene()->GetFeatureProcessor<ReflectionProbeFeatureProcessor>();
            m_handleGlobalShaderOptionUpdate = RPI::ShaderSystemInterface::GlobalShaderOptionUpdatedEvent::Handler
            {
                [this](const AZ::Name&, RPI::ShaderOptionValue) { m_forceRebuildDrawPackets = true; }
            };
            RPI::ShaderSystemInterface::Get()->Connect(m_handleGlobalShaderOptionUpdate);
            EnableSceneNotification();

            // Must read cvar from AZ::Console due to static variable in multiple libraries, see ghi-5537
            bool enablePerMeshShaderOptionFlagsCvar = false;
            if (auto* console = AZ::Interface<AZ::IConsole>::Get(); console != nullptr)
            {
                console->GetCvarValue("r_enablePerMeshShaderOptionFlags", enablePerMeshShaderOptionFlagsCvar);

                // push the cvars value so anything in this dll can access it directly.
                console->PerformCommand(AZStd::string::format("r_enablePerMeshShaderOptionFlags %s", enablePerMeshShaderOptionFlagsCvar ? "true" : "false").c_str());
            }

            m_meshMovedFlag = GetParentScene()->GetViewTagBitRegistry().AcquireTag(MeshCommon::MeshMovedName);
            m_meshMotionDrawListTag = AZ::RHI::RHISystemInterface::Get()->GetDrawListTagRegistry()->AcquireTag(MeshCommon::MotionDrawListTagName);
            
            if (auto* console = AZ::Interface<AZ::IConsole>::Get(); console != nullptr)
            {
                console->GetCvarValue("r_meshInstancingEnabled", m_enableMeshInstancing);

                // push the cvars value so anything in this dll can access it directly.
                console->PerformCommand(
                    AZStd::string::format("r_meshInstancingEnabled %s", m_enableMeshInstancing ? "true" : "false")
                        .c_str());
            }
        }

        void MeshFeatureProcessor::Deactivate()
        {
            m_flagRegistry.reset();

            m_handleGlobalShaderOptionUpdate.Disconnect();

            DisableSceneNotification();
            AZ_Warning("MeshFeatureProcessor", m_modelData.size() == 0,
                "Deactivating the MeshFeatureProcessor, but there are still outstanding mesh handles.\n"
            );
            m_transformService = nullptr;
            m_rayTracingFeatureProcessor = nullptr;
            m_reflectionProbeFeatureProcessor = nullptr;
            m_forceRebuildDrawPackets = false;

            GetParentScene()->GetViewTagBitRegistry().ReleaseTag(m_meshMovedFlag);
            RHI::RHISystemInterface::Get()->GetDrawListTagRegistry()->ReleaseTag(m_meshMotionDrawListTag);
        }

        TransformServiceFeatureProcessorInterface::ObjectId MeshFeatureProcessor::GetObjectId(const MeshHandle& meshHandle) const
        {
            if (meshHandle.IsValid())
            {
                return meshHandle->m_objectId;
            }

            return TransformServiceFeatureProcessorInterface::ObjectId::Null;
        }

        void MeshFeatureProcessor::Simulate(const FeatureProcessor::SimulatePacket& packet)
        {
            AZ_PROFILE_SCOPE(RPI, "MeshFeatureProcessor: Simulate");

            AZ::Job* parentJob = packet.m_parentJob;
            AZStd::concurrency_check_scope scopeCheck(m_meshDataChecker);

            // If the instancing cvar has changed, we need to re-initalize the ModelDataInstances
            CheckForInstancingCVarChange();

            AZStd::vector<Job*> initJobQueue = CreateInitJobQueue();
            AZStd::vector<Job*> updateCullingJobQueue = CreateUpdateCullingJobQueue();

            if (!r_meshInstancingEnabled)
            {
                // There's no need for all the init jobs to finish before any of the update culling jobs are run.
                // Any update culling job can run once it's corresponding init job is done. So instead of separating the jobs
                // entirely, use individual job dependencies to synchronize them. This performs better than having a big sync between them
                ExecuteCombinedJobQueue(initJobQueue, updateCullingJobQueue, parentJob);
            }
            else
            {
                AZStd::vector<Job*> perInstanceGroupJobQueue = CreatePerInstanceGroupJobQueue();

                ExecuteSimulateJobQueue(initJobQueue, parentJob);
                // Per-InstanceGroup work must be done after the Init jobs are complete, because the init jobs will determine which instance
                // group each mesh belongs to and populate those instance groups
                ExecuteSimulateJobQueue(perInstanceGroupJobQueue, parentJob);
                // Updating the culling scene must happen after the per-instance group work is done
                // because the per-instance group work will update the draw packets.
                ExecuteSimulateJobQueue(updateCullingJobQueue, parentJob);
            }

            m_forceRebuildDrawPackets = false;
        }

        void MeshFeatureProcessor::CheckForInstancingCVarChange()
        {
            if (m_enableMeshInstancing != r_meshInstancingEnabled)
            {
                // DeInit and re-init every object
                for (auto& modelDataInstance : m_modelData)
                {
                    modelDataInstance.ReInit(this);
                }
                m_enableMeshInstancing = r_meshInstancingEnabled;
            }
        }

        AZStd::vector<Job*> MeshFeatureProcessor::CreatePerInstanceGroupJobQueue()
        {
            const auto instanceManagerRanges = m_meshInstanceManager.GetParallelRanges();
            AZStd::vector<Job*> perInstanceGroupJobQueue;
            perInstanceGroupJobQueue.reserve(instanceManagerRanges.size());
            RPI::Scene* scene = GetParentScene();
            for (const auto& iteratorRange : instanceManagerRanges)
            {
                const auto perInstanceGroupJobLambda = [this, scene, iteratorRange]() -> void
                {
                    AZ_PROFILE_SCOPE(AzRender, "MeshFeatureProcessor: Simulate: PerInstanceGroupUpdate");
                    for (auto instanceGroupDataIter = iteratorRange.m_begin; instanceGroupDataIter != iteratorRange.m_end;
                         ++instanceGroupDataIter)
                    {
                        RPI::MeshDrawPacket& drawPacket = instanceGroupDataIter->m_drawPacket;
                        if (drawPacket.Update(*scene, m_forceRebuildDrawPackets))
                        {
                            // Clear any cached draw packets, since they need to be re-created
                            instanceGroupDataIter->m_perViewDrawPackets.clear();

                            // We're going to need an interval for the root constant data that we update every frame for each draw item, so
                            // cache that here
                            CacheRootConstantInterval(*instanceGroupDataIter);

                            instanceGroupDataIter->m_updateDrawPacketEvent.Signal();
                        }
                    }
                };
                Job* executePerInstanceGroupJob =
                    aznew JobFunction<decltype(perInstanceGroupJobLambda)>(perInstanceGroupJobLambda, true, nullptr); // Auto-deletes
                perInstanceGroupJobQueue.push_back(executePerInstanceGroupJob);
            }
            return perInstanceGroupJobQueue;
        }

        AZStd::vector<Job*> MeshFeatureProcessor::CreateInitJobQueue()
        {
            const auto iteratorRanges = m_modelData.GetParallelRanges();
            AZStd::vector<Job*> initJobQueue;
            initJobQueue.reserve(iteratorRanges.size());
            for (const auto& iteratorRange : iteratorRanges)
            {
                const auto initJobLambda = [this, iteratorRange]() -> void
                {
                    AZ_PROFILE_SCOPE(AzRender, "MeshFeatureProcessor: Simulate: Init");

                    for (auto meshDataIter = iteratorRange.m_begin; meshDataIter != iteratorRange.m_end; ++meshDataIter)
                    {
                        if (!meshDataIter->m_model)
                        {
                            continue; // model not loaded yet
                        }

                        if (!meshDataIter->m_flags.m_visible)
                        {
                            continue;
                        }

                        if (meshDataIter->m_flags.m_needsInit)
                        {
                            meshDataIter->Init(this);
                        }

                        if (meshDataIter->m_flags.m_objectSrgNeedsUpdate)
                        {
                            meshDataIter->UpdateObjectSrg(this);
                        }

                        if (meshDataIter->m_flags.m_needsSetRayTracingData)
                        {
                            meshDataIter->SetRayTracingData(this);
                        }

                        // If instancing is enabled, the draw packets will be updated by the per-instance group jobs,
                        // so they don't need to be updated here
                        if (!r_meshInstancingEnabled)
                        {
                            // [GFX TODO] [ATOM-1357] Currently all of the draw packets have to be checked for material ID changes because
                            // material properties can impact which actual shader is used, which impacts the SRG in the draw packet.
                            // This is scheduled to be optimized so the work is only done on draw packets that need it instead of having
                            // to check every one.
                            meshDataIter->UpdateDrawPackets(m_forceRebuildDrawPackets);
                        }
                    }
                };
                Job* executeInitJob = aznew JobFunction<decltype(initJobLambda)>(initJobLambda, true, nullptr); // Auto-deletes
                initJobQueue.push_back(executeInitJob);
            }
            return initJobQueue;
        }

        AZStd::vector<Job*> MeshFeatureProcessor::CreateUpdateCullingJobQueue()
        {
            const auto iteratorRanges = m_modelData.GetParallelRanges();
            AZStd::vector<Job*> updateCullingJobQueue;
            updateCullingJobQueue.reserve(iteratorRanges.size());
            for (const auto& iteratorRange : iteratorRanges)
            {
                const auto updateCullingJobLambda = [this, iteratorRange]() -> void
                {
                    AZ_PROFILE_SCOPE(AzRender, "MeshFeatureProcessor: Simulate: UpdateCulling");

                    for (auto meshDataIter = iteratorRange.m_begin; meshDataIter != iteratorRange.m_end; ++meshDataIter)
                    {
                        if (!meshDataIter->m_model)
                        {
                            continue; // model not loaded yet
                        }

                        if (meshDataIter->m_flags.m_cullableNeedsRebuild)
                        {
                            meshDataIter->BuildCullable(this);
                        }

                        if (meshDataIter->m_flags.m_cullBoundsNeedsUpdate)
                        {
                            meshDataIter->UpdateCullBounds(this);
                        }
                    }
                };
                Job* executeUpdateGroupJob =
                    aznew JobFunction<decltype(updateCullingJobLambda)>(updateCullingJobLambda, true, nullptr); // Auto-deletes
                updateCullingJobQueue.push_back(executeUpdateGroupJob);
            }
            return updateCullingJobQueue;
        }

        void MeshFeatureProcessor::ExecuteCombinedJobQueue(AZStd::span<Job*> initQueue, AZStd::span<Job*> updateCullingQueue, Job* parentJob)
        {
            AZ::JobCompletion jobCompletion;
            for (size_t i = 0; i < initQueue.size(); ++i)
            {
                // Update Culling work should happen after Init is done
                initQueue[i]->SetDependent(updateCullingQueue[i]);

                // FeatureProcessor::Simulate is optionally run with a parent job.
                if (parentJob)
                {
                    // When a parent job is used, we set dependencies on it and use WaitForChildren to wait for them to finish executing
                    parentJob->StartAsChild(updateCullingQueue[i]);
                    initQueue[i]->Start();
                }
                else
                {
                    // When a parent job is not used, we use a job completion to synchronize
                    updateCullingQueue[i]->SetDependent(&jobCompletion);
                    initQueue[i]->Start();
                    updateCullingQueue[i]->Start();
                }
            }

            if (parentJob)
            {
                parentJob->WaitForChildren();
            }
            else
            {
                jobCompletion.StartAndWaitForCompletion();
            }
        }

        void MeshFeatureProcessor::ExecuteSimulateJobQueue(AZStd::span<Job*> jobQueue, Job* parentJob)
        {
            AZ::JobCompletion jobCompletion;
            for (Job* childJob : jobQueue)
            {
                // FeatureProcessor::Simulate is optionally run with a parent job.
                if (parentJob)
                {
                    // When a parent job is used, we set dependencies on it and use WaitForChildren to wait for them to finish executing
                    parentJob->StartAsChild(childJob);
                }
                else
                {
                    // When a parent job is not used, we use a job completion to synchronize
                    childJob->SetDependent(&jobCompletion);
                    childJob->Start();
                }
            }

            if (parentJob)
            {
                parentJob->WaitForChildren();
            }
            else
            {
                jobCompletion.StartAndWaitForCompletion();
            }
        }

        void MeshFeatureProcessor::OnBeginPrepareRender()
        {
            m_meshDataChecker.soft_lock();
            AZ_Error("MeshFeatureProcessor::OnBeginPrepareRender", !(r_enablePerMeshShaderOptionFlags && r_meshInstancingEnabled),
                "r_enablePerMeshShaderOptionFlags and r_meshInstancingEnabled are incompatible at this time. r_enablePerMeshShaderOptionFlags results "
                "in a unique shader permutation for a given object depending on which light types are in range of the object. This isn't known until "
                "immediately before rendering. Determining whether or not two meshes can be instanced happens when the object is first set up, and we don't "
                "want to update that instance map every frame, so if instancing is enabled we treat r_enablePerMeshShaderOptionFlags as disabled. "
                "This can be relaxed for static meshes in the future when we know they won't be moving. ");
            if (!r_enablePerMeshShaderOptionFlags && m_enablePerMeshShaderOptionFlags && !r_meshInstancingEnabled)
            {
                // Per mesh shader option flags was on, but now turned off, so reset all the shader options.
                for (auto& modelHandle : m_modelData)
                {
                    for (RPI::MeshDrawPacketList& drawPacketList : modelHandle.m_drawPacketListsByLod)
                    {
                        for (RPI::MeshDrawPacket& drawPacket : drawPacketList)
                        {
                            m_flagRegistry->VisitTags(
                                [&](AZ::Name shaderOption, [[maybe_unused]] FlagRegistry::TagType tag)
                                {
                                    drawPacket.UnsetShaderOption(shaderOption);
                                }
                            );
                            drawPacket.Update(*GetParentScene(), true);
                        }
                    }
                    modelHandle.m_cullable.m_shaderOptionFlags = 0;
                    modelHandle.m_cullable.m_prevShaderOptionFlags = 0;
                    modelHandle.m_flags.m_cullableNeedsRebuild = true;

                    // [GHI-13619]
                    // Update the draw packets on the cullable, since we just set a shader item.
                    // BuildCullable is a bit overkill here, this could be reduced to just updating the drawPacket specific info
                    // It's also going to cause m_cullableNeedsUpdate to be set, which will execute next frame, which we don't need
                    modelHandle.BuildCullable(this);
                }
            }

            m_enablePerMeshShaderOptionFlags = r_enablePerMeshShaderOptionFlags && !r_meshInstancingEnabled;

            if (m_enablePerMeshShaderOptionFlags)
            {
                for (auto& modelHandle : m_modelData)
                {
                    if (modelHandle.m_cullable.m_prevShaderOptionFlags != modelHandle.m_cullable.m_shaderOptionFlags)
                    {
                        // Per mesh shader option flags have changed, so rebuild the draw packet with the new shader options.
                        for (RPI::MeshDrawPacketList& drawPacketList : modelHandle.m_drawPacketListsByLod)
                        {
                            for (RPI::MeshDrawPacket& drawPacket : drawPacketList)
                            {
                                m_flagRegistry->VisitTags(
                                    [&](AZ::Name shaderOption, FlagRegistry::TagType tag)
                                    {
                                        bool shaderOptionValue = (modelHandle.m_cullable.m_shaderOptionFlags & tag.GetIndex()) > 0;
                                        drawPacket.SetShaderOption(shaderOption, AZ::RPI::ShaderOptionValue(shaderOptionValue));
                                    }
                                );
                                drawPacket.Update(*GetParentScene(), true);
                            }
                        }
                        modelHandle.m_flags.m_cullableNeedsRebuild = true;

                        // [GHI-13619]
                        // Update the draw packets on the cullable, since we just set a shader item.
                        // BuildCullable is a bit overkill here, this could be reduced to just updating the drawPacket specific info
                        // It's also going to cause m_cullableNeedsUpdate to be set, which will execute next frame, which we don't need
                        modelHandle.BuildCullable(this);
                    }
                }
            }

        }

        void MeshFeatureProcessor::OnEndPrepareRender()
        {
            m_meshDataChecker.soft_unlock();

            if (m_reportShaderOptionFlags)
            {
                m_reportShaderOptionFlags = false;
                PrintShaderOptionFlags();
            }
            for (auto& model : m_modelData)
            {
                model.m_cullable.m_prevShaderOptionFlags = model.m_cullable.m_shaderOptionFlags.exchange(0);
                model.m_cullable.m_flags = model.m_flags.m_isAlwaysDynamic ? m_meshMovedFlag.GetIndex() : 0;
            }
        }

        MeshFeatureProcessor::MeshHandle MeshFeatureProcessor::AcquireMesh(
            const MeshHandleDescriptor& descriptor, const CustomMaterialMap& materials)
        {
            AZ_PROFILE_SCOPE(AzRender, "MeshFeatureProcessor: AcquireMesh");

            // don't need to check the concurrency during emplace() because the StableDynamicArray won't move the other elements during
            // insertion
            MeshHandle meshDataHandle = m_modelData.emplace();

            meshDataHandle->m_descriptor = descriptor;
            meshDataHandle->m_scene = GetParentScene();
            meshDataHandle->m_customMaterials = materials;
            meshDataHandle->m_objectId = m_transformService->ReserveObjectId();
            meshDataHandle->m_rayTracingUuid = AZ::Uuid::CreateRandom();
            meshDataHandle->m_originalModelAsset = descriptor.m_modelAsset;
            meshDataHandle->m_meshLoader = AZStd::make_unique<ModelDataInstance::MeshLoader>(descriptor.m_modelAsset, &*meshDataHandle);
            meshDataHandle->m_flags.m_isAlwaysDynamic = descriptor.m_isAlwaysDynamic;

            if (descriptor.m_excludeFromReflectionCubeMaps)
            {
                meshDataHandle->m_cullable.m_cullData.m_hideFlags |= RPI::View::UsageReflectiveCubeMap;
            }

            return meshDataHandle;
        }

        MeshFeatureProcessor::MeshHandle MeshFeatureProcessor::AcquireMesh(
            const MeshHandleDescriptor& descriptor, const Data::Instance<RPI::Material>& material)
        {
            Render::CustomMaterialMap materials;
            if (material)
            {
                materials[AZ::Render::DefaultCustomMaterialId] = { material };
            }

            return AcquireMesh(descriptor, materials);
        }

        bool MeshFeatureProcessor::ReleaseMesh(MeshHandle& meshHandle)
        {
            if (meshHandle.IsValid())
            {
                meshHandle->m_meshLoader.reset();
                meshHandle->DeInit(this);
                m_transformService->ReleaseObjectId(meshHandle->m_objectId);

                AZStd::concurrency_check_scope scopeCheck(m_meshDataChecker);
                m_modelData.erase(meshHandle);

                return true;
            }
            return false;
        }

        MeshFeatureProcessor::MeshHandle MeshFeatureProcessor::CloneMesh(const MeshHandle& meshHandle)
        {
            if (meshHandle.IsValid())
            {
                return AcquireMesh(meshHandle->m_descriptor, meshHandle->m_customMaterials);
            }
            return MeshFeatureProcessor::MeshHandle();
        }

        Data::Instance<RPI::Model> MeshFeatureProcessor::GetModel(const MeshHandle& meshHandle) const
        {
            return meshHandle.IsValid() ? meshHandle->m_model : nullptr;
        }

        Data::Asset<RPI::ModelAsset> MeshFeatureProcessor::GetModelAsset(const MeshHandle& meshHandle) const
        {
            if (meshHandle.IsValid())
            {
                return meshHandle->m_originalModelAsset;
            }

            return {};
        }
        
        const RPI::MeshDrawPacketLods& MeshFeatureProcessor::GetDrawPackets(const MeshHandle& meshHandle) const
        {
            // This function is being deprecated. It's currently used to get draw packets so that we can print some
            // debug information about the draw packets in an imgui menu. But the ownership model for draw packets is changing.
            // We can no longer assume a meshHandle directly keeps a copy of all of its draw packets.

            return meshHandle.IsValid() && !r_meshInstancingEnabled ? meshHandle->m_drawPacketListsByLod : m_emptyDrawPacketLods;
        }

        const AZStd::vector<Data::Instance<RPI::ShaderResourceGroup>>& MeshFeatureProcessor::GetObjectSrgs(const MeshHandle& meshHandle) const
        {
            static AZStd::vector<Data::Instance<RPI::ShaderResourceGroup>> staticEmptyList;
            return meshHandle.IsValid() ? meshHandle->m_objectSrgList : staticEmptyList;
        }

        void MeshFeatureProcessor::QueueObjectSrgForCompile(const MeshHandle& meshHandle) const
        {
            if (meshHandle.IsValid())
            {
                meshHandle->m_flags.m_objectSrgNeedsUpdate = true;
            }
        }

        void MeshFeatureProcessor::SetCustomMaterials(const MeshHandle& meshHandle, const Data::Instance<RPI::Material>& material)
        {
            Render::CustomMaterialMap materials;
            materials[AZ::Render::DefaultCustomMaterialId] = { material };
            return SetCustomMaterials(meshHandle, materials);
        }

        void MeshFeatureProcessor::SetCustomMaterials(const MeshHandle& meshHandle, const CustomMaterialMap& materials)
        {
            if (meshHandle.IsValid())
            {
                meshHandle->m_customMaterials = materials;
                if (meshHandle->m_model)
                {
                    meshHandle->ReInit(this);
                }

                meshHandle->m_flags.m_objectSrgNeedsUpdate = true;
            }
        }

        const CustomMaterialMap& MeshFeatureProcessor::GetCustomMaterials(const MeshHandle& meshHandle) const
        {
            return meshHandle.IsValid() ? meshHandle->m_customMaterials : DefaultCustomMaterialMap;
        }

        void MeshFeatureProcessor::ConnectModelChangeEventHandler(const MeshHandle& meshHandle, ModelChangedEvent::Handler& handler)
        {
            if (meshHandle.IsValid())
            {
                handler.Connect(meshHandle->m_meshLoader->GetModelChangedEvent());
            }
        }

        void MeshFeatureProcessor::ConnectObjectSrgCreatedEventHandler(const MeshHandle& meshHandle, ObjectSrgCreatedEvent::Handler& handler)
        {
            if (meshHandle.IsValid())
            {
                handler.Connect(meshHandle->GetObjectSrgCreatedEvent());
            }
        }

        void MeshFeatureProcessor::SetTransform(const MeshHandle& meshHandle, const AZ::Transform& transform, const AZ::Vector3& nonUniformScale)
        {
            if (meshHandle.IsValid())
            {
                ModelDataInstance& modelData = *meshHandle;
                modelData.m_flags.m_cullBoundsNeedsUpdate = true;
                modelData.m_flags.m_objectSrgNeedsUpdate = true;
                modelData.m_cullable.m_flags = modelData.m_cullable.m_flags | m_meshMovedFlag.GetIndex();

                // Only set m_dynamic flag if the model instance is initialized.
                if (!modelData.m_flags.m_dynamic)
                {
                    modelData.m_flags.m_dynamic = (modelData.m_model && !modelData.m_flags.m_needsInit) ? true : false;

                    // Enable draw motion for all the DrawPacket referenced by this model
                    if (r_meshInstancingEnabled && modelData.m_flags.m_dynamic)
                    {
                        for (size_t lodIndex = 0; lodIndex < modelData.m_instanceGroupHandlesByLod.size(); ++lodIndex)
                        {
                            ModelDataInstance::InstanceGroupHandleList& instanceGroupHandles = modelData.m_instanceGroupHandlesByLod[lodIndex];
                            for (size_t meshIndex = 0; meshIndex < instanceGroupHandles.size(); ++meshIndex)
                            {
                                AZStd::scoped_lock<AZStd::mutex> scopedLock(instanceGroupHandles[meshIndex]->m_eventLock);
                                MeshInstanceGroupData& instanceGroupData = m_meshInstanceManager[instanceGroupHandles[meshIndex]];
                                if (!instanceGroupData.m_isDrawMotion)
                                {
                                    instanceGroupData.m_isDrawMotion = true;
                                    instanceGroupData.m_drawPacket.SetEnableDraw(m_meshMotionDrawListTag, true);
                                }
                            }
                        }
                    }
                }

                m_transformService->SetTransformForId(meshHandle->m_objectId, transform, nonUniformScale);

                // ray tracing data needs to be updated with the new transform
                if (m_rayTracingFeatureProcessor)
                {
                    m_rayTracingFeatureProcessor->SetMeshTransform(meshHandle->m_rayTracingUuid, transform, nonUniformScale);
                }
            }
        }

        void MeshFeatureProcessor::SetLocalAabb(const MeshHandle& meshHandle, const AZ::Aabb& localAabb)
        {
            if (meshHandle.IsValid())
            {
                ModelDataInstance& modelData = *meshHandle;
                modelData.m_aabb = localAabb;
                modelData.m_flags.m_cullBoundsNeedsUpdate = true;
                modelData.m_flags.m_objectSrgNeedsUpdate = true;
            }
        };

        AZ::Aabb MeshFeatureProcessor::GetLocalAabb(const MeshHandle& meshHandle) const
        {
            if (meshHandle.IsValid())
            {
                return meshHandle->m_aabb;
            }
            else
            {
                AZ_Assert(false, "Invalid mesh handle");
                return Aabb::CreateNull();
            }
        }

        Transform MeshFeatureProcessor::GetTransform(const MeshHandle& meshHandle)
        {
            if (meshHandle.IsValid())
            {
                return m_transformService->GetTransformForId(meshHandle->m_objectId);
            }
            else
            {
                AZ_Assert(false, "Invalid mesh handle");
                return Transform::CreateIdentity();
            }
        }

        Vector3 MeshFeatureProcessor::GetNonUniformScale(const MeshHandle& meshHandle)
        {
            if (meshHandle.IsValid())
            {
                return m_transformService->GetNonUniformScaleForId(meshHandle->m_objectId);
            }
            else
            {
                AZ_Assert(false, "Invalid mesh handle");
                return Vector3::CreateOne();
            }
        }

        void MeshFeatureProcessor::SetSortKey(const MeshHandle& meshHandle, RHI::DrawItemSortKey sortKey)
        {
            if (meshHandle.IsValid())
            {
                meshHandle->SetSortKey(this, sortKey);
            }
        }

        RHI::DrawItemSortKey MeshFeatureProcessor::GetSortKey(const MeshHandle& meshHandle) const
        {
            if (meshHandle.IsValid())
            {
                return meshHandle->GetSortKey();
            }
            else
            {
                AZ_Assert(false, "Invalid mesh handle");
                return 0;
            }
        }

        void MeshFeatureProcessor::SetMeshLodConfiguration(const MeshHandle& meshHandle, const RPI::Cullable::LodConfiguration& meshLodConfig)
        {
            if (meshHandle.IsValid())
            {
                meshHandle->SetMeshLodConfiguration(meshLodConfig);
            }
        }

        RPI::Cullable::LodConfiguration MeshFeatureProcessor::GetMeshLodConfiguration(const MeshHandle& meshHandle) const
        {
            if (meshHandle.IsValid())
            {
                return meshHandle->GetMeshLodConfiguration();
            }
            else
            {
                AZ_Assert(false, "Invalid mesh handle");
                return { RPI::Cullable::LodType::Default, 0, 0.0f, 0.0f };
            }
        }

        void MeshFeatureProcessor::SetIsAlwaysDynamic(const MeshHandle & meshHandle, bool isAlwaysDynamic)
        {
            if (meshHandle.IsValid())
            {
                meshHandle->m_flags.m_isAlwaysDynamic = isAlwaysDynamic;
            }
        }

        bool MeshFeatureProcessor::GetIsAlwaysDynamic(const MeshHandle& meshHandle) const
        {
            if (!meshHandle.IsValid())
            {
                AZ_Assert(false, "Invalid mesh handle");
                return false;
            }
            return meshHandle->m_flags.m_isAlwaysDynamic;
        }

        void MeshFeatureProcessor::SetExcludeFromReflectionCubeMaps(const MeshHandle& meshHandle, bool excludeFromReflectionCubeMaps)
        {
            if (meshHandle.IsValid())
            {
                meshHandle->m_descriptor.m_excludeFromReflectionCubeMaps = excludeFromReflectionCubeMaps;
                if (excludeFromReflectionCubeMaps)
                {
                    meshHandle->m_cullable.m_cullData.m_hideFlags |= RPI::View::UsageReflectiveCubeMap;
                }
                else
                {
                    meshHandle->m_cullable.m_cullData.m_hideFlags &= ~RPI::View::UsageReflectiveCubeMap;
                }
            }
        }

        bool MeshFeatureProcessor::GetExcludeFromReflectionCubeMaps(const MeshHandle& meshHandle) const
        {
            if (meshHandle.IsValid())
            {
                return meshHandle->m_descriptor.m_excludeFromReflectionCubeMaps;
            }
            return false;
        }

        void MeshFeatureProcessor::SetRayTracingEnabled(const MeshHandle& meshHandle, bool rayTracingEnabled)
        {
            if (meshHandle.IsValid())
            {
                // update the ray tracing data based on the current state and the new state
                if (rayTracingEnabled && !meshHandle->m_descriptor.m_isRayTracingEnabled)
                {
                    // add to ray tracing
                    meshHandle->m_flags.m_needsSetRayTracingData = true;
                }
                else if (!rayTracingEnabled && meshHandle->m_descriptor.m_isRayTracingEnabled)
                {
                    // remove from ray tracing
                    if (m_rayTracingFeatureProcessor)
                    {
                        m_rayTracingFeatureProcessor->RemoveMesh(meshHandle->m_rayTracingUuid);
                    }
                }

                // set new state
                meshHandle->m_descriptor.m_isRayTracingEnabled = rayTracingEnabled;
            }
        }

        bool MeshFeatureProcessor::GetRayTracingEnabled(const MeshHandle& meshHandle) const
        {
            if (meshHandle.IsValid())
            {
                return meshHandle->m_descriptor.m_isRayTracingEnabled;
            }
            else
            {
                AZ_Assert(false, "Invalid mesh handle");
                return false;
            }
        }

        bool MeshFeatureProcessor::GetVisible(const MeshHandle& meshHandle) const
        {
            if (meshHandle.IsValid())
            {
                return meshHandle->m_flags.m_visible;
            }
            return false;
        }

        void MeshFeatureProcessor::SetVisible(const MeshHandle& meshHandle, bool visible)
        {
            if (meshHandle.IsValid())
            {
                meshHandle->SetVisible(visible);

                if (m_rayTracingFeatureProcessor && meshHandle->m_descriptor.m_isRayTracingEnabled)
                {
                    // always remove from ray tracing first
                    m_rayTracingFeatureProcessor->RemoveMesh(meshHandle->m_rayTracingUuid);

                    // now add if it's visible
                    if (visible)
                    {
                        meshHandle->m_flags.m_needsSetRayTracingData = true;
                    }
                }
            }
        }

        void MeshFeatureProcessor::SetUseForwardPassIblSpecular(const MeshHandle& meshHandle, bool useForwardPassIblSpecular)
        {
            if (meshHandle.IsValid())
            {
                meshHandle->m_descriptor.m_useForwardPassIblSpecular = useForwardPassIblSpecular;
                meshHandle->m_flags.m_objectSrgNeedsUpdate = true;

                if (meshHandle->m_model)
                {
                    const size_t modelLodCount = meshHandle->m_model->GetLodCount();
                    for (size_t modelLodIndex = 0; modelLodIndex < modelLodCount; ++modelLodIndex)
                    {
                        meshHandle->BuildDrawPacketList(this, modelLodIndex);
                    }
                }
            }
        }

        void MeshFeatureProcessor::SetRayTracingDirty(const MeshHandle& meshHandle)
        {
            if (meshHandle.IsValid())
            {
                meshHandle->m_flags.m_needsSetRayTracingData = true;
            }
        }

        RHI::Ptr<MeshFeatureProcessor::FlagRegistry> MeshFeatureProcessor::GetShaderOptionFlagRegistry()
        {
            if (m_flagRegistry == nullptr)
            {
                m_flagRegistry = FlagRegistry::Create();
            }
            return m_flagRegistry;
        };

        void MeshFeatureProcessor::ForceRebuildDrawPackets([[maybe_unused]] const AZ::ConsoleCommandContainer& arguments)
        {
            m_forceRebuildDrawPackets = true;
        }

        void MeshFeatureProcessor::OnRenderPipelineChanged([[maybe_unused]] RPI::RenderPipeline* pipeline,
            [[maybe_unused]] RPI::SceneNotification::RenderPipelineChangeType changeType)
        {
            m_forceRebuildDrawPackets = true;
        }

        void MeshFeatureProcessor::UpdateMeshReflectionProbes()
        {
            for (auto& meshInstance : m_modelData)
            {
                // we need to rebuild the Srg for any meshes that are using the forward pass IBL specular option
                if (meshInstance.m_descriptor.m_useForwardPassIblSpecular)
                {
                    meshInstance.m_flags.m_objectSrgNeedsUpdate = true;
                }

                // update the raytracing reflection probe data if necessary
                RayTracingFeatureProcessor::Mesh::ReflectionProbe reflectionProbe;
                bool currentHasRayTracingReflectionProbe = meshInstance.m_flags.m_hasRayTracingReflectionProbe;
                meshInstance.SetRayTracingReflectionProbeData(this, reflectionProbe);

                if (meshInstance.m_flags.m_hasRayTracingReflectionProbe ||
                    (currentHasRayTracingReflectionProbe != meshInstance.m_flags.m_hasRayTracingReflectionProbe))
                {
                    m_rayTracingFeatureProcessor->SetMeshReflectionProbe(meshInstance.m_rayTracingUuid, reflectionProbe);
                }
            }
        }

        void MeshFeatureProcessor::ReportShaderOptionFlags([[maybe_unused]] const AZ::ConsoleCommandContainer& arguments)
        {
            m_reportShaderOptionFlags = true;
        }

        RayTracingFeatureProcessor* MeshFeatureProcessor::GetRayTracingFeatureProcessor() const
        {
            return m_rayTracingFeatureProcessor;
        }

        ReflectionProbeFeatureProcessor* MeshFeatureProcessor::GetReflectionProbeFeatureProcessor() const
        {
            return m_reflectionProbeFeatureProcessor;
        }

        TransformServiceFeatureProcessor* MeshFeatureProcessor::GetTransformServiceFeatureProcessor() const
        {
            return m_transformService;
        }

        MeshInstanceManager& MeshFeatureProcessor::GetMeshInstanceManager()
        {
            return m_meshInstanceManager;
        }

        bool MeshFeatureProcessor::IsMeshInstancingEnabled() const
        {
            return m_enableMeshInstancing;
        }

        void MeshFeatureProcessor::PrintShaderOptionFlags()
        {
            AZStd::map<FlagRegistry::TagType, AZ::Name> tags;
            AZStd::string registeredFoundMessage = "Registered flags: ";

            auto gatherTags = [&](const Name& name, FlagRegistry::TagType tag)
            {
                tags[tag] = name;
                registeredFoundMessage.append(name.GetCStr() + AZStd::string(", "));
            };

            m_flagRegistry->VisitTags(gatherTags);

            registeredFoundMessage.erase(registeredFoundMessage.end() - 2);

            AZ_Printf("MeshFeatureProcessor", registeredFoundMessage.c_str());

            AZStd::map<uint32_t, uint32_t> flagStats;

            for (auto& model : m_modelData)
            {
                ++flagStats[model.m_cullable.m_shaderOptionFlags.load()];
            }

            for (auto [flag, references] : flagStats)
            {
                AZStd::string flagList;

                if (flag == 0)
                {
                    flagList = "(None)";
                }
                else
                {
                    for (auto [tag, name] : tags)
                    {
                        if ((tag.GetIndex() & flag) > 0)
                        {
                            flagList.append(name.GetCStr());
                            flagList.append(", ");
                        }
                    }
                    flagList.erase(flagList.end() - 2);
                }

                AZ_Printf("MeshFeatureProcessor", "Found %u references to [%s]", references, flagList.c_str());
            }
        }

        // ModelDataInstance::MeshLoader...

        MeshFeatureProcessorInterface::ModelChangedEvent& ModelDataInstance::MeshLoader::GetModelChangedEvent()
        {
            return m_modelChangedEvent;
        }

        ModelDataInstance::MeshLoader::MeshLoader(const Data::Asset<RPI::ModelAsset>& modelAsset, ModelDataInstance* parent)
            : m_modelAsset(modelAsset)
            , m_parent(parent)
        {
            if (!m_modelAsset.GetId().IsValid())
            {
                AZ_Error("ModelDataInstance::MeshLoader", false, "Invalid model asset Id.");
                return;
            }

            if (!m_modelAsset.IsReady())
            {
                m_modelAsset.QueueLoad();
            }

            Data::AssetBus::Handler::BusConnect(modelAsset.GetId());
            AzFramework::AssetCatalogEventBus::Handler::BusConnect();
        }

        ModelDataInstance::MeshLoader::~MeshLoader()
        {
            AzFramework::AssetCatalogEventBus::Handler::BusDisconnect();
            Data::AssetBus::Handler::BusDisconnect();
        }

        //! AssetBus::Handler overrides...
        void ModelDataInstance::MeshLoader::OnAssetReady(Data::Asset<Data::AssetData> asset)
        {
            Data::Asset<RPI::ModelAsset> modelAsset = asset;

            // Assign the fully loaded asset back to the mesh handle to not only hold asset id, but the actual data as well.
            m_parent->m_originalModelAsset = asset;

            if (const auto& modelTags = modelAsset->GetTags(); !modelTags.empty())
            {
                RPI::AssetQuality highestLodBias = RPI::AssetQualityLowest;
                for (const AZ::Name& tag : modelTags)
                {
                    RPI::AssetQuality tagQuality = RPI::AssetQualityHighest;
                    RPI::ModelTagBus::BroadcastResult(tagQuality, &RPI::ModelTagBus::Events::GetQuality, tag);

                    highestLodBias = AZStd::min(highestLodBias, tagQuality);
                }

                if (highestLodBias >= modelAsset->GetLodCount())
                {
                    highestLodBias = aznumeric_caster(modelAsset->GetLodCount() - 1);
                }

                m_parent->m_lodBias = highestLodBias;

                for (const AZ::Name& tag : modelTags)
                {
                    RPI::ModelTagBus::Broadcast(&RPI::ModelTagBus::Events::RegisterAsset, tag, modelAsset->GetId());
                }
            }
            else
            {
                m_parent->m_lodBias = 0;
            }

            Data::Instance<RPI::Model> model;
            // Check if a requires cloning callback got set and if so check if cloning the model asset is requested.
            if (m_parent->m_descriptor.m_requiresCloneCallback &&
                m_parent->m_descriptor.m_requiresCloneCallback(modelAsset))
            {
                // Clone the model asset to force create another model instance.
                AZ::Data::AssetId newId(AZ::Uuid::CreateRandom(), /*subId=*/0);
                Data::Asset<RPI::ModelAsset> clonedAsset;
                // Assume cloned models will involve some kind of geometry deformation
                m_parent->m_flags.m_isAlwaysDynamic = true;
                if (AZ::RPI::ModelAssetCreator::Clone(modelAsset, clonedAsset, newId))
                {
                    model = RPI::Model::FindOrCreate(clonedAsset);
                }
                else
                {
                    AZ_Error("ModelDataInstance", false, "Cannot clone model for '%s'. Cloth simulation results won't be individual per entity.", modelAsset->GetName().GetCStr());
                    model = RPI::Model::FindOrCreate(modelAsset);
                }
            }
            else
            {
                // Static mesh, no cloth buffer present.
                model = RPI::Model::FindOrCreate(modelAsset);
            }
            
            if (model)
            {
                RayTracingFeatureProcessor* rayTracingFeatureProcessor =
                    m_parent->m_scene->GetFeatureProcessor<RayTracingFeatureProcessor>();
                m_parent->RemoveRayTracingData(rayTracingFeatureProcessor);
                m_parent->QueueInit(model);
                m_modelChangedEvent.Signal(AZStd::move(model));
            }
            else
            {
                //when running with null renderer, the RPI::Model::FindOrCreate(...) is expected to return nullptr, so suppress this error.
                AZ_Error(
                    "ModelDataInstance::OnAssetReady", RHI::IsNullRHI(), "Failed to create model instance for '%s'",
                    asset.GetHint().c_str());
            }
        }

        
        void ModelDataInstance::MeshLoader::OnModelReloaded(Data::Asset<Data::AssetData> asset)
        {
            OnAssetReady(asset);
        }

        void ModelDataInstance::MeshLoader::OnAssetError(Data::Asset<Data::AssetData> asset)
        {
            // Note: m_modelAsset and asset represents same asset, but only m_modelAsset contains the file path in its hint from serialization
            AZ_Error(
                "ModelDataInstance::MeshLoader", false, "Failed to load asset %s. It may be missing, or not be finished processing",
                m_modelAsset.GetHint().c_str());

            AzFramework::AssetSystemRequestBus::Broadcast(
                &AzFramework::AssetSystem::AssetSystemRequests::EscalateAssetByUuid, m_modelAsset.GetId().m_guid);
        }
        
        void ModelDataInstance::MeshLoader::OnCatalogAssetChanged(const AZ::Data::AssetId& assetId)
        {
            if (assetId == m_modelAsset.GetId())
            {
                Data::Asset<RPI::ModelAsset> modelAssetReference = m_modelAsset;

                // If the asset was modified, reload it
                AZ::SystemTickBus::QueueFunction(
                    [=]() mutable
                    {
                        ModelReloaderSystemInterface::Get()->ReloadModel(modelAssetReference, m_modelReloadedEventHandler);
                    });
            }
        }

        void ModelDataInstance::MeshLoader::OnCatalogAssetAdded(const AZ::Data::AssetId& assetId)
        {
            if (assetId == m_modelAsset.GetId())
            {
                Data::Asset<RPI::ModelAsset> modelAssetReference = m_modelAsset;
                
                // If the asset didn't exist in the catalog when it first attempted to load, we need to try loading it again
                AZ::SystemTickBus::QueueFunction(
                    [=]() mutable
                    {
                        ModelReloaderSystemInterface::Get()->ReloadModel(modelAssetReference, m_modelReloadedEventHandler);
                    });
            }
        }

        ModelDataInstance::ModelDataInstance()
        {
            m_flags.m_cullBoundsNeedsUpdate = false;
            m_flags.m_cullableNeedsRebuild = false;
            m_flags.m_needsInit = false;
            m_flags.m_objectSrgNeedsUpdate = true;
            m_flags.m_isAlwaysDynamic = false;
            m_flags.m_dynamic = false;
            m_flags.m_isDrawMotion = false;
            m_flags.m_visible = true;
            m_flags.m_useForwardPassIblSpecular = false;
            m_flags.m_hasForwardPassIblSpecularMaterial = false;
            m_flags.m_needsSetRayTracingData = false;
            m_flags.m_hasRayTracingReflectionProbe = false;
        }

        void ModelDataInstance::DeInit(MeshFeatureProcessor* meshFeatureProcessor)
        {
            RayTracingFeatureProcessor* rayTracingFeatureProcessor = meshFeatureProcessor->GetRayTracingFeatureProcessor();
            m_scene->GetCullingScene()->UnregisterCullable(m_cullable);

            RemoveRayTracingData(rayTracingFeatureProcessor);

            // We're intentionally using the MeshFeatureProcessor's value instead of using the cvar directly here,
            // because DeInit might be called after the cvar changes, but we want to do the de-initialization based
            // on what the setting was before (when the resources were initialized). The MeshFeatureProcessor will still have the cached value in that case
            if (!meshFeatureProcessor->IsMeshInstancingEnabled())
            {
                m_drawPacketListsByLod.clear();
            }
            else
            {
                // Remove all the meshes from the MeshInstanceManager
                MeshInstanceManager& meshInstanceManager = meshFeatureProcessor->GetMeshInstanceManager();
                AZ_Assert(
                    m_instanceGroupHandlesByLod.size() == m_updateDrawPacketEventHandlersByLod.size(),
                    "MeshFeatureProcessor: InstanceGroup handles and update draw packet event handlers do not match.");

                for (size_t lodIndex = 0; lodIndex < m_instanceGroupHandlesByLod.size(); ++lodIndex)
                {
                    InstanceGroupHandleList& instanceGroupHandles = m_instanceGroupHandlesByLod[lodIndex];
                    UpdateDrawPacketHandlerList& updateDrawPacketHandlers = m_updateDrawPacketEventHandlersByLod[lodIndex];
                    AZ_Assert(
                        instanceGroupHandles.size() == updateDrawPacketHandlers.size(),
                        "MeshFeatureProcessor: InstanceGroup handles and update draw packet event handlers do not match.");

                    for (size_t meshIndex = 0; meshIndex < instanceGroupHandles.size(); ++meshIndex)
                    {
                        {
                            // Disconnect the event handlers
                            AZStd::scoped_lock<AZStd::mutex> scopedLock(instanceGroupHandles[meshIndex]->m_eventLock);
                            updateDrawPacketHandlers[meshIndex].Disconnect();
                        }
                        
                        // Remove instance will decrement the use-count of the instance group, and only release the instance group
                        // if nothing else is referring to it.
                        meshInstanceManager.RemoveInstance(instanceGroupHandles[meshIndex]);
                    }
                    instanceGroupHandles.clear();
                    updateDrawPacketHandlers.clear();
                }
                m_instanceGroupHandlesByLod.clear();
                m_updateDrawPacketEventHandlersByLod.clear();
            }

            m_customMaterials.clear();
            m_objectSrgList = {};
            m_model = {};
        }

        void ModelDataInstance::ReInit(MeshFeatureProcessor* meshFeatureProcessor)
        {
            CustomMaterialMap customMaterials = m_customMaterials;
            const Data::Instance<RPI::Model> model = m_model;
            DeInit(meshFeatureProcessor);
            m_customMaterials = customMaterials;
            m_model = model;
            QueueInit(m_model);
        }

        void ModelDataInstance::QueueInit(const Data::Instance<RPI::Model>& model)
        {
            m_model = model;
            m_flags.m_needsInit = true;
            m_flags.m_isDrawMotion = m_flags.m_isAlwaysDynamic;
            m_aabb = m_model->GetModelAsset()->GetAabb();
        }

        void ModelDataInstance::Init(MeshFeatureProcessor* meshFeatureProcessor)
        {
            const size_t modelLodCount = m_model->GetLodCount();
            
            if (!r_meshInstancingEnabled)
            {
                m_drawPacketListsByLod.resize(modelLodCount);
            }
            else
            {
                m_instanceGroupHandlesByLod.resize(modelLodCount);
                m_updateDrawPacketEventHandlersByLod.resize(modelLodCount);
            }
            
            for (size_t modelLodIndex = 0; modelLodIndex < modelLodCount; ++modelLodIndex)
            {
                BuildDrawPacketList(meshFeatureProcessor, modelLodIndex);
            }

            for(auto& objectSrg : m_objectSrgList)
            {
                // Set object Id once since it never changes
                RHI::ShaderInputNameIndex objectIdIndex = "m_objectId";
                objectSrg->SetConstant(objectIdIndex, m_objectId.GetIndex());
                objectIdIndex.AssertValid();
            }

            if (m_flags.m_visible && m_descriptor.m_isRayTracingEnabled)
            {
                m_flags.m_needsSetRayTracingData = true;
            }

            m_flags.m_cullableNeedsRebuild = true;
            m_flags.m_cullBoundsNeedsUpdate = true;
            m_flags.m_objectSrgNeedsUpdate = true;
            m_flags.m_needsInit = false;
        }

        void ModelDataInstance::BuildDrawPacketList(MeshFeatureProcessor* meshFeatureProcessor, size_t modelLodIndex)
        {
            RPI::ModelLod& modelLod = *m_model->GetLods()[modelLodIndex];
            const size_t meshCount = modelLod.GetMeshes().size();
            MeshInstanceManager& meshInstanceManager = meshFeatureProcessor->GetMeshInstanceManager();

            if (!r_meshInstancingEnabled)
            {
                RPI::MeshDrawPacketList& drawPacketListOut = m_drawPacketListsByLod[modelLodIndex];
                drawPacketListOut.clear();
                drawPacketListOut.reserve(meshCount);
            }

            auto meshMotionDrawListTag = AZ::RHI::RHISystemInterface::Get()->GetDrawListTagRegistry()->FindTag(MeshCommon::MotionDrawListTagName);
            
            for (size_t meshIndex = 0; meshIndex < meshCount; ++meshIndex)
            {
                const RPI::ModelLod::Mesh& mesh = modelLod.GetMeshes()[meshIndex];

                // Determine if there is a custom material specified for this submission
                const CustomMaterialId customMaterialId(aznumeric_cast<AZ::u64>(modelLodIndex), mesh.m_materialSlotStableId);
                const auto& customMaterialInfo = GetCustomMaterialWithFallback(customMaterialId);
                const auto& material = customMaterialInfo.m_material ? customMaterialInfo.m_material : mesh.m_material;

                if (!material)
                {
                    AZ_Warning("MeshFeatureProcessor", false, "No material provided for mesh. Skipping.");
                    continue;
                }

                auto& objectSrgLayout = material->GetAsset()->GetObjectSrgLayout();

                if (!objectSrgLayout)
                {
                    AZ_Warning("MeshFeatureProcessor", false, "No per-object ShaderResourceGroup found.");
                    continue;
                }

                Data::Instance<RPI::ShaderResourceGroup> meshObjectSrg;

                // See if the object SRG for this mesh is already in our list of object SRGs
                for (auto& objectSrgIter : m_objectSrgList)
                {
                    if (objectSrgIter->GetLayout()->GetHash() == objectSrgLayout->GetHash())
                    {
                        meshObjectSrg = objectSrgIter;
                    }
                }

                // If the object SRG for this mesh was not already in the list, create it and add it to the list
                if (!meshObjectSrg)
                {
                    auto& shaderAsset = material->GetAsset()->GetMaterialTypeAsset()->GetShaderAssetForObjectSrg();
                    meshObjectSrg = RPI::ShaderResourceGroup::Create(shaderAsset, objectSrgLayout->GetName());
                    if (!meshObjectSrg)
                    {
                        AZ_Warning("MeshFeatureProcessor", false, "Failed to create a new shader resource group, skipping.");
                        continue;
                    }
                    m_objectSrgCreatedEvent.Signal(meshObjectSrg);
                    m_objectSrgList.push_back(meshObjectSrg);
                }

                
                bool materialRequiresForwardPassIblSpecular = MaterialRequiresForwardPassIblSpecular(material);

                // Track whether any materials in this mesh require ForwardPassIblSpecular, we need this information when the ObjectSrg is
                // updated
                m_flags.m_hasForwardPassIblSpecularMaterial |= materialRequiresForwardPassIblSpecular;

                MeshInstanceManager::InsertResult instanceGroupInsertResult{ MeshInstanceManager::Handle{}, 0 };

                if (r_meshInstancingEnabled)
                {
                    // Get the instance index for referencing the draw packet
                    MeshInstanceGroupKey key{};

                    // Only meshes from the same model and lod with a matching material instance can be instanced
                    key.m_modelId = m_model->GetId();
                    key.m_lodIndex = static_cast<uint32_t>(modelLodIndex);
                    key.m_meshIndex = static_cast<uint32_t>(meshIndex);
                    key.m_materialId = material->GetId();

                    // Two meshes that could otherwise be instanced but have manually specified sort keys will not be instanced together
                    key.m_sortKey = m_sortKey;

                    // Using a random uuid will force this mesh into it's own unique instance group, which is always done for now since
                    // no actual instancing is supported yet.
                    key.m_forceInstancingOff = Uuid::CreateRandom();

                    instanceGroupInsertResult = meshInstanceManager.AddInstance(key);
                    m_instanceGroupHandlesByLod[modelLodIndex].push_back(instanceGroupInsertResult.m_handle);

                    // Add an update draw packet event handler for the current mesh
                    m_updateDrawPacketEventHandlersByLod[modelLodIndex].push_back(AZ::Event<>::Handler{
                        [this]()
                        {
                            HandleDrawPacketUpdate();
                        }});
                    // Connect to the update draw packet event
                    {
                        AZStd::scoped_lock<AZStd::mutex> scopedLock(instanceGroupInsertResult.m_handle->m_eventLock);
                        m_updateDrawPacketEventHandlersByLod[modelLodIndex][meshIndex].Connect(
                            instanceGroupInsertResult.m_handle->m_updateDrawPacketEvent);
                    }
                }

                // If this condition is true, we're dealing with a new, uninitialized draw packet, either because instancing is disabled
                // or because this was the first object in the instance group. So we need to initialize it
                if (!r_meshInstancingEnabled || instanceGroupInsertResult.m_instanceCount == 1)
                {
                    // setup the mesh draw packet
                    RPI::MeshDrawPacket drawPacket(
                        modelLod,
                        meshIndex,
                        material,
                        meshObjectSrg,
                        customMaterialInfo.m_uvMapping);

                    // set the shader option to select forward pass IBL specular if necessary
                    if (!drawPacket.SetShaderOption(s_o_meshUseForwardPassIBLSpecular_Name, AZ::RPI::ShaderOptionValue{ m_descriptor.m_useForwardPassIblSpecular }))
                    {
                        AZ_Warning("MeshDrawPacket", false, "Failed to set o_meshUseForwardPassIBLSpecular on mesh draw packet");
                    }

                    // stencil bits
                    uint8_t stencilRef = m_descriptor.m_useForwardPassIblSpecular || materialRequiresForwardPassIblSpecular
                        ? Render::StencilRefs::None
                        : Render::StencilRefs::UseIBLSpecularPass;
                    stencilRef |= Render::StencilRefs::UseDiffuseGIPass;

                    drawPacket.SetStencilRef(stencilRef);
                    drawPacket.SetSortKey(m_sortKey);
                    drawPacket.SetEnableDraw(meshMotionDrawListTag, m_flags.m_isDrawMotion);
                    drawPacket.Update(*m_scene, false);

                    if (!r_meshInstancingEnabled)
                    {
                        m_drawPacketListsByLod[modelLodIndex].emplace_back(AZStd::move(drawPacket));
                    }
                    else
                    {
                        MeshInstanceGroupData& instanceGroupData = meshInstanceManager[instanceGroupInsertResult.m_handle];
                        instanceGroupData.m_drawPacket = drawPacket;
                        instanceGroupData.m_isDrawMotion = m_flags.m_isDrawMotion;

                        // We're going to need an interval for the root constant data that we update every frame for each draw item, so cache that here
                        CacheRootConstantInterval(instanceGroupData);
                    }
                }

                // For mesh instancing only
                // If this model needs to draw motion, enable draw motion vector for the DrawPacket.
                // This means any mesh instances which are using this draw packet would draw motion vector too. This is fine, just not optimized. 
                if (r_meshInstancingEnabled && m_flags.m_isDrawMotion)
                {
                    MeshInstanceGroupData& instanceGroupData = meshInstanceManager[instanceGroupInsertResult.m_handle];
                    if (!instanceGroupData.m_isDrawMotion)
                    {
                        instanceGroupData.m_isDrawMotion = true;
                        instanceGroupData.m_drawPacket.SetEnableDraw(meshMotionDrawListTag, true);
                    }
                }
            }
        }

        void ModelDataInstance::SetRayTracingData(MeshFeatureProcessor* meshFeatureProcessor)
        {
            RayTracingFeatureProcessor* rayTracingFeatureProcessor = meshFeatureProcessor->GetRayTracingFeatureProcessor();
            TransformServiceFeatureProcessor* transformServiceFeatureProcessor =
                meshFeatureProcessor->GetTransformServiceFeatureProcessor();
            RemoveRayTracingData(rayTracingFeatureProcessor);

            if (!m_model)
            {
                return;
            }

            if (!rayTracingFeatureProcessor)
            {
                return;
            }

            const AZStd::span<const Data::Instance<RPI::ModelLod>>& modelLods = m_model->GetLods();
            if (modelLods.empty())
            {
                return;
            }

            // use the lowest LOD for raytracing
            uint32_t rayTracingLod = aznumeric_cast<uint32_t>(modelLods.size() - 1);
            const Data::Instance<RPI::ModelLod>& modelLod = modelLods[rayTracingLod];

            // setup a stream layout and shader input contract for the vertex streams
            static const char* PositionSemantic = "POSITION";
            static const char* NormalSemantic = "NORMAL";
            static const char* TangentSemantic = "TANGENT";
            static const char* BitangentSemantic = "BITANGENT";
            static const char* UVSemantic = "UV";
            static const RHI::Format PositionStreamFormat = RHI::Format::R32G32B32_FLOAT;
            static const RHI::Format NormalStreamFormat = RHI::Format::R32G32B32_FLOAT;
            static const RHI::Format TangentStreamFormat = RHI::Format::R32G32B32A32_FLOAT;
            static const RHI::Format BitangentStreamFormat = RHI::Format::R32G32B32_FLOAT;
            static const RHI::Format UVStreamFormat = RHI::Format::R32G32_FLOAT;

            RHI::InputStreamLayoutBuilder layoutBuilder;
            layoutBuilder.AddBuffer()->Channel(PositionSemantic, PositionStreamFormat);
            layoutBuilder.AddBuffer()->Channel(NormalSemantic, NormalStreamFormat);
            layoutBuilder.AddBuffer()->Channel(UVSemantic, UVStreamFormat);
            layoutBuilder.AddBuffer()->Channel(TangentSemantic, TangentStreamFormat);
            layoutBuilder.AddBuffer()->Channel(BitangentSemantic, BitangentStreamFormat);
            RHI::InputStreamLayout inputStreamLayout = layoutBuilder.End();

            RPI::ShaderInputContract::StreamChannelInfo positionStreamChannelInfo;
            positionStreamChannelInfo.m_semantic = RHI::ShaderSemantic(AZ::Name(PositionSemantic));
            positionStreamChannelInfo.m_componentCount = RHI::GetFormatComponentCount(PositionStreamFormat);

            RPI::ShaderInputContract::StreamChannelInfo normalStreamChannelInfo;
            normalStreamChannelInfo.m_semantic = RHI::ShaderSemantic(AZ::Name(NormalSemantic));
            normalStreamChannelInfo.m_componentCount = RHI::GetFormatComponentCount(NormalStreamFormat);

            RPI::ShaderInputContract::StreamChannelInfo tangentStreamChannelInfo;
            tangentStreamChannelInfo.m_semantic = RHI::ShaderSemantic(AZ::Name(TangentSemantic));
            tangentStreamChannelInfo.m_componentCount = RHI::GetFormatComponentCount(TangentStreamFormat);
            tangentStreamChannelInfo.m_isOptional = true;

            RPI::ShaderInputContract::StreamChannelInfo bitangentStreamChannelInfo;
            bitangentStreamChannelInfo.m_semantic = RHI::ShaderSemantic(AZ::Name(BitangentSemantic));
            bitangentStreamChannelInfo.m_componentCount = RHI::GetFormatComponentCount(BitangentStreamFormat);
            bitangentStreamChannelInfo.m_isOptional = true;

            RPI::ShaderInputContract::StreamChannelInfo uvStreamChannelInfo;
            uvStreamChannelInfo.m_semantic = RHI::ShaderSemantic(AZ::Name(UVSemantic));
            uvStreamChannelInfo.m_componentCount = RHI::GetFormatComponentCount(UVStreamFormat);
            uvStreamChannelInfo.m_isOptional = true;

            RPI::ShaderInputContract shaderInputContract;
            shaderInputContract.m_streamChannels.emplace_back(positionStreamChannelInfo);
            shaderInputContract.m_streamChannels.emplace_back(normalStreamChannelInfo);
            shaderInputContract.m_streamChannels.emplace_back(tangentStreamChannelInfo);
            shaderInputContract.m_streamChannels.emplace_back(bitangentStreamChannelInfo);
            shaderInputContract.m_streamChannels.emplace_back(uvStreamChannelInfo);

            // setup the raytracing data for each sub-mesh 
            const size_t meshCount = modelLod->GetMeshes().size();
            RayTracingFeatureProcessor::SubMeshVector subMeshes;
            for (uint32_t meshIndex = 0; meshIndex < meshCount; ++meshIndex)
            {
                const RPI::ModelLod::Mesh& mesh = modelLod->GetMeshes()[meshIndex];

                // retrieve the material
                const CustomMaterialId customMaterialId(rayTracingLod, mesh.m_materialSlotStableId);
                const auto& customMaterialInfo = GetCustomMaterialWithFallback(customMaterialId);
                const auto& material = customMaterialInfo.m_material ? customMaterialInfo.m_material : mesh.m_material;

                if (!material)
                {
                    AZ_Warning("MeshFeatureProcessor", false, "No material provided for mesh. Skipping.");
                    continue;
                }

                // retrieve vertex/index buffers
                RPI::ModelLod::StreamBufferViewList streamBufferViews;
                [[maybe_unused]] bool result = modelLod->GetStreamsForMesh(
                    inputStreamLayout,
                    streamBufferViews,
                    nullptr,
                    shaderInputContract,
                    meshIndex,
                    customMaterialInfo.m_uvMapping,
                    material->GetAsset()->GetMaterialTypeAsset()->GetUvNameMap());
                AZ_Assert(result, "Failed to retrieve mesh stream buffer views");

                // note that the element count is the size of the entire buffer, even though this mesh may only
                // occupy a portion of the vertex buffer.  This is necessary since we are accessing it using
                // a ByteAddressBuffer in the raytracing shaders and passing the byte offset to the shader in a constant buffer.
                uint32_t positionBufferByteCount = static_cast<uint32_t>(const_cast<RHI::Buffer*>(streamBufferViews[0].GetBuffer())->GetDescriptor().m_byteCount);
                RHI::BufferViewDescriptor positionBufferDescriptor = RHI::BufferViewDescriptor::CreateRaw(0, positionBufferByteCount);

                uint32_t normalBufferByteCount = static_cast<uint32_t>(const_cast<RHI::Buffer*>(streamBufferViews[1].GetBuffer())->GetDescriptor().m_byteCount);
                RHI::BufferViewDescriptor normalBufferDescriptor = RHI::BufferViewDescriptor::CreateRaw(0, normalBufferByteCount);

                uint32_t tangentBufferByteCount = static_cast<uint32_t>(const_cast<RHI::Buffer*>(streamBufferViews[2].GetBuffer())->GetDescriptor().m_byteCount);
                RHI::BufferViewDescriptor tangentBufferDescriptor = RHI::BufferViewDescriptor::CreateRaw(0, tangentBufferByteCount);

                uint32_t bitangentBufferByteCount = static_cast<uint32_t>(const_cast<RHI::Buffer*>(streamBufferViews[3].GetBuffer())->GetDescriptor().m_byteCount);
                RHI::BufferViewDescriptor bitangentBufferDescriptor = RHI::BufferViewDescriptor::CreateRaw(0, bitangentBufferByteCount);

                uint32_t uvBufferByteCount = static_cast<uint32_t>(const_cast<RHI::Buffer*>(streamBufferViews[4].GetBuffer())->GetDescriptor().m_byteCount);
                RHI::BufferViewDescriptor uvBufferDescriptor = RHI::BufferViewDescriptor::CreateRaw(0, uvBufferByteCount);

                const RHI::IndexBufferView& indexBufferView = mesh.m_indexBufferView;
                uint32_t indexElementSize = indexBufferView.GetIndexFormat() == RHI::IndexFormat::Uint16 ? 2 : 4;
                uint32_t indexElementCount = (uint32_t)indexBufferView.GetBuffer()->GetDescriptor().m_byteCount / indexElementSize;
                RHI::BufferViewDescriptor indexBufferDescriptor;
                indexBufferDescriptor.m_elementOffset = 0;
                indexBufferDescriptor.m_elementCount = indexElementCount;
                indexBufferDescriptor.m_elementSize = indexElementSize;
                indexBufferDescriptor.m_elementFormat = indexBufferView.GetIndexFormat() == RHI::IndexFormat::Uint16 ? RHI::Format::R16_UINT : RHI::Format::R32_UINT;

                // set the SubMesh data to pass to the RayTracingFeatureProcessor, starting with vertex/index data
                RayTracingFeatureProcessor::SubMesh subMesh;
                subMesh.m_positionFormat = PositionStreamFormat;
                subMesh.m_positionVertexBufferView = streamBufferViews[0];
                subMesh.m_positionShaderBufferView = const_cast<RHI::Buffer*>(streamBufferViews[0].GetBuffer())->GetBufferView(positionBufferDescriptor);

                subMesh.m_normalFormat = NormalStreamFormat;
                subMesh.m_normalVertexBufferView = streamBufferViews[1];
                subMesh.m_normalShaderBufferView = const_cast<RHI::Buffer*>(streamBufferViews[1].GetBuffer())->GetBufferView(normalBufferDescriptor);

                if (tangentBufferByteCount > 0)
                {
                    subMesh.m_bufferFlags |= RayTracingSubMeshBufferFlags::Tangent;
                    subMesh.m_tangentFormat = TangentStreamFormat;
                    subMesh.m_tangentVertexBufferView = streamBufferViews[2];
                    subMesh.m_tangentShaderBufferView = const_cast<RHI::Buffer*>(streamBufferViews[2].GetBuffer())->GetBufferView(tangentBufferDescriptor);
                }

                if (bitangentBufferByteCount > 0)
                {
                    subMesh.m_bufferFlags |= RayTracingSubMeshBufferFlags::Bitangent;
                    subMesh.m_bitangentFormat = BitangentStreamFormat;
                    subMesh.m_bitangentVertexBufferView = streamBufferViews[3];
                    subMesh.m_bitangentShaderBufferView = const_cast<RHI::Buffer*>(streamBufferViews[3].GetBuffer())->GetBufferView(bitangentBufferDescriptor);
                }

                if (uvBufferByteCount > 0)
                {
                    subMesh.m_bufferFlags |= RayTracingSubMeshBufferFlags::UV;
                    subMesh.m_uvFormat = UVStreamFormat;
                    subMesh.m_uvVertexBufferView = streamBufferViews[4];
                    subMesh.m_uvShaderBufferView = const_cast<RHI::Buffer*>(streamBufferViews[4].GetBuffer())->GetBufferView(uvBufferDescriptor);
                }

                subMesh.m_indexBufferView = mesh.m_indexBufferView;
                subMesh.m_indexShaderBufferView = const_cast<RHI::Buffer*>(mesh.m_indexBufferView.GetBuffer())->GetBufferView(indexBufferDescriptor);

                // add material data
                if (material)
                {
                    RPI::MaterialPropertyIndex propertyIndex;

                    // base color
                    propertyIndex = material->FindPropertyIndex(s_baseColor_color_Name);
                    if (propertyIndex.IsValid())
                    {
                        subMesh.m_baseColor = material->GetPropertyValue<AZ::Color>(propertyIndex);
                    }

                    propertyIndex = material->FindPropertyIndex(s_baseColor_factor_Name);
                    if (propertyIndex.IsValid())
                    {
                        subMesh.m_baseColor *= material->GetPropertyValue<float>(propertyIndex);
                    }

                    // metallic
                    propertyIndex = material->FindPropertyIndex(s_metallic_factor_Name);
                    if (propertyIndex.IsValid())
                    {
                        subMesh.m_metallicFactor = material->GetPropertyValue<float>(propertyIndex);
                    }

                    // roughness
                    propertyIndex = material->FindPropertyIndex(s_roughness_factor_Name);
                    if (propertyIndex.IsValid())
                    {
                        subMesh.m_roughnessFactor = material->GetPropertyValue<float>(propertyIndex);
                    }

                    // emissive color
                    propertyIndex = material->FindPropertyIndex(s_emissive_enable_Name);
                    if (propertyIndex.IsValid())
                    {
                        if (material->GetPropertyValue<bool>(propertyIndex))
                        {
                            propertyIndex = material->FindPropertyIndex(s_emissive_color_Name);
                            if (propertyIndex.IsValid())
                            {
                                subMesh.m_emissiveColor = material->GetPropertyValue<AZ::Color>(propertyIndex);
                            }

                            propertyIndex = material->FindPropertyIndex(s_emissive_intensity_Name);
                            if (propertyIndex.IsValid())
                            {
                                subMesh.m_emissiveColor *= material->GetPropertyValue<float>(propertyIndex);
                            }
                        }
                    }

                    // textures
                    Data::Instance<RPI::Image> baseColorImage; // can be used for irradiance color below
                    propertyIndex = material->FindPropertyIndex(s_baseColor_textureMap_Name);
                    if (propertyIndex.IsValid())
                    {
                        Data::Instance<RPI::Image> image = material->GetPropertyValue<Data::Instance<RPI::Image>>(propertyIndex);
                        if (image.get())
                        {
                            subMesh.m_textureFlags |= RayTracingSubMeshTextureFlags::BaseColor;
                            subMesh.m_baseColorImageView = image->GetImageView();
                            baseColorImage = image;
                        }
                    }

                    propertyIndex = material->FindPropertyIndex(s_normal_textureMap_Name);
                    if (propertyIndex.IsValid())
                    {
                        Data::Instance<RPI::Image> image = material->GetPropertyValue<Data::Instance<RPI::Image>>(propertyIndex);
                        if (image.get())
                        {
                            subMesh.m_textureFlags |= RayTracingSubMeshTextureFlags::Normal;
                            subMesh.m_normalImageView = image->GetImageView();
                        }
                    }

                    propertyIndex = material->FindPropertyIndex(s_metallic_textureMap_Name);
                    if (propertyIndex.IsValid())
                    {
                        Data::Instance<RPI::Image> image = material->GetPropertyValue<Data::Instance<RPI::Image>>(propertyIndex);
                        if (image.get())
                        {
                            subMesh.m_textureFlags |= RayTracingSubMeshTextureFlags::Metallic;
                            subMesh.m_metallicImageView = image->GetImageView();
                        }
                    }

                    propertyIndex = material->FindPropertyIndex(s_roughness_textureMap_Name);
                    if (propertyIndex.IsValid())
                    {
                        Data::Instance<RPI::Image> image = material->GetPropertyValue<Data::Instance<RPI::Image>>(propertyIndex);
                        if (image.get())
                        {
                            subMesh.m_textureFlags |= RayTracingSubMeshTextureFlags::Roughness;
                            subMesh.m_roughnessImageView = image->GetImageView();
                        }
                    }

                    propertyIndex = material->FindPropertyIndex(s_emissive_textureMap_Name);
                    if (propertyIndex.IsValid())
                    {
                        Data::Instance<RPI::Image> image = material->GetPropertyValue<Data::Instance<RPI::Image>>(propertyIndex);
                        if (image.get())
                        {
                            subMesh.m_textureFlags |= RayTracingSubMeshTextureFlags::Emissive;
                            subMesh.m_emissiveImageView = image->GetImageView();
                        }
                    }

                    // irradiance color
                    SetIrradianceData(subMesh, material, baseColorImage);
                }

                subMeshes.push_back(subMesh);
            }

            // setup the RayTracing Mesh
            RayTracingFeatureProcessor::Mesh rayTracingMesh;
            rayTracingMesh.m_assetId = m_model->GetModelAsset()->GetId();
            rayTracingMesh.m_transform = transformServiceFeatureProcessor->GetTransformForId(m_objectId);
            rayTracingMesh.m_nonUniformScale = transformServiceFeatureProcessor->GetNonUniformScaleForId(m_objectId);

            // setup the reflection probe data, and track if this mesh is currently affected by a reflection probe
            SetRayTracingReflectionProbeData(meshFeatureProcessor, rayTracingMesh.m_reflectionProbe);

            // add the mesh
            rayTracingFeatureProcessor->AddMesh(m_rayTracingUuid, rayTracingMesh, subMeshes);
            m_flags.m_needsSetRayTracingData = false;
        }

        void ModelDataInstance::SetIrradianceData(
            RayTracingFeatureProcessor::SubMesh& subMesh,
            const Data::Instance<RPI::Material> material,
            const Data::Instance<RPI::Image> baseColorImage)
        {
            RPI::MaterialPropertyIndex propertyIndex = material->FindPropertyIndex(s_irradiance_irradianceColorSource_Name);
            if (!propertyIndex.IsValid())
            {
                return;
            }

            uint32_t enumVal = material->GetPropertyValue<uint32_t>(propertyIndex);
            AZ::Name irradianceColorSource = material->GetMaterialPropertiesLayout()->GetPropertyDescriptor(propertyIndex)->GetEnumName(enumVal);

            if (irradianceColorSource.IsEmpty() || irradianceColorSource == s_Manual_Name)
            {
                propertyIndex = material->FindPropertyIndex(s_irradiance_manualColor_Name);
                if (propertyIndex.IsValid())
                {
                    subMesh.m_irradianceColor = material->GetPropertyValue<AZ::Color>(propertyIndex);
                }
                else
                {
                    // Couldn't find irradiance.manualColor -> check for an irradiance.color in case the material type
                    // doesn't have the concept of manual vs. automatic irradiance color, allow a simpler property name
                    propertyIndex = material->FindPropertyIndex(s_irradiance_color_Name);
                    if (propertyIndex.IsValid())
                    {
                        subMesh.m_irradianceColor = material->GetPropertyValue<AZ::Color>(propertyIndex);
                    }
                    else
                    {
                        AZ_Warning(
                            "MeshFeatureProcessor", false,
                            "No irradiance.manualColor or irradiance.color field found. Defaulting to 1.0f.");
                        subMesh.m_irradianceColor = AZ::Colors::White;
                    }
                }
            }
            else if (irradianceColorSource == s_BaseColorTint_Name)
            {
                // Use only the baseColor, no texture on top of it
                subMesh.m_irradianceColor = subMesh.m_baseColor;
            }
            else if (irradianceColorSource == s_BaseColor_Name)
            {
                // Check if texturing is enabled
                bool useTexture;
                propertyIndex = material->FindPropertyIndex(s_baseColor_useTexture_Name);
                if (propertyIndex.IsValid())
                {
                    useTexture = material->GetPropertyValue<bool>(propertyIndex);
                }
                else
                {
                    // No explicit baseColor.useTexture switch found, assuming the user wants to use
                    // a texture if a texture was found.
                    useTexture = true;
                }

                // If texturing was requested: check if we found a texture and use it
                if (useTexture && baseColorImage.get())
                {
                    // Currently GetAverageColor() is only implemented for a StreamingImage
                    auto baseColorStreamingImg = azdynamic_cast<RPI::StreamingImage*>(baseColorImage.get());
                    if (baseColorStreamingImg)
                    {
                        // Note: there are quite a few hidden assumptions in using the average
                        // texture color. For instance, (1) it assumes that every texel in the
                        // texture actually gets mapped to the surface (or non-mapped regions are
                        // colored with a meaningful 'average' color, or have zero opacity); (2) it
                        // assumes that the mapping from uv space to the mesh surface is
                        // (approximately) area-preserving to get a properly weighted average; and
                        // mostly, (3) it assumes that a single 'average color' is a meaningful
                        // characterisation of the full material.
                        Color avgColor = baseColorStreamingImg->GetAverageColor();

                        // We do a simple 'multiply' blend with the base color
                        // Note: other blend modes are currently not supported
                        subMesh.m_irradianceColor = avgColor * subMesh.m_baseColor;
                    }
                    else
                    {
                        AZ_Warning("MeshFeatureProcessor", false, "Using BaseColor as irradianceColorSource "
                                "is currently only supported for textures of type StreamingImage");
                        // Default to the flat base color
                        subMesh.m_irradianceColor = subMesh.m_baseColor;
                    }
                }
                else
                {
                    // No texture, simply copy the baseColor
                    subMesh.m_irradianceColor = subMesh.m_baseColor;
                }
            }
            else
            {
                AZ_Warning("MeshFeatureProcessor", false, "Unknown irradianceColorSource value: %s, "
                        "defaulting to 1.0f.", irradianceColorSource.GetCStr());
                subMesh.m_irradianceColor = AZ::Colors::White;
            }


            // Overall scale factor
            propertyIndex = material->FindPropertyIndex(s_irradiance_factor_Name);
            if (propertyIndex.IsValid())
            {
                subMesh.m_irradianceColor *= material->GetPropertyValue<float>(propertyIndex);
            }

            // set the raytracing transparency from the material opacity factor
            float opacity = 1.0f;
            propertyIndex = material->FindPropertyIndex(s_opacity_mode_Name);
            if (propertyIndex.IsValid())
            {
                // only query the opacity factor if it's a non-Opaque mode
                uint32_t mode = material->GetPropertyValue<uint32_t>(propertyIndex);
                if (mode > 0)
                {
                    propertyIndex = material->FindPropertyIndex(s_opacity_factor_Name);
                    if (propertyIndex.IsValid())
                    {
                        opacity = material->GetPropertyValue<float>(propertyIndex);
                    }
                }
            }

            subMesh.m_irradianceColor.SetA(opacity);
        }

        void ModelDataInstance::SetRayTracingReflectionProbeData(
            MeshFeatureProcessor* meshFeatureProcessor,
            RayTracingFeatureProcessor::Mesh::ReflectionProbe& reflectionProbe)
        {
            TransformServiceFeatureProcessor* transformServiceFeatureProcessor = meshFeatureProcessor->GetTransformServiceFeatureProcessor();
            ReflectionProbeFeatureProcessor* reflectionProbeFeatureProcessor = meshFeatureProcessor->GetReflectionProbeFeatureProcessor();
            AZ::Transform transform = transformServiceFeatureProcessor->GetTransformForId(m_objectId);

            // retrieve reflection probes
            Aabb aabbWS = m_aabb;
            aabbWS.ApplyTransform(transform);

            ReflectionProbeHandleVector reflectionProbeHandles;
            reflectionProbeFeatureProcessor->FindReflectionProbes(aabbWS, reflectionProbeHandles);

            m_flags.m_hasRayTracingReflectionProbe = !reflectionProbeHandles.empty();
            if (m_flags.m_hasRayTracingReflectionProbe)
            {
                // take the last handle from the list, which will be the smallest (most influential) probe
                ReflectionProbeHandle handle = reflectionProbeHandles.back();
                reflectionProbe.m_modelToWorld = reflectionProbeFeatureProcessor->GetTransform(handle);
                reflectionProbe.m_outerObbHalfLengths = reflectionProbeFeatureProcessor->GetOuterObbWs(handle).GetHalfLengths();
                reflectionProbe.m_innerObbHalfLengths = reflectionProbeFeatureProcessor->GetInnerObbWs(handle).GetHalfLengths();
                reflectionProbe.m_useParallaxCorrection = reflectionProbeFeatureProcessor->GetUseParallaxCorrection(handle);
                reflectionProbe.m_exposure = reflectionProbeFeatureProcessor->GetRenderExposure(handle);
                reflectionProbe.m_reflectionProbeCubeMap = reflectionProbeFeatureProcessor->GetCubeMap(handle);
            }
        }

        void ModelDataInstance::RemoveRayTracingData(RayTracingFeatureProcessor* rayTracingFeatureProcessor)
        {
            // remove from ray tracing
            if (rayTracingFeatureProcessor)
            {
                rayTracingFeatureProcessor->RemoveMesh(m_rayTracingUuid);
            }
        }

        void ModelDataInstance::SetSortKey(MeshFeatureProcessor* meshFeatureProcessor, RHI::DrawItemSortKey sortKey)
        {
            RHI::DrawItemSortKey previousSortKey = m_sortKey;
            m_sortKey = sortKey;
            if (previousSortKey != m_sortKey)
            {
                if (!r_meshInstancingEnabled)
                {
                    for (auto& drawPacketList : m_drawPacketListsByLod)
                    {
                        for (auto& drawPacket : drawPacketList)
                        {
                            drawPacket.SetSortKey(sortKey);
                        }
                    }
                }
                else
                {
                    // If the ModelDataInstance has already been initialized
                    if (m_model && !m_flags.m_needsInit)
                    {
                        // DeInit/ReInit is overkill (destroys and re-creates ray-tracing data)
                        // but it works for now since SetSortKey is infrequent
                        // Init needs to be called because that is where we determine what can be part of the same instance group,
                        // and the sort key is part of that.
                        ReInit(meshFeatureProcessor);
                    }
                }
            }
        }

        RHI::DrawItemSortKey ModelDataInstance::GetSortKey() const
        {
            return m_sortKey;
        }

        void ModelDataInstance::SetMeshLodConfiguration(RPI::Cullable::LodConfiguration meshLodConfig)
        {
            m_cullable.m_lodData.m_lodConfiguration = meshLodConfig;
        }

        RPI::Cullable::LodConfiguration ModelDataInstance::GetMeshLodConfiguration() const
        {
            return m_cullable.m_lodData.m_lodConfiguration;
        }

        void ModelDataInstance::UpdateDrawPackets(bool forceUpdate /*= false*/)
        {
            AZ_Assert(!r_meshInstancingEnabled, "If mesh instancing is enabled, the draw packet update should be going through the MeshInstanceManager.");

            // Only enable draw motion if model is dynamic and draw motion was disabled
            bool enableDrawMotion = !m_flags.m_isDrawMotion && m_flags.m_dynamic;
            RHI::DrawListTag meshMotionDrawListTag;
            if (enableDrawMotion)
            {
                meshMotionDrawListTag = AZ::RHI::RHISystemInterface::Get()->GetDrawListTagRegistry()->FindTag(MeshCommon::MotionDrawListTagName);
            }

            for (auto& drawPacketList : m_drawPacketListsByLod)
            {
                for (auto& drawPacket : drawPacketList)
                {
                    if (enableDrawMotion)
                    {
                        drawPacket.SetEnableDraw(meshMotionDrawListTag, true);
                    }
                    if (drawPacket.Update(*m_scene, forceUpdate))
                    {
                        m_flags.m_cullableNeedsRebuild = true;
                    }
                }
            }
        }

        void ModelDataInstance::BuildCullable(MeshFeatureProcessor* meshFeatureProcessor)
        {
            AZ_Assert(m_flags.m_cullableNeedsRebuild, "This function only needs to be called if the cullable to be rebuilt");
            AZ_Assert(m_model, "The model has not finished loading yet");

            RPI::Cullable::CullData& cullData = m_cullable.m_cullData;
            RPI::Cullable::LodData& lodData = m_cullable.m_lodData;
            MeshInstanceManager& meshInstanceManager = meshFeatureProcessor->GetMeshInstanceManager();

            const Aabb& localAabb = m_aabb;
            lodData.m_lodSelectionRadius = 0.5f*localAabb.GetExtents().GetMaxElement();

            const size_t modelLodCount = m_model->GetLodCount();
            const auto& lodAssets = m_model->GetModelAsset()->GetLodAssets();
            AZ_Assert(lodAssets.size() == modelLodCount, "Number of asset lods must match number of model lods");
            AZ_Assert(m_lodBias <= modelLodCount - 1, "Incorrect lod bias");

            lodData.m_lods.resize(modelLodCount);
            cullData.m_drawListMask.reset();

            const size_t lodCount = lodAssets.size();

            for (size_t lodIndex = 0; lodIndex < lodCount; ++lodIndex)
            {
                //initialize the lod
                RPI::Cullable::LodData::Lod& lod = lodData.m_lods[lodIndex];
                // non-used lod (except if forced)
                if (lodIndex < m_lodBias)
                {
                    // set impossible screen coverage to disable it
                    lod.m_screenCoverageMax = 0.0f;
                    lod.m_screenCoverageMin = 1.0f;
                }
                else
                {
                    if (lodIndex == m_lodBias)
                    {
                        //first lod
                        lod.m_screenCoverageMax = 1.0f;
                    }
                    else
                    {
                        //every other lod: use the previous lod's min
                        lod.m_screenCoverageMax = AZStd::GetMax(lodData.m_lods[lodIndex - 1].m_screenCoverageMin, lodData.m_lodConfiguration.m_minimumScreenCoverage);
                    }

                    if (lodIndex < lodAssets.size() - 1)
                    {
                        //first and middle lods: compute a stepdown value for the min
                        lod.m_screenCoverageMin = AZStd::GetMax(lodData.m_lodConfiguration.m_qualityDecayRate * lod.m_screenCoverageMax, lodData.m_lodConfiguration.m_minimumScreenCoverage);
                    }
                    else
                    {
                        //last lod: use MinimumScreenCoverage for the min
                        lod.m_screenCoverageMin = lodData.m_lodConfiguration.m_minimumScreenCoverage;
                    }
                }

                lod.m_drawPackets.clear();
                size_t meshCount = lodAssets[lodIndex + m_lodBias]->GetMeshes().size();
                for (size_t meshIndex = 0; meshIndex < meshCount; ++meshIndex)
                {
                    const RHI::DrawPacket* rhiDrawPacket = nullptr;
                    if (!r_meshInstancingEnabled)
                    {
                        // If mesh instancing is disabled, get the draw packets directly from this ModelDataInstance
                        rhiDrawPacket = m_drawPacketListsByLod[lodIndex + m_lodBias][meshIndex].GetRHIDrawPacket();
                    }
                    else
                    {
                        // If mesh instancing is enabled, get the draw packets from the mesh instance manager
                        InstanceGroupHandle& instanceGroupHandle = m_instanceGroupHandlesByLod[lodIndex + m_lodBias][meshIndex];
                        rhiDrawPacket = meshInstanceManager[instanceGroupHandle].m_drawPacket.GetRHIDrawPacket();
                    }

                    if (rhiDrawPacket)
                    {
                        //OR-together all the drawListMasks (so we know which views to cull against)
                        cullData.m_drawListMask |= rhiDrawPacket->GetDrawListMask();

                        lod.m_drawPackets.push_back(rhiDrawPacket);
                    }
                }
            }

            cullData.m_hideFlags = RPI::View::UsageNone;
            if (m_descriptor.m_excludeFromReflectionCubeMaps)
            {
                cullData.m_hideFlags |= RPI::View::UsageReflectiveCubeMap;
            }

#ifdef AZ_CULL_DEBUG_ENABLED
            m_cullable.SetDebugName(AZ::Name(AZStd::string::format("%s - objectId: %u", m_model->GetModelAsset()->GetName().GetCStr(), m_objectId.GetIndex())));
#endif

            m_flags.m_cullableNeedsRebuild = false;
            m_flags.m_cullBoundsNeedsUpdate = true;
        }

        void ModelDataInstance::UpdateCullBounds(const MeshFeatureProcessor* meshFeatureProcessor)
        {
            AZ_Assert(m_flags.m_cullBoundsNeedsUpdate, "This function only needs to be called if the culling bounds need to be rebuilt");
            AZ_Assert(m_model, "The model has not finished loading yet");
            const TransformServiceFeatureProcessor* transformService = meshFeatureProcessor->GetTransformServiceFeatureProcessor();
            Transform localToWorld = transformService->GetTransformForId(m_objectId);
            Vector3 nonUniformScale = transformService->GetNonUniformScaleForId(m_objectId);

            Vector3 center;
            float radius;
            Aabb localAabb = m_aabb;
            localAabb.MultiplyByScale(nonUniformScale);

            localAabb.GetTransformedAabb(localToWorld).GetAsSphere(center, radius);

            m_cullable.m_lodData.m_lodSelectionRadius = 0.5f*localAabb.GetExtents().GetMaxElement();

            m_cullable.m_cullData.m_boundingSphere = Sphere(center, radius);
            m_cullable.m_cullData.m_boundingObb = localAabb.GetTransformedObb(localToWorld);
            m_cullable.m_cullData.m_visibilityEntry.m_boundingVolume = localAabb.GetTransformedAabb(localToWorld);
            m_cullable.m_cullData.m_visibilityEntry.m_userData = &m_cullable;
            m_cullable.m_cullData.m_visibilityEntry.m_typeFlags = AzFramework::VisibilityEntry::TYPE_RPI_Cullable;
            m_scene->GetCullingScene()->RegisterOrUpdateCullable(m_cullable);

            m_flags.m_cullBoundsNeedsUpdate = false;
        }

        void ModelDataInstance::UpdateObjectSrg(MeshFeatureProcessor* meshFeatureProcessor)
        {
            ReflectionProbeFeatureProcessor* reflectionProbeFeatureProcessor = meshFeatureProcessor->GetReflectionProbeFeatureProcessor();
            TransformServiceFeatureProcessor* transformServiceFeatureProcessor = meshFeatureProcessor->GetTransformServiceFeatureProcessor();
            for (auto& objectSrg : m_objectSrgList)
            {
                if (reflectionProbeFeatureProcessor && (m_descriptor.m_useForwardPassIblSpecular || m_flags.m_hasForwardPassIblSpecularMaterial))
                {
                    // retrieve probe constant indices
                    AZ::RHI::ShaderInputConstantIndex modelToWorldConstantIndex = objectSrg->FindShaderInputConstantIndex(Name("m_reflectionProbeData.m_modelToWorld"));
                    AZ_Error("ModelDataInstance", modelToWorldConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

                    AZ::RHI::ShaderInputConstantIndex modelToWorldInverseConstantIndex = objectSrg->FindShaderInputConstantIndex(Name("m_reflectionProbeData.m_modelToWorldInverse"));
                    AZ_Error("ModelDataInstance", modelToWorldInverseConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

                    AZ::RHI::ShaderInputConstantIndex outerObbHalfLengthsConstantIndex = objectSrg->FindShaderInputConstantIndex(Name("m_reflectionProbeData.m_outerObbHalfLengths"));
                    AZ_Error("ModelDataInstance", outerObbHalfLengthsConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

                    AZ::RHI::ShaderInputConstantIndex innerObbHalfLengthsConstantIndex = objectSrg->FindShaderInputConstantIndex(Name("m_reflectionProbeData.m_innerObbHalfLengths"));
                    AZ_Error("ModelDataInstance", innerObbHalfLengthsConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

                    AZ::RHI::ShaderInputConstantIndex useReflectionProbeConstantIndex = objectSrg->FindShaderInputConstantIndex(Name("m_reflectionProbeData.m_useReflectionProbe"));
                    AZ_Error("ModelDataInstance", useReflectionProbeConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

                    AZ::RHI::ShaderInputConstantIndex useParallaxCorrectionConstantIndex = objectSrg->FindShaderInputConstantIndex(Name("m_reflectionProbeData.m_useParallaxCorrection"));
                    AZ_Error("ModelDataInstance", useParallaxCorrectionConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

                    AZ::RHI::ShaderInputConstantIndex exposureConstantIndex = objectSrg->FindShaderInputConstantIndex(Name("m_reflectionProbeData.m_exposure"));
                    AZ_Error("ModelDataInstance", exposureConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

                    // retrieve probe cubemap index
                    Name reflectionCubeMapImageName = Name("m_reflectionProbeCubeMap");
                    RHI::ShaderInputImageIndex reflectionCubeMapImageIndex = objectSrg->FindShaderInputImageIndex(reflectionCubeMapImageName);
                    AZ_Error("ModelDataInstance", reflectionCubeMapImageIndex.IsValid(), "Failed to find shader image index [%s]", reflectionCubeMapImageName.GetCStr());

                    // retrieve the list of probes that overlap the mesh bounds
                    Transform transform = transformServiceFeatureProcessor->GetTransformForId(m_objectId);

                    Aabb aabbWS = m_aabb;
                    aabbWS.ApplyTransform(transform);

                    ReflectionProbeHandleVector reflectionProbeHandles;
                    reflectionProbeFeatureProcessor->FindReflectionProbes(aabbWS, reflectionProbeHandles);

                    if (!reflectionProbeHandles.empty())
                    {
                        // take the last handle from the list, which will be the smallest (most influential) probe
                        ReflectionProbeHandle handle = reflectionProbeHandles.back();

                        objectSrg->SetConstant(modelToWorldConstantIndex, Matrix3x4::CreateFromTransform(reflectionProbeFeatureProcessor->GetTransform(handle)));
                        objectSrg->SetConstant(modelToWorldInverseConstantIndex, Matrix3x4::CreateFromTransform(reflectionProbeFeatureProcessor->GetTransform(handle)).GetInverseFull());
                        objectSrg->SetConstant(outerObbHalfLengthsConstantIndex, reflectionProbeFeatureProcessor->GetOuterObbWs(handle).GetHalfLengths());
                        objectSrg->SetConstant(innerObbHalfLengthsConstantIndex, reflectionProbeFeatureProcessor->GetInnerObbWs(handle).GetHalfLengths());
                        objectSrg->SetConstant(useReflectionProbeConstantIndex, true);
                        objectSrg->SetConstant(useParallaxCorrectionConstantIndex, reflectionProbeFeatureProcessor->GetUseParallaxCorrection(handle));
                        objectSrg->SetConstant(exposureConstantIndex, reflectionProbeFeatureProcessor->GetRenderExposure(handle));

                        objectSrg->SetImage(reflectionCubeMapImageIndex, reflectionProbeFeatureProcessor->GetCubeMap(handle));
                    }
                    else
                    {
                        objectSrg->SetConstant(useReflectionProbeConstantIndex, false);
                    }
                }

                objectSrg->Compile();
            }

            // Set m_objectSrgNeedsUpdate to false if there are object SRGs in the list
            m_flags.m_objectSrgNeedsUpdate = m_flags.m_objectSrgNeedsUpdate && (m_objectSrgList.size() == 0);
        }

        bool ModelDataInstance::MaterialRequiresForwardPassIblSpecular(Data::Instance<RPI::Material> material) const
        {
            bool requiresForwardPassIbl = false;

            // look for a shader that has the o_materialUseForwardPassIBLSpecular option set
            // Note: this should be changed to have the material automatically set the forwardPassIBLSpecular
            // property and look for that instead of the shader option.
            // [GFX TODO][ATOM-5040] Address Property Metadata Feedback Loop
            material->ForAllShaderItems(
                [&](const Name&, const RPI::ShaderCollection::Item& shaderItem)
                {
                    if (shaderItem.IsEnabled())
                    {
                        RPI::ShaderOptionIndex index = shaderItem.GetShaderOptionGroup().GetShaderOptionLayout()->FindShaderOptionIndex(Name{"o_materialUseForwardPassIBLSpecular"});
                        if (index.IsValid())
                        {
                            RPI::ShaderOptionValue value = shaderItem.GetShaderOptionGroup().GetValue(Name{"o_materialUseForwardPassIBLSpecular"});
                            if (value.GetIndex() == 1)
                            {
                                requiresForwardPassIbl = true;
                                return false; // break
                            }
                        }
                    }

                    return true; // continue
                });

            return requiresForwardPassIbl;
        }

        void ModelDataInstance::SetVisible(bool isVisible)
        {
            m_flags.m_visible = isVisible;
            m_cullable.m_isHidden = !isVisible;
        }

        CustomMaterialInfo ModelDataInstance::GetCustomMaterialWithFallback(const CustomMaterialId& id) const
        {
            const CustomMaterialId ignoreLodId(DefaultCustomMaterialLodIndex, id.second);
            for (const auto& currentId : { id, ignoreLodId, DefaultCustomMaterialId })
            {
                if (auto itr = m_customMaterials.find(currentId); itr != m_customMaterials.end() && itr->second.m_material)
                {
                    return itr->second;
                }
            }
            return CustomMaterialInfo{};
        }

        void ModelDataInstance::HandleDrawPacketUpdate()
        {
            // When the drawpacket is updated, the cullable must be rebuilt to use the latest draw packet
            m_flags.m_cullableNeedsRebuild = true;
        }

    } // namespace Render
} // namespace AZ
