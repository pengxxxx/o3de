/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#pragma once

#include <AzCore/Component/EntityId.h>
#include <AzCore/Math/Vector3.h>
#include <AzFramework/Viewport/CameraState.h>
#include <AzToolsFramework/Viewport/ViewportTypes.h>
#include <AzToolsFramework/ViewportSelection/EditorTransformComponentSelectionRequestBus.h>

namespace AZ
{
    class Aabb;
}

namespace AzFramework
{
    struct CameraState;
}

namespace AzToolsFramework
{
    /// Is the pivot at the center of the object (middle of extents) or at the
    /// exported authored object root position.
    inline bool Centered(const EditorTransformComponentSelectionRequests::Pivot pivot)
    {
        return pivot == EditorTransformComponentSelectionRequests::Pivot::Center;
    }

    /// Return offset from object pivot to center if center is true, otherwise Vector3::Zero.
    AZ::Vector3 CalculateCenterOffset(AZ::EntityId entityId, EditorTransformComponentSelectionRequests::Pivot pivot);

    /// Calculate scale factor based on distance from camera
    float CalculateScreenToWorldMultiplier(
        const AZ::Vector3& worldPosition, const AzFramework::CameraState& cameraState);

    /// Map from world space to screen space.
     AzFramework::ScreenPoint GetScreenPosition(int viewportId, const AZ::Vector3& worldTranslation);

    /// Given a mouse interaction, determine if the pick ray from its position
    /// in screen space intersected an aabb in world space.
    bool AabbIntersectMouseRay(
        const ViewportInteraction::MouseInteraction& mouseInteraction, const AZ::Aabb& aabb);

    /// Return if a mouse interaction (pick ray) did intersect the tested EntityId.
    bool PickEntity(
        AZ::EntityId entityId, const ViewportInteraction::MouseInteraction& mouseInteraction,
        float& closestDistance, int viewportId);

    /// Wrapper for EBus call to return the CameraState for a given viewport.
    AzFramework::CameraState GetCameraState(int viewportId);

    /// Wrapper for EBus call to return the DPI scaling for a given viewport.
    float GetScreenDisplayScaling(const int viewportId);

    /// A utility to return the center of several points.
    /// Take several positions and store the min and max of each in
    /// turn - when all points have been added return the center/midpoint.
    class MidpointCalculator
    {
    public:
        /// Default constructed with min and max initialized to opposites.
        MidpointCalculator() = default;

        /// Call this for all positions you want to be considered.
        void AddPosition(const AZ::Vector3& position)
        {
            m_minPosition = position.GetMin(m_minPosition);
            m_maxPosition = position.GetMax(m_maxPosition);
        }

        /// Once all positions have been added, call this to return the midpoint.
        AZ::Vector3 CalculateMidpoint() const
        {
            return m_minPosition + (m_maxPosition - m_minPosition) * 0.5f;
        }

    private:
        AZ::Vector3 m_minPosition = AZ::Vector3(std::numeric_limits<float>::max());
        AZ::Vector3 m_maxPosition = AZ::Vector3(-std::numeric_limits<float>::max());
    };
} // namespace AzToolsFramework
