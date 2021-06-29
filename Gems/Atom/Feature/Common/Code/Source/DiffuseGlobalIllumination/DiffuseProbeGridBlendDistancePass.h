/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <Atom/RHI/CommandList.h>
#include <Atom/RHI/ScopeProducer.h>
#include <Atom/RPI.Public/Pass/RenderPass.h>
#include <Atom/RPI.Public/Shader/Shader.h>

namespace AZ
{
    namespace RHI
    {
        class PipelineState;
    }

    namespace Render
    {
        //! Compute shader that generates the distance texture from the diffuse probe raytrace.
        class DiffuseProbeGridBlendDistancePass final
            : public RPI::RenderPass
        {
        public:
            AZ_RPI_PASS(DiffuseProbeGridBlendDistancePass);

            AZ_RTTI(AZ::Render::DiffuseProbeGridBlendDistancePass, "{8D57B51E-3432-49F6-83F8-DB6DF2F5A5CA}", RPI::RenderPass);
            AZ_CLASS_ALLOCATOR(DiffuseProbeGridBlendDistancePass, SystemAllocator, 0);
            virtual ~DiffuseProbeGridBlendDistancePass() = default;

            static RPI::Ptr<DiffuseProbeGridBlendDistancePass> Create(const RPI::PassDescriptor& descriptor);

        private:
            DiffuseProbeGridBlendDistancePass(const RPI::PassDescriptor& descriptor);

            void LoadShader();

            // Pass overrides
            void FrameBeginInternal(FramePrepareParams params) override;

            void SetupFrameGraphDependencies(RHI::FrameGraphInterface frameGraph) override;
            void CompileResources(const RHI::FrameGraphCompileContext& context) override;
            void BuildCommandListInternal(const RHI::FrameGraphExecuteContext& context) override;

            // shader
            Data::Instance<RPI::Shader> m_shader;
            const RHI::PipelineState* m_pipelineState = nullptr;
            RHI::Ptr<RHI::ShaderResourceGroupLayout> m_srgLayout;
            RHI::DispatchDirect m_dispatchArgs;
        };
    }   // namespace Render
}   // namespace AZ
