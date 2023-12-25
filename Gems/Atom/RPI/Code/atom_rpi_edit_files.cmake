#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Include/Atom/RPI.Edit/Common/AssetUtils.h
    Include/Atom/RPI.Edit/Common/AssetAliasesSourceData.h
    Include/Atom/RPI.Edit/Common/ColorUtils.h
    Include/Atom/RPI.Edit/Common/ConvertibleSource.h
    Include/Atom/RPI.Edit/Common/JsonReportingHelper.h
    Include/Atom/RPI.Edit/Common/JsonUtils.h
    Include/Atom/RPI.Edit/Material/LuaMaterialFunctorSourceData.h
    Include/Atom/RPI.Edit/Material/MaterialTypeSourceData.h
    Include/Atom/RPI.Edit/Material/MaterialConverterBus.h
    Include/Atom/RPI.Edit/Material/MaterialPropertyId.h
    Include/Atom/RPI.Edit/Material/MaterialPropertyConnectionSerializer.h
    Include/Atom/RPI.Edit/Material/MaterialPropertyGroupSerializer.h
    Include/Atom/RPI.Edit/Material/MaterialPropertySerializer.h
    Include/Atom/RPI.Edit/Material/MaterialPropertySourceData.h
    Include/Atom/RPI.Edit/Material/MaterialPropertyValueSerializer.h
    Include/Atom/RPI.Edit/Material/MaterialPropertyValueSourceData.h
    Include/Atom/RPI.Edit/Material/MaterialPropertyValueSourceDataSerializer.h
    Include/Atom/RPI.Edit/Material/MaterialSourceData.h
    Include/Atom/RPI.Edit/Material/MaterialFunctorSourceData.h
    Include/Atom/RPI.Edit/Material/MaterialFunctorSourceDataHolder.h
    Include/Atom/RPI.Edit/Material/MaterialFunctorSourceDataSerializer.h
    Include/Atom/RPI.Edit/Material/MaterialFunctorSourceDataRegistration.h
    Include/Atom/RPI.Edit/Material/MaterialPipelineSourceData.h
    Include/Atom/RPI.Edit/Material/MaterialUtils.h
    Include/Atom/RPI.Edit/ResourcePool/ResourcePoolSourceData.h
    Include/Atom/RPI.Edit/Shader/ShaderOptionValuesSourceData.h
    Include/Atom/RPI.Edit/Shader/ShaderSourceData.h
    Include/Atom/RPI.Edit/Shader/ShaderVariantListSourceData.h
    Include/Atom/RPI.Edit/Shader/ShaderVariantAssetCreator.h
    Include/Atom/RPI.Edit/Shader/ShaderVariantTreeAssetCreator.h
    Include/Atom/RPI.Edit/Rendering/RenderingSettingData.h
    Include/Atom/RPI.Edit/Rendering/RenderingSettingDataSerializer.h
    Include/Atom/RPI.Edit/Rendering/RenderingSettingUtil.h
    Source/RPI.Edit/Material/LuaMaterialFunctorSourceData.cpp
    Source/RPI.Edit/Material/MaterialTypeSourceData.cpp
    Source/RPI.Edit/Material/MaterialPropertyId.cpp
    Source/RPI.Edit/Material/MaterialPropertyGroupSerializer.cpp
    Source/RPI.Edit/Material/MaterialPropertyConnectionSerializer.cpp
    Source/RPI.Edit/Material/MaterialPropertySerializer.cpp
    Source/RPI.Edit/Material/MaterialPropertySourceData.cpp
    Source/RPI.Edit/Material/MaterialPropertyValueSerializer.cpp
    Source/RPI.Edit/Material/MaterialPropertyValueSourceData.cpp
    Source/RPI.Edit/Material/MaterialPropertyValueSourceDataSerializer.cpp
    Source/RPI.Edit/Material/MaterialSourceData.cpp
    Source/RPI.Edit/Material/MaterialFunctorSourceData.cpp
    Source/RPI.Edit/Material/MaterialFunctorSourceDataHolder.cpp
    Source/RPI.Edit/Material/MaterialFunctorSourceDataSerializer.cpp
    Source/RPI.Edit/Material/MaterialFunctorSourceDataRegistration.cpp
    Source/RPI.Edit/Material/MaterialPipelineSourceData.cpp
    Source/RPI.Edit/Material/MaterialUtils.cpp
    Source/RPI.Edit/Shader/ShaderSourceData.cpp
    Source/RPI.Edit/Shader/ShaderVariantListSourceData.cpp
    Source/RPI.Edit/Shader/ShaderVariantAssetCreator.cpp
    Source/RPI.Edit/Shader/ShaderVariantTreeAssetCreator.cpp
    Source/RPI.Edit/Common/AssetUtils.cpp
    Source/RPI.Edit/Common/AssetAliasesSourceData.cpp
    Source/RPI.Edit/Common/ColorUtils.cpp
    Source/RPI.Edit/Common/ConvertibleSource.cpp
    Source/RPI.Edit/Common/JsonReportingHelper.cpp
    Source/RPI.Edit/Common/JsonUtils.cpp
    Source/RPI.Edit/Rendering/RenderingSettingData.cpp
    Source/RPI.Edit/Rendering/RenderingSettingDataSerializer.cpp
)
