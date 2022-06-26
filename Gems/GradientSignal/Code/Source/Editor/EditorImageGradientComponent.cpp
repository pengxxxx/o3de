/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "EditorImageGradientComponent.h"

namespace GradientSignal
{
    void EditorImageGradientComponent::Reflect(AZ::ReflectContext* context)
    {
        EditorGradientComponentBase::ReflectSubClass<EditorImageGradientComponent, BaseClassType>(context);
    }

    void EditorImageGradientComponent::OnCompositionChanged()
    {
        m_component.WriteOutConfig(&m_configuration);
        SetDirty();

        AzToolsFramework::ToolsApplicationEvents::Bus::Broadcast(
            &AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay, AzToolsFramework::Refresh_AttributesAndValues);
    }
}
