/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */


#pragma once

#include <AzCore/EBus/EBus.h>

namespace EMotionFX
{
    class AnimGraphNode;
    class AnimGraphObject;

    namespace Integration
    {
        class EMotionFXRequests
            : public AZ::EBusTraits
        {
        public:
            static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
            static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

            virtual void RegisterAnimGraphObjectType(EMotionFX::AnimGraphObject* objectTemplate) = 0;
        };
        using EMotionFXRequestBus = AZ::EBus<EMotionFXRequests>;
    }
}
