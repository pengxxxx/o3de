/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "InvertGradientComponent.h"
#include <AzCore/Math/MathUtils.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace GradientSignal
{
    void InvertGradientConfig::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context);
        if (serialize)
        {
            serialize->Class<InvertGradientConfig, AZ::ComponentConfig>()
                ->Version(0)
                ->Field("Gradient", &InvertGradientConfig::m_gradientSampler)
                ;

            AZ::EditContext* edit = serialize->GetEditContext();
            if (edit)
            {
                edit->Class<InvertGradientConfig>(
                    "Invert Gradient", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(0, &InvertGradientConfig::m_gradientSampler, "Gradient", "Input gradient whose values will be inverted.")
                    ;
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<InvertGradientConfig>()
                ->Constructor()
                ->Attribute(AZ::Script::Attributes::Category, "Vegetation")
                ->Property("gradientSampler", BehaviorValueProperty(&InvertGradientConfig::m_gradientSampler))
                ;
        }
    }

    void InvertGradientComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services)
    {
        services.push_back(AZ_CRC("GradientService", 0x21c18d23));
    }

    void InvertGradientComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& services)
    {
        services.push_back(AZ_CRC("GradientService", 0x21c18d23));
    }

    void InvertGradientComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& services)
    {
    }

    void InvertGradientComponent::Reflect(AZ::ReflectContext* context)
    {
        InvertGradientConfig::Reflect(context);

        AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context);
        if (serialize)
        {
            serialize->Class<InvertGradientComponent, AZ::Component>()
                ->Version(0)
                ->Field("Configuration", &InvertGradientComponent::m_configuration)
                ;
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Constant("InvertGradientComponentTypeId", BehaviorConstant(InvertGradientComponentTypeId));

            behaviorContext->Class<InvertGradientComponent>()->RequestBus("InvertGradientRequestBus");

            behaviorContext->EBus<InvertGradientRequestBus>("InvertGradientRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Vegetation")
                ->Event("GetGradientSampler", &InvertGradientRequestBus::Events::GetGradientSampler)
                ;
        }
    }

    InvertGradientComponent::InvertGradientComponent(const InvertGradientConfig& configuration)
        : m_configuration(configuration)
    {
    }

    void InvertGradientComponent::Activate()
    {
        m_dependencyMonitor.Reset();
        m_dependencyMonitor.ConnectOwner(GetEntityId());
        m_dependencyMonitor.ConnectDependency(m_configuration.m_gradientSampler.m_gradientId);
        GradientRequestBus::Handler::BusConnect(GetEntityId());
        InvertGradientRequestBus::Handler::BusConnect(GetEntityId());
    }

    void InvertGradientComponent::Deactivate()
    {
        m_dependencyMonitor.Reset();
        GradientRequestBus::Handler::BusDisconnect();
        InvertGradientRequestBus::Handler::BusDisconnect();
    }

    bool InvertGradientComponent::ReadInConfig(const AZ::ComponentConfig* baseConfig)
    {
        if (auto config = azrtti_cast<const InvertGradientConfig*>(baseConfig))
        {
            m_configuration = *config;
            return true;
        }
        return false;
    }

    bool InvertGradientComponent::WriteOutConfig(AZ::ComponentConfig* outBaseConfig) const
    {
        if (auto config = azrtti_cast<InvertGradientConfig*>(outBaseConfig))
        {
            *config = m_configuration;
            return true;
        }
        return false;
    }

    float InvertGradientComponent::GetValue(const GradientSampleParams& sampleParams) const
    {
        float output = 0.0f;

        output = 1.0f - AZ::GetClamp(m_configuration.m_gradientSampler.GetValue(sampleParams), 0.0f, 1.0f);

        return output;
    }

    bool InvertGradientComponent::IsEntityInHierarchy(const AZ::EntityId& entityId) const
    {
        return m_configuration.m_gradientSampler.IsEntityInHierarchy(entityId);
    }

    GradientSampler& InvertGradientComponent::GetGradientSampler()
    {
        return m_configuration.m_gradientSampler;
    }
}
