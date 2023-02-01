#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    IAudioInterfacesCommonData.h
    IAudioSystem.h
    ICmdLine.h
    IConsole.h
    IFont.h
    IGem.h
    IIndexedMesh.h
    ILevelSystem.h
    ILocalizationManager.h
    LocalizationManagerBus.h
    LocalizationManagerBus.inl
    ILog.h
    IMiniLog.h
    IMovieSystem.h
    IRenderAuxGeom.h
    IRenderer.h
    ISerialize.h
    ISplines.h
    ISystem.h
    ITexture.h
    IValidator.h
    IWindowMessageHandler.h
    IXml.h
    MicrophoneBus.h
    INavigationSystem.h
    IMNM.h
    SerializationTypes.h
    CryEndian.h
    CryRandomInternal.h
    Random.h
    LCGRandom.h
    BaseTypes.h
    AnimKey.h
    CryAssert.h
    CryFile.h
    CryListenerSet.h
    CryPath.h
    CrySystemBus.h
    CryVersion.h
    MiniQueue.h
    MultiThread_Containers.h
    NullAudioSystem.h
    PNoise3.h
    ProjectDefines.h
    Range.h
    ScopedVariableSetter.h
    SerializeFwd.h
    SimpleSerialize.h
    smartptr.h
    StlUtils.h
    TimeValue.h
    VectorMap.h
    VertexFormats.h
    XMLBinaryHeaders.h
    MainThreadRenderRequestBus.h
    Cry_Matrix33.h
    Cry_Matrix34.h
    Cry_Matrix44.h
    Cry_Vector4.h
    Cry_Color.h
    Cry_Geo.h
    Cry_GeoDistance.h
    Cry_GeoIntersect.h
    Cry_Math.h
    Cry_Quat.h
    Cry_ValidNumber.h
    Cry_Vector2.h
    Cry_Vector3.h
    CryHalf.inl
    MathConversion.h
    AndroidSpecific.h
    AppleSpecific.h
    Linux32Specific.h
    Linux64Specific.h
    Linux_Win32Wrapper.h
    LinuxSpecific.h
    LoadScreenBus.h
    MacSpecific.h
    platform.h
    platform_impl.cpp
    Win64specific.h
    Maestro/Bus/EditorSequenceAgentComponentBus.h
    Maestro/Bus/EditorSequenceBus.h
    Maestro/Bus/EditorSequenceComponentBus.h
    Maestro/Bus/SequenceComponentBus.h
    Maestro/Bus/SequenceAgentComponentBus.h
    Maestro/Types/AnimNodeType.h
    Maestro/Types/AnimParamType.h
    Maestro/Types/AnimValueType.h
    Maestro/Types/AssetBlendKey.h
    Maestro/Types/AssetBlends.h
    Maestro/Types/SequenceType.h
    WinBase.cpp
)
