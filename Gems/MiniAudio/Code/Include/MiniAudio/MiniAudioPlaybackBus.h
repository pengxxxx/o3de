/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <MiniAudio/SoundAsset.h>
#include <MiniAudio/SoundAssetRef.h>
#include <AzCore/Math/Vector3.h>

namespace MiniAudio
{
    class MiniAudioPlaybackRequests : public AZ::ComponentBus
    {
    public:
        ~MiniAudioPlaybackRequests() override = default;

        virtual void Play() = 0;
        virtual void Stop() = 0;
        virtual void SetVolume(float volume) = 0;
        virtual void SetLooping(bool loop) = 0;
        virtual bool IsLooping() const = 0;
        virtual void SetSoundAsset(AZ::Data::Asset<SoundAsset> soundAsset) = 0;
        virtual AZ::Data::Asset<SoundAsset> GetSoundAsset() const = 0;

        //! Custom setter for scripting
        virtual void SetSoundAssetRef(const SoundAssetRef& soundAssetRef) = 0;
        //! Custom getter for scripting
        virtual SoundAssetRef GetSoundAssetRef() const = 0;

        //! Cone controls for directional attenuation
        virtual float GetInnerAngleInRadians() const = 0;
        virtual void SetInnerAngleInRadians(const float& innerAngleInRadians) = 0;
        virtual float GetInnerAngleInDegrees() const = 0;
        virtual void SetInnerAngleInDegrees(const float& innerAngleInDegrees) = 0;
        virtual float GetOuterAngleInRadians() const = 0;
        virtual void SetOuterAngleInRadians(const float& outerAngleInRadians) = 0;
        virtual float GetOuterAngleInDegrees() const = 0;
        virtual void SetOuterAngleInDegrees(const float& outerAngleInDegrees) = 0;
        virtual float GetOuterVolume() const = 0;
        virtual void SetOuterVolume(const float& outerVolume) = 0;
        virtual bool GetFixedDirecion() const = 0;
        virtual void SetFixedDirecion(const bool& fixedDirection) = 0;
        virtual float GetDirectionalAttenuationFactor() const = 0;
        virtual void SetDirectionalAttenuationFactor(const float& directionalAttenuationFactor) = 0;
        virtual AZ::Vector3 GetDirection() const = 0;
        virtual void SetDirection(const AZ::Vector3& direction) = 0;
    };

    using MiniAudioPlaybackRequestBus = AZ::EBus<MiniAudioPlaybackRequests>;

} // namespace MiniAudio
