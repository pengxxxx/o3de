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

#include <Atom/RPI.Public/FeatureProcessor.h>
#include <Atom/Feature/CoreLights/PhotometricValue.h>

namespace AZ
{
    class Vector3;

    namespace Render
    {
        //! SimpleSpotLightFeatureProcessorInterface provides an interface to acquire, release, and update a simple spot light.
        class SimpleSpotLightFeatureProcessorInterface
            : public RPI::FeatureProcessor
        {
        public:
            AZ_RTTI(AZ::Render::SimpleSpotLightFeatureProcessorInterface, "{1DE04BF2-DD8F-437C-9B6D-4BDAC4BE2BAC}", AZ::RPI::FeatureProcessor);
            
            using LightHandle = RHI::Handle<uint16_t, class SimpleSpotLight>;
            static constexpr PhotometricUnit PhotometricUnitType = PhotometricUnit::Candela;

            //! Creates a new spot light which can be referenced by the returned LightHandle. Must be released via ReleaseLight() when no longer needed.
            virtual LightHandle AcquireLight() = 0;
            //! Releases a LightHandle which removes the spot light.
            virtual bool ReleaseLight(LightHandle& handle) = 0;
            //! Creates a new LightHandle by copying data from an existing LightHandle.
            virtual LightHandle CloneLight(LightHandle handle) = 0;

            //! Sets the intensity in RGB candela for a given LightHandle.
            virtual void SetRgbIntensity(LightHandle handle, const PhotometricColor<PhotometricUnitType>& lightColor) = 0;
            //! Sets the position for a given LightHandle.
            virtual void SetPosition(LightHandle handle, const AZ::Vector3& lightPosition) = 0;
            //! Sets the direction for a given LightHandle.
            virtual void SetDirection(LightHandle handle, const AZ::Vector3& lightDirection) = 0;
            //! Sets the radius in meters at which the provided LightHandle will no longer have an effect.
            virtual void SetAttenuationRadius(LightHandle handle, float attenuationRadius) = 0;
            //! Sets the inner and outer cone angles in radians.
            virtual void SetConeAngles(LightHandle handle, float innerRadians, float outerRadians) = 0;
        };
    } // namespace Render
} // namespace AZ
