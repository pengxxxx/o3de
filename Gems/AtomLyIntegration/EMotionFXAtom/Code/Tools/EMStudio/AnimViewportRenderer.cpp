/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzFramework/Components/TransformComponent.h>

#include <Integration/ActorComponentBus.h>
#include <Integration/Components/ActorComponent.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>
#include <Atom/Feature/CoreLights/DirectionalLightFeatureProcessorInterface.h>
#include <Atom/Feature/DisplayMapper/DisplayMapperFeatureProcessorInterface.h>
#include <Atom/Feature/PostProcess/PostProcessFeatureProcessorInterface.h>
#include <Atom/Feature/ImageBasedLights/ImageBasedLightFeatureProcessorInterface.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/Component/DebugCamera/CameraComponent.h>
#include <Atom/Component/DebugCamera/NoClipControllerComponent.h>

#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentConstants.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AtomLyIntegration/CommonFeatures/Grid/GridComponentConstants.h>
#include <AtomLyIntegration/CommonFeatures/Grid/GridComponentConfig.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentConstants.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>
#include <AtomLyIntegration/CommonFeatures/PostProcess/PostFxLayerComponentConstants.h>
#include <AtomLyIntegration/CommonFeatures/PostProcess/ExposureControl/ExposureControlComponentConstants.h>
#include <AtomLyIntegration/CommonFeatures/ImageBasedLights/ImageBasedLightComponentConstants.h>

#include <EMotionFX/Source/EMotionFXManager.h>
#include <EMotionFX/Source/ActorManager.h>
#include <EMStudio/AnimViewportRenderer.h>


namespace EMStudio
{
    static constexpr float DepthNear = 0.01f;

    AnimViewportRenderer::AnimViewportRenderer(AZStd::shared_ptr<AZ::RPI::WindowContext> windowContext)
        : m_windowContext(windowContext)
    {
        // Create a new entity context
        m_entityContext = AZStd::make_unique<AzFramework::EntityContext>();
        m_entityContext->InitContext();

        // Create the scene
        auto sceneSystem = AzFramework::SceneSystemInterface::Get();
        AZ_Assert(sceneSystem, "Unable to retrieve scene system.");
        AZ::Outcome<AZStd::shared_ptr<AzFramework::Scene>, AZStd::string> createSceneOutcome = sceneSystem->CreateScene("AnimViewport");
        AZ_Assert(createSceneOutcome, "%s", createSceneOutcome.GetError().data());
        m_frameworkScene = createSceneOutcome.TakeValue();
        m_frameworkScene->SetSubsystem<AzFramework::EntityContext::SceneStorageType>(m_entityContext.get());

        // Create and register a scene with all available feature processors
        // TODO: We don't need every procesors, only register the processor we are going to use.
        AZ::RPI::SceneDescriptor sceneDesc;
        m_scene = AZ::RPI::Scene::CreateScene(sceneDesc);
        m_scene->EnableAllFeatureProcessors();

        // Link our RPI::Scene to the AzFramework::Scene
        m_frameworkScene->SetSubsystem(m_scene);

        // Create a render pipeline from the specified asset for the window context and add the pipeline to the scene
        AZStd::string defaultPipelineAssetPath = "passes/MainRenderPipeline.azasset";
        AZ::Data::Asset<AZ::RPI::AnyAsset> pipelineAsset = AZ::RPI::AssetUtils::LoadAssetByProductPath<AZ::RPI::AnyAsset>(
            defaultPipelineAssetPath.c_str(), AZ::RPI::AssetUtils::TraceLevel::Error);
        m_renderPipeline = AZ::RPI::RenderPipeline::CreateRenderPipelineForWindow(pipelineAsset, *m_windowContext.get());
        pipelineAsset.Release();
        m_scene->AddRenderPipeline(m_renderPipeline);

        // Currently the scene has to be activated after render pipeline was added so some feature processors (i.e. imgui) can be
        // initialized properly with pipeline's pass information.
        m_scene->Activate();
        AZ::RPI::RPISystemInterface::Get()->RegisterScene(m_scene);

        AzFramework::EntityContextId entityContextId = m_entityContext->GetContextId();

        // Configure camera
        AzFramework::EntityContextRequestBus::EventResult(
            m_cameraEntity, entityContextId, &AzFramework::EntityContextRequestBus::Events::CreateEntity, "Cameraentity");
        AZ_Assert(m_cameraEntity != nullptr, "Failed to create camera entity.");

        // Add debug camera and controller components
        AZ::Debug::CameraComponentConfig cameraConfig(m_windowContext);
        cameraConfig.m_fovY = AZ::Constants::HalfPi;
        cameraConfig.m_depthNear = DepthNear;
        m_cameraComponent = m_cameraEntity->CreateComponent(azrtti_typeid<AZ::Debug::CameraComponent>());
        m_cameraComponent->SetConfiguration(cameraConfig);
        m_cameraEntity->CreateComponent(azrtti_typeid<AzFramework::TransformComponent>());
        m_cameraEntity->CreateComponent(azrtti_typeid<AZ::Debug::NoClipControllerComponent>());
        m_cameraEntity->Init();
        m_cameraEntity->Activate();

        // Connect camera to pipeline's default view after camera entity activated
        m_renderPipeline->SetDefaultViewFromEntity(m_cameraEntity->GetId());

        // Get the FeatureProcessors
        m_meshFeatureProcessor = m_scene->GetFeatureProcessor<AZ::Render::MeshFeatureProcessorInterface>();

        // Configure tone mapper
        AzFramework::EntityContextRequestBus::EventResult(
            m_postProcessEntity, entityContextId, &AzFramework::EntityContextRequestBus::Events::CreateEntity, "postProcessEntity");
        AZ_Assert(m_postProcessEntity != nullptr, "Failed to create post process entity.");

        m_postProcessEntity->CreateComponent(AZ::Render::PostFxLayerComponentTypeId);
        m_postProcessEntity->CreateComponent(AZ::Render::ExposureControlComponentTypeId);
        m_postProcessEntity->CreateComponent(azrtti_typeid<AzFramework::TransformComponent>());
        m_postProcessEntity->Activate();

        // Init directional light processor
        m_directionalLightFeatureProcessor = m_scene->GetFeatureProcessor<AZ::Render::DirectionalLightFeatureProcessorInterface>();

        // Init display mapper processor
        m_displayMapperFeatureProcessor = m_scene->GetFeatureProcessor<AZ::Render::DisplayMapperFeatureProcessorInterface>();

        // Init Skybox
        m_skyboxFeatureProcessor = m_scene->GetFeatureProcessor<AZ::Render::SkyBoxFeatureProcessorInterface>();
        m_skyboxFeatureProcessor->Enable(true);
        m_skyboxFeatureProcessor->SetSkyboxMode(AZ::Render::SkyBoxMode::Cubemap);

        // Create IBL
        AzFramework::EntityContextRequestBus::EventResult(
            m_iblEntity, entityContextId, &AzFramework::EntityContextRequestBus::Events::CreateEntity, "IblEntity");
        AZ_Assert(m_iblEntity != nullptr, "Failed to create ibl entity.");

        m_iblEntity->CreateComponent(AZ::Render::ImageBasedLightComponentTypeId);
        m_iblEntity->CreateComponent(azrtti_typeid<AzFramework::TransformComponent>());
        m_iblEntity->Activate();

        // Load light preset
        AZ::Data::Asset<AZ::RPI::AnyAsset> lightingPresetAsset = AZ::RPI::AssetUtils::LoadAssetByProductPath<AZ::RPI::AnyAsset>(
            "lightingpresets/default.lightingpreset.azasset", AZ::RPI::AssetUtils::TraceLevel::Warning);
        const AZ::Render::LightingPreset* preset = lightingPresetAsset->GetDataAs<AZ::Render::LightingPreset>();
        SetLightingPreset(preset);

        // Create grid
        AzFramework::EntityContextRequestBus::EventResult(
            m_gridEntity, entityContextId, &AzFramework::EntityContextRequestBus::Events::CreateEntity, "ViewportGrid");
        AZ_Assert(m_gridEntity != nullptr, "Failed to create grid entity.");

        AZ::Render::GridComponentConfig gridConfig;
        gridConfig.m_gridSize = 4.0f;
        gridConfig.m_axisColor = AZ::Color(0.5f, 0.5f, 0.5f, 1.0f);
        gridConfig.m_primaryColor = AZ::Color(0.3f, 0.3f, 0.3f, 1.0f);
        gridConfig.m_secondaryColor = AZ::Color(0.5f, 0.1f, 0.1f, 1.0f);
        auto gridComponent = m_gridEntity->CreateComponent(AZ::Render::GridComponentTypeId);
        gridComponent->SetConfiguration(gridConfig);

        m_gridEntity->CreateComponent(azrtti_typeid<AzFramework::TransformComponent>());
        m_gridEntity->Activate();

        Reinit();
    }

    AnimViewportRenderer::~AnimViewportRenderer()
    {
        // Destroy all the entity we created.
        m_entityContext->DestroyEntity(m_iblEntity);
        m_entityContext->DestroyEntity(m_postProcessEntity);
        m_entityContext->DestroyEntity(m_cameraEntity);
        m_entityContext->DestroyEntity(m_gridEntity);
        for (AZ::Entity* entity : m_actorEntities)
        {
            m_entityContext->DestroyEntity(entity);
        }
        m_actorEntities.clear();
        m_entityContext->DestroyContext();

        for (AZ::Render::DirectionalLightFeatureProcessorInterface::LightHandle& handle : m_lightHandles)
        {
            m_directionalLightFeatureProcessor->ReleaseLight(handle);
        }
        m_lightHandles.clear();

        m_frameworkScene->UnsetSubsystem(m_scene);

        auto sceneSystem = AzFramework::SceneSystemInterface::Get();
        AZ_Assert(sceneSystem, "AtomViewportRenderer was unable to get the scene system during destruction.");
        bool removeSuccess = sceneSystem->RemoveScene("AnimViewport");
        AZ_Assert(removeSuccess, "AtomViewportRenderer should be removed.");

        AZ::RPI::RPISystemInterface::Get()->UnregisterScene(m_scene);
        m_scene = nullptr;
    }

    void AnimViewportRenderer::Reinit()
    {
        ReinitActorEntities();
        ResetEnvironment();
    }

    void AnimViewportRenderer::ResetEnvironment()
    {
        // Reset environment
        AZ::Transform iblTransform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::Event(m_iblEntity->GetId(), &AZ::TransformBus::Events::SetLocalTM, iblTransform);

        const AZ::Matrix4x4 rotationMatrix = AZ::Matrix4x4::CreateIdentity();
        AZ::RPI::ScenePtr scene = AZ::RPI::RPISystemInterface::Get()->GetDefaultScene();
        auto skyBoxFeatureProcessorInterface = scene->GetFeatureProcessor<AZ::Render::SkyBoxFeatureProcessorInterface>();
        skyBoxFeatureProcessorInterface->SetCubemapRotationMatrix(rotationMatrix);

        Camera::Configuration cameraConfig;
        Camera::CameraRequestBus::EventResult(
            cameraConfig, m_cameraEntity->GetId(), &Camera::CameraRequestBus::Events::GetCameraConfiguration);

        // Reset the camera position
        static constexpr float StartingDistanceMultiplier = 2.0f;
        static constexpr float StartingRotationAngle = AZ::Constants::QuarterPi / 2.0f;

        AZ::Vector3 targetPosition = iblTransform.GetTranslation();
        const float distance = 1.0f * StartingDistanceMultiplier;
        const AZ::Quaternion cameraRotation = AZ::Quaternion::CreateFromAxisAngle(AZ::Vector3::CreateAxisZ(), StartingRotationAngle);
        AZ::Vector3 cameraPosition(targetPosition.GetX(), targetPosition.GetY() - distance, targetPosition.GetZ());
        cameraPosition = cameraRotation.TransformVector(cameraPosition);
        AZ::Transform cameraTransform = AZ::Transform::CreateFromQuaternionAndTranslation(cameraRotation, cameraPosition);
        AZ::TransformBus::Event(m_cameraEntity->GetId(), &AZ::TransformBus::Events::SetLocalTM, cameraTransform);

        // Setup primary camera controls
        AZ::Debug::CameraControllerRequestBus::Event(
            m_cameraEntity->GetId(), &AZ::Debug::CameraControllerRequestBus::Events::Enable, azrtti_typeid<AZ::Debug::NoClipControllerComponent>());
    }

    void AnimViewportRenderer::ReinitActorEntities()
    {
        // 1. Destroy all the entities that do not point to any actorAsset anymore.
        AZStd::set<AZ::Data::AssetId> assetLookup;
        AzFramework::EntityContext* entityContext = m_entityContext.get();
        const size_t numActors = EMotionFX::GetActorManager().GetNumActors();
        for (size_t i = 0; i < numActors; ++i)
        {
            assetLookup.emplace(EMotionFX::GetActorManager().GetActorAsset(i).GetId());
        }
        m_actorEntities.erase(
            AZStd::remove_if(
                m_actorEntities.begin(), m_actorEntities.end(),
                [&assetLookup, entityContext](AZ::Entity* entity)
                {
                    EMotionFX::Integration::ActorComponent* actorComponent =
                        entity->FindComponent<EMotionFX::Integration::ActorComponent>();
                    if (assetLookup.find(actorComponent->GetActorAsset().GetId()) == assetLookup.end())
                    {
                        entityContext->DestroyEntity(entity);
                        return true;
                    }
                    return false;
                }),
            m_actorEntities.end());

        // 2. Create an entity for every actorAsset stored in actor manager.
        for (size_t i = 0; i < numActors; ++i)
        {
            AZ::Data::Asset<EMotionFX::Integration::ActorAsset> actorAsset = EMotionFX::GetActorManager().GetActorAsset(i);
            if (!actorAsset->IsReady())
            {
                continue;
            }

            AZ::Entity* entity = FindActorEntity(actorAsset);
            if (!entity)
            {
                m_actorEntities.emplace_back(CreateActorEntity(actorAsset));
            }
        }
    }

    AZ::Entity* AnimViewportRenderer::FindActorEntity(AZ::Data::Asset<EMotionFX::Integration::ActorAsset> actorAsset) const
    {
        const auto foundEntity = AZStd::find_if(
            begin(m_actorEntities), end(m_actorEntities),
            [match = actorAsset](const AZ::Entity* entity)
            {
                EMotionFX::Integration::ActorComponent* actorComponent = entity->FindComponent<EMotionFX::Integration::ActorComponent>();
                return actorComponent->GetActorAsset() == match;
            });
        return foundEntity != end(m_actorEntities) ? (*foundEntity) : nullptr;
    }

    AZ::Entity* AnimViewportRenderer::CreateActorEntity(AZ::Data::Asset<EMotionFX::Integration::ActorAsset> actorAsset)
    {
        AZ::Entity* actorEntity = m_entityContext->CreateEntity(actorAsset->GetActor()->GetName());
        actorEntity->CreateComponent(azrtti_typeid<EMotionFX::Integration::ActorComponent>());
        actorEntity->CreateComponent(AZ::Render::MaterialComponentTypeId);
        actorEntity->CreateComponent(azrtti_typeid<AzFramework::TransformComponent>());
        actorEntity->Activate();

        EMotionFX::Integration::ActorComponent* actorComponent = actorEntity->FindComponent<EMotionFX::Integration::ActorComponent>();
        actorComponent->SetActorAsset(actorAsset);

        return actorEntity;
    }

    void AnimViewportRenderer::SetLightingPreset(const AZ::Render::LightingPreset* preset)
    {
        if (!preset)
        {
            AZ_Warning("AnimViewportRenderer", false, "Attempting to set invalid lighting preset.");
            return;
        }

        AZ::Render::ImageBasedLightFeatureProcessorInterface* iblFeatureProcessor =
            m_scene->GetFeatureProcessor<AZ::Render::ImageBasedLightFeatureProcessorInterface>();
        AZ::Render::PostProcessFeatureProcessorInterface* postProcessFeatureProcessor =
            m_scene->GetFeatureProcessor<AZ::Render::PostProcessFeatureProcessorInterface>();
        AZ::Render::ExposureControlSettingsInterface* exposureControlSettingInterface =
            postProcessFeatureProcessor->GetOrCreateSettingsInterface(m_postProcessEntity->GetId())
                ->GetOrCreateExposureControlSettingsInterface();

        Camera::Configuration cameraConfig;
        Camera::CameraRequestBus::EventResult(
            cameraConfig, m_cameraEntity->GetId(), &Camera::CameraRequestBus::Events::GetCameraConfiguration);

        preset->ApplyLightingPreset(
            iblFeatureProcessor, m_skyboxFeatureProcessor, exposureControlSettingInterface, m_directionalLightFeatureProcessor,
            cameraConfig, m_lightHandles, nullptr, AZ::RPI::MaterialPropertyIndex::Null, false);
    }
} // namespace EMStudio
