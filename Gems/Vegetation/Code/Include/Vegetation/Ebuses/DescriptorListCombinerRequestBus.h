/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Component/EntityId.h>

namespace Vegetation
{
    class DescriptorListCombinerRequests
        : public AZ::ComponentBus
    {
    public:
        /**
         * Overrides the default AZ::EBusTraits handler policy to allow one
         * listener only.
         */
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

        virtual size_t GetNumDescriptors() const = 0;
        virtual AZ::EntityId GetDescriptorEntityId(int index) const = 0;
        virtual void RemoveDescriptorEntityId(int index) = 0;
        virtual void SetDescriptorEntityId(int index, AZ::EntityId entityId) = 0;
        virtual void AddDescriptorEntityId(AZ::EntityId entityId) = 0;
    };

    using DescriptorListCombinerRequestBus = AZ::EBus<DescriptorListCombinerRequests>;
}
