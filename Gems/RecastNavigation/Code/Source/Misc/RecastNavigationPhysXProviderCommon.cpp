/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Misc/RecastNavigationPhysXProviderCommon.h>
#include <AzCore/Debug/Profiler.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Shape.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <DebugDraw/DebugDrawBus.h>

AZ_CVAR(
    bool, cl_navmesh_showInputData, false, nullptr, AZ::ConsoleFunctorFlags::Null,
    "If enabled, draws triangle mesh input data that was used for the navigation mesh calculation");
AZ_CVAR(
    float, cl_navmesh_showInputDataSeconds, 30.f, nullptr, AZ::ConsoleFunctorFlags::Null,
    "If enabled, keeps the debug triangle mesh input for the specified number of seconds");

AZ_DECLARE_BUDGET(Navigation);

namespace RecastNavigation
{
    RecastNavigationPhysXProviderCommon::RecastNavigationPhysXProviderCommon(bool useEditorScene) : m_useEditorScene(useEditorScene)
    {
    }

    const char* RecastNavigationPhysXProviderCommon::GetSceneName() const
    {
        return m_useEditorScene ? AzPhysics::EditorPhysicsSceneName : AzPhysics::DefaultPhysicsSceneName;
    }

    void RecastNavigationPhysXProviderCommon::CollectCollidersWithinVolume(const AZ::Aabb& volume, QueryHits& overlapHits)
    {
        AZ_PROFILE_SCOPE(Navigation, "Navigation: CollectGeometryWithinVolume");

        AZ::Vector3 dimension = volume.GetExtents();
        AZ::Transform pose = AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), volume.GetCenter());

        Physics::BoxShapeConfiguration shapeConfiguration;
        shapeConfiguration.m_dimensions = dimension;

        AzPhysics::OverlapRequest request = AzPhysics::OverlapRequestHelpers::CreateBoxOverlapRequest(dimension, pose, nullptr);
        request.m_queryType = AzPhysics::SceneQuery::QueryType::Static; // only looking for static PhysX colliders
        request.m_collisionGroup = AzPhysics::CollisionGroup::All;

        AzPhysics::SceneQuery::UnboundedOverlapHitCallback unboundedOverlapHitCallback =
            [&overlapHits](AZStd::optional<AzPhysics::SceneQueryHit>&& hit)
        {
            if (hit && ((hit->m_resultFlags & AzPhysics::SceneQuery::EntityId) != 0))
            {
                const AzPhysics::SceneQueryHit& sceneQueryHit = *hit;
                overlapHits.push_back(sceneQueryHit);
            }

            return true;
        };

        //! We need to use unbounded callback, otherwise the results will be limited to 32 or so objects.
        request.m_unboundedOverlapHitCallback = unboundedOverlapHitCallback;

        if (auto sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get())
        {
            AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(GetSceneName());

            // Note: blocking call
            sceneInterface->QueryScene(sceneHandle, &request);
            // results are in overlapHits
        }
    }

    static void AddDebugVertexIfEnabled(AZStd::vector<AZ::Vector3>& transformed, const AZ::Vector3& translated, bool debugDrawInputData)
    {
        if (cl_navmesh_showInputData || debugDrawInputData)
        {
            transformed.push_back(translated);
        }        
    }

    static void AddDebugDrawIfEnabled(AZStd::vector<AZ::Vector3>& transformed, const AZStd::vector<AZ::u32>& indices, bool debugDrawInputData)
    {
        if (cl_navmesh_showInputData || debugDrawInputData)
        {
            for (size_t i = 2; i < indices.size(); i += 3)
            {
                DebugDraw::DebugDrawRequestBus::Broadcast(&DebugDraw::DebugDrawRequests::DrawLineLocationToLocation,
                    transformed[indices[i - 2]], transformed[indices[i - 1]], AZ::Colors::Red, cl_navmesh_showInputDataSeconds);
                DebugDraw::DebugDrawRequestBus::Broadcast(&DebugDraw::DebugDrawRequests::DrawLineLocationToLocation,
                    transformed[indices[i - 1]], transformed[indices[i - 0]], AZ::Colors::Red, cl_navmesh_showInputDataSeconds);
                DebugDraw::DebugDrawRequestBus::Broadcast(&DebugDraw::DebugDrawRequests::DrawLineLocationToLocation,
                    transformed[indices[i - 0]], transformed[indices[i - 2]], AZ::Colors::Red, cl_navmesh_showInputDataSeconds);
            }
        }
    }

    void RecastNavigationPhysXProviderCommon::AppendColliderGeometry(
        TileGeometry& geometry,
        const QueryHits& overlapHits,
        bool debugDrawInputData)
    {
        AZ_PROFILE_SCOPE(Navigation, "Navigation: AppendColliderGeometry");

        AZStd::vector<AZ::Vector3> vertices;
        AZStd::vector<AZ::u32> indices;
        AZStd::size_t indicesCount = geometry.m_indices.size();

        AzPhysics::SceneInterface* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(GetSceneName());

        for (const auto& overlapHit : overlapHits)
        {
            AzPhysics::SimulatedBody* body = sceneInterface->GetSimulatedBodyFromHandle(sceneHandle, overlapHit.m_bodyHandle);
            if (!body)
            {
                continue;
            }

            AZ::Transform t = AZ::Transform::CreateFromQuaternionAndTranslation(body->GetOrientation(), body->GetPosition());
            overlapHit.m_shape->GetGeometry(vertices, indices, nullptr);
            // Note: geometry data is in local space

            if (!vertices.empty())
            {
                if (indices.empty())
                {
                    // Some PhysX colliders (convex shapes) return geometry without indices. Build indices now.
                    AZStd::vector<AZ::Vector3> transformed;

                    int currentLocalIndex = 0;
                    for (const AZ::Vector3& vertex : vertices)
                    {
                        const AZ::Vector3 translated = t.TransformPoint(vertex);
                        geometry.m_vertices.push_back(RecastVector3::CreateFromVector3SwapYZ(translated));

                        AddDebugVertexIfEnabled(transformed, translated, debugDrawInputData);

                        geometry.m_indices.push_back(aznumeric_cast<AZ::u32>(indicesCount + currentLocalIndex));
                        currentLocalIndex++;
                    }

                    AddDebugDrawIfEnabled(transformed, indices, debugDrawInputData);
                }
                else
                {
                    AZStd::vector<AZ::Vector3> transformed;

                    for (const AZ::Vector3& vertex : vertices)
                    {
                        const AZ::Vector3 translated = t.TransformPoint(vertex);
                        geometry.m_vertices.push_back(RecastVector3::CreateFromVector3SwapYZ(translated));

                        AddDebugVertexIfEnabled(transformed, translated, debugDrawInputData);
                    }

                    for (size_t i = 2; i < indices.size(); i += 3)
                    {
                        geometry.m_indices.push_back(aznumeric_cast<AZ::u32>(indicesCount + indices[i]));
                        geometry.m_indices.push_back(aznumeric_cast<AZ::u32>(indicesCount + indices[i - 1]));
                        geometry.m_indices.push_back(aznumeric_cast<AZ::u32>(indicesCount + indices[i - 2]));
                    }

                    AddDebugDrawIfEnabled(transformed, indices, debugDrawInputData);
                }

                indicesCount += vertices.size();
                vertices.clear();
                indices.clear();
            }
        }
    }

    AZStd::vector<AZStd::shared_ptr<TileGeometry>> RecastNavigationPhysXProviderCommon::CollectGeometryImpl(
        float tileSize, float borderSize, const AZ::Aabb& worldVolume, bool debugDrawInputData)
    {
        AZ_PROFILE_SCOPE(Navigation, "Navigation: CollectGeometry");

        AZStd::vector<AZStd::shared_ptr<TileGeometry>> tiles;

        const AZ::Vector3 extents = worldVolume.GetExtents();
        int tilesAlongX = static_cast<int>(AZStd::ceilf(extents.GetX() / tileSize));
        int tilesAlongY = static_cast<int>(AZStd::ceilf(extents.GetY() / tileSize));

        const AZ::Vector3& worldMin = worldVolume.GetMin();
        const AZ::Vector3& worldMax = worldVolume.GetMax();

        const AZ::Vector3 border = AZ::Vector3::CreateOne() * borderSize;

        // Find all geometry one tile at a time.
        for (int y = 0; y < tilesAlongY; ++y)
        {
            for (int x = 0; x < tilesAlongX; ++x)
            {
                const AZ::Vector3 tileMin{
                    worldMin.GetX() + aznumeric_cast<float>(x) * tileSize,
                    worldMin.GetY() + aznumeric_cast<float>(y) * tileSize,
                    worldMin.GetZ()
                };

                const AZ::Vector3 tileMax{
                    worldMin.GetX() + aznumeric_cast<float>(x + 1) * tileSize,
                    worldMin.GetY() + aznumeric_cast<float>(y + 1) * tileSize,
                    worldMax.GetZ()
                };

                // Recast wants extra triangle data around each tile, so that each tile can connect to each other.
                AZ::Aabb tileVolume = AZ::Aabb::CreateFromMinMax(tileMin, tileMax);
                AZ::Aabb scanVolume = AZ::Aabb::CreateFromMinMax(tileMin - border, tileMax + border);

                QueryHits results;
                CollectCollidersWithinVolume(scanVolume, results);

                AZStd::shared_ptr<TileGeometry> geometryData = AZStd::make_unique<TileGeometry>();
                geometryData->m_worldBounds = tileVolume;
                AppendColliderGeometry(*geometryData, results, debugDrawInputData);

                geometryData->m_tileX = x;
                geometryData->m_tileY = y;
                tiles.push_back(geometryData);
            }
        }

        return tiles;
    }
} // namespace RecastNavigation
