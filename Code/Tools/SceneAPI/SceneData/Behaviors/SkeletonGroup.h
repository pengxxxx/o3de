/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <SceneAPI/SceneCore/Components/BehaviorComponent.h>
#include <SceneAPI/SceneCore/Events/ManifestMetaInfoBus.h>
#include <SceneAPI/SceneCore/Events/AssetImportRequest.h>

namespace AZ
{
    namespace SceneAPI
    {
        namespace Behaviors
        {
            class SkeletonGroup 
                : public SceneCore::BehaviorComponent
                , public Events::ManifestMetaInfoBus::Handler
                , public Events::AssetImportRequestBus::Handler
            {
            public:
                AZ_COMPONENT(SkeletonGroup, "{9243A4BA-46BD-4961-950F-DEFAE9A919E5}", SceneCore::BehaviorComponent);

                ~SkeletonGroup() override = default;

                void Activate() override;
                void Deactivate() override;
                static void Reflect(ReflectContext* context);

                void GetCategoryAssignments(CategoryRegistrationList& categories, const Containers::Scene& scene) override;
                void InitializeObject(const Containers::Scene& scene, DataTypes::IManifestObject& target) override;
                Events::ProcessingResult UpdateManifest(Containers::Scene& scene, ManifestAction action,
                    RequestingApplication requester) override;
                void GetPolicyName(AZStd::string& result) const override
                {
                    result = "SkeletonGroup";
                }

            private:
                Events::ProcessingResult BuildDefault(Containers::Scene& scene);
                Events::ProcessingResult UpdateSkeletonGroups(Containers::Scene& scene) const;

                bool SceneHasSkeletonGroup(const Containers::Scene& scene) const;
                
                static const int s_rigsPreferredTabOrder;
                bool m_isDefaultConstructing{ false };
            };
        } // namespace Behaviors
    } // namespace SceneAPI
} // namespace AZ
