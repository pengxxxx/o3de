/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <PostProcessing/EditorModeFeedbackDepthTransitionPass.h>

namespace AZ
{
    namespace Render
    {
        /**
         *  The color grading pass.
         */
        class EditorModeDesaturationPass
            : public EditorModeFeedbackDepthTransitionPass
            //TODO: , public PostProcessingShaderOptionBase
        {
        public:
            AZ_RTTI(EditorModeDesaturationPass, "{3587B748-7EA8-497F-B2D1-F60E369EACF4}", EditorModeFeedbackDepthTransitionPass);
            AZ_CLASS_ALLOCATOR(EditorModeDesaturationPass, SystemAllocator, 0);

            virtual ~EditorModeDesaturationPass() = default;

            //! Creates a EditorModeDesaturationPass
            static RPI::Ptr<EditorModeDesaturationPass> Create(const RPI::PassDescriptor& descriptor);

            void SetDesaturationAmount(float value);

        protected:
            EditorModeDesaturationPass(const RPI::PassDescriptor& descriptor);
            
            //! Pass behavior overrides
            void InitializeInternal() override;
            void FrameBeginInternal(FramePrepareParams params) override;
            bool IsEnabled() const override;

        private:
            void SetSrgConstants();

            RHI::ShaderInputNameIndex m_desaturationAmountIndex = "m_desaturationAmount";

            float m_desaturationAmount = 1.0f;
        };
    }   // namespace Render
}   // namespace AZ
