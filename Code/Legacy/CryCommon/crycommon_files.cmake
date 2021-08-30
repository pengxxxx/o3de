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
    IEntityRenderState.h
    IFont.h
    IFunctorBase.h
    IGem.h
    IIndexedMesh.h
    ILevelSystem.h
    ILocalizationManager.h
    LocalizationManagerBus.h
    LocalizationManagerBus.inl
    ILog.h
    IMaterial.h
    IMiniLog.h
    IMovieSystem.h
    IProcess.h
    IReadWriteXMLSink.h
    IRenderAuxGeom.h
    IRenderer.h
    ISerialize.h
    IShader.h
    ISplines.h
    IStatObj.h
    StatObjBus.h
    ISurfaceType.h
    ISystem.h
    ITexture.h
    ITimer.h
    IValidator.h
    IViewSystem.h
    IWindowMessageHandler.h
    IXml.h
    MicrophoneBus.h
    HMDBus.h
    VRCommon.h
    INavigationSystem.h
    IMNM.h
    SFunctor.h
    FunctorBaseFunction.h
    FunctorBaseMember.h
    SerializationTypes.h
    CryEndian.h
    CryRandomInternal.h
    Random.h
    LCGRandom.h
    BaseTypes.h
    AnimKey.h
    BitFiddling.h
    Common_TypeInfo.cpp
    CryAssert.h
    CryCrc32.h
    CryFile.h
    CryListenerSet.h
    CryLegacyAllocator.h
    CryPath.h
    CryPodArray.h
    CrySizer.h
    CrySystemBus.h
    CryVersion.h
    LegacyAllocator.cpp
    LegacyAllocator.h
    MetaUtils.h
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
    Synchronization.h
    Timer.h
    TimeValue.h
    VectorMap.h
    VertexFormats.h
    XMLBinaryHeaders.h
    RenderBus.h
    MainThreadRenderRequestBus.h
    Cry_Matrix33.h
    Cry_Matrix34.h
    Cry_Matrix44.h
    Cry_Vector4.h
    Cry_Camera.h
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
    Cry_HWMatrix.h
    Cry_HWVector3.h
    AndroidSpecific.h
    AppleSpecific.h
    CryAssert_Android.h
    CryAssert_impl.h
    CryAssert_iOS.h
    CryAssert_Linux.h
    CryAssert_Mac.h
    CryLibrary.cpp
    CryLibrary.h
    Linux32Specific.h
    Linux64Specific.h
    Linux_Win32Wrapper.h
    LinuxSpecific.h
    LoadScreenBus.h
    MacSpecific.h
    platform.h
    platform_impl.cpp
    Win32specific.h
    Win64specific.h
    LyShine/IDraw2d.h
    LyShine/ILyShine.h
    LyShine/ISprite.h
    LyShine/IRenderGraph.h
    LyShine/UiAssetTypes.h
    LyShine/UiComponentTypes.h
    LyShine/UiBase.h
    LyShine/UiEntityContext.h
    LyShine/UiLayoutCellBase.h
    LyShine/UiSerializeHelpers.h
    LyShine/Animation/IUiAnimation.h
    LyShine/Bus/UiAnimateEntityBus.h
    LyShine/Bus/UiAnimationBus.h
    LyShine/Bus/UiButtonBus.h
    LyShine/Bus/UiCanvasBus.h
    LyShine/Bus/UiCanvasManagerBus.h
    LyShine/Bus/UiCanvasUpdateNotificationBus.h
    LyShine/Bus/UiCheckboxBus.h
    LyShine/Bus/UiCursorBus.h
    LyShine/Bus/UiDraggableBus.h
    LyShine/Bus/UiDropdownBus.h
    LyShine/Bus/UiDropdownOptionBus.h
    LyShine/Bus/UiDropTargetBus.h
    LyShine/Bus/UiDynamicLayoutBus.h
    LyShine/Bus/UiDynamicScrollBoxBus.h
    LyShine/Bus/UiEditorBus.h
    LyShine/Bus/UiEditorCanvasBus.h
    LyShine/Bus/UiEditorChangeNotificationBus.h
    LyShine/Bus/UiElementBus.h
    LyShine/Bus/UiEntityContextBus.h
    LyShine/Bus/UiFaderBus.h
    LyShine/Bus/UiFlipbookAnimationBus.h
    LyShine/Bus/UiGameEntityContextBus.h
    LyShine/Bus/UiImageBus.h
    LyShine/Bus/UiImageSequenceBus.h
    LyShine/Bus/UiIndexableImageBus.h
    LyShine/Bus/UiInitializationBus.h
    LyShine/Bus/UiInteractableActionsBus.h
    LyShine/Bus/UiInteractableBus.h
    LyShine/Bus/UiInteractableStatesBus.h
    LyShine/Bus/UiInteractionMaskBus.h
    LyShine/Bus/UiLayoutBus.h
    LyShine/Bus/UiLayoutCellBus.h
    LyShine/Bus/UiLayoutCellDefaultBus.h
    LyShine/Bus/UiLayoutColumnBus.h
    LyShine/Bus/UiLayoutControllerBus.h
    LyShine/Bus/UiLayoutFitterBus.h
    LyShine/Bus/UiLayoutGridBus.h
    LyShine/Bus/UiLayoutManagerBus.h
    LyShine/Bus/UiLayoutRowBus.h
    LyShine/Bus/UiMarkupButtonBus.h
    LyShine/Bus/UiMaskBus.h
    LyShine/Bus/UiNavigationBus.h
    LyShine/Bus/UiParticleEmitterBus.h
    LyShine/Bus/UiRadioButtonBus.h
    LyShine/Bus/UiRadioButtonCommunicationBus.h
    LyShine/Bus/UiRadioButtonGroupBus.h
    LyShine/Bus/UiRadioButtonGroupCommunicationBus.h
    LyShine/Bus/UiRenderBus.h
    LyShine/Bus/UiRenderControlBus.h
    LyShine/Bus/UiScrollableBus.h
    LyShine/Bus/UiScrollBarBus.h
    LyShine/Bus/UiScrollBoxBus.h
    LyShine/Bus/UiScrollerBus.h
    LyShine/Bus/UiSliderBus.h
    LyShine/Bus/UiSpawnerBus.h
    LyShine/Bus/UiSystemBus.h
    LyShine/Bus/UiTextBus.h
    LyShine/Bus/UiTextInputBus.h
    LyShine/Bus/UiTooltipBus.h
    LyShine/Bus/UiTooltipDataPopulatorBus.h
    LyShine/Bus/UiTooltipDisplayBus.h
    LyShine/Bus/UiTransform2dBus.h
    LyShine/Bus/UiTransformBus.h
    LyShine/Bus/UiVisualBus.h
    LyShine/Bus/Sprite/UiSpriteBus.h
    LyShine/Bus/World/UiCanvasOnMeshBus.h
    LyShine/Bus/World/UiCanvasRefBus.h
    LyShine/Bus/Tools/UiSystemToolsBus.h
    Maestro/Bus/EditorSequenceAgentComponentBus.h
    Maestro/Bus/EditorSequenceBus.h
    Maestro/Bus/EditorSequenceComponentBus.h
    Maestro/Bus/SequenceComponentBus.h
    Maestro/Bus/SequenceAgentComponentBus.h
    Maestro/Types/AnimNodeType.h
    Maestro/Types/AnimParamType.h
    Maestro/Types/AnimValue.h
    Maestro/Types/AnimValueType.h
    Maestro/Types/AssetBlendKey.h
    Maestro/Types/AssetBlends.h
    Maestro/Types/SequenceType.h
    StaticInstance.h
    WinBase.cpp
)
