/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Thumbnails/Rendering/CommonThumbnailRenderer.h>
#include <Thumbnails/Rendering/ThumbnailRendererSteps/CaptureStep.h>

namespace AZ
{
    namespace LyIntegration
    {
        namespace Thumbnails
        {
            CaptureStep::CaptureStep(CommonThumbnailRenderer* renderer)
                : ThumbnailRendererStep(renderer)
            {
            }

            void CaptureStep::Start()
            {
                m_ticksToCapture = 1;
                m_renderer->UpdateScene();
                TickBus::Handler::BusConnect();
            }

            void CaptureStep::Stop()
            {
                m_renderer->EndCapture();
                TickBus::Handler::BusDisconnect();
                Render::FrameCaptureNotificationBus::Handler::BusDisconnect();
            }

            void CaptureStep::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] ScriptTimePoint time)
            {
                if (m_ticksToCapture-- <= 0)
                {
                    // Reset the capture flag if the capture request was successful. Otherwise try capture it again next tick.
                    if (m_renderer->StartCapture())
                    {
                        Render::FrameCaptureNotificationBus::Handler::BusConnect();
                        TickBus::Handler::BusDisconnect();
                    }
                }
            }

            void CaptureStep::OnCaptureFinished(
                [[maybe_unused]] Render::FrameCaptureResult result, [[maybe_unused]] const AZStd::string& info)
            {
                m_renderer->CompleteThumbnail();
            }
        } // namespace Thumbnails
    } // namespace LyIntegration
} // namespace AZ
