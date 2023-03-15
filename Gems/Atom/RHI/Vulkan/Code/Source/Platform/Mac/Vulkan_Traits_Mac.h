/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#define AZ_TRAIT_ATOM_SHADERBUILDER_DXC "Builders/DirectXShaderCompiler/bin/dxc"
#define AZ_TRAIT_ATOM_AZSL_SHADER_HEADER UNUSED_TRAIT // Mac can not build vulkan shaders at the moment
#define AZ_TRAIT_ATOM_AZSL_PLATFORM_HEADER UNUSED_TRAIT // Mac can not build vulkan shaders at the moment
#define AZ_TRAIT_ATOM_VULKAN_DISABLE_DUAL_SOURCE_BLENDING 0
#define AZ_TRAIT_ATOM_VULKAN_LAYER_LUNARG_STD_VALIDATION_SUPPORT 0
#define AZ_TRAIT_ATOM_VULKAN_RECREATE_SWAPCHAIN_WHEN_SUBOPTIMAL 1
