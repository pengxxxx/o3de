#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Include/Atom/RHI/Allocator.h
    Include/Atom/RHI/FreeListAllocator.h
    Include/Atom/RHI/LinearAllocator.h
    Include/Atom/RHI/PoolAllocator.h
    Source/RHI/Allocator.cpp
    Source/RHI/FreeListAllocator.cpp
    Source/RHI/LinearAllocator.cpp
    Source/RHI/PoolAllocator.cpp
    Include/Atom/RHI/Buffer.h
    Include/Atom/RHI/MultiDeviceBuffer.h
    Include/Atom/RHI/BufferView.h
    Include/Atom/RHI/IndexBufferView.h
    Include/Atom/RHI/MultiDeviceIndexBufferView.h
    Include/Atom/RHI/StreamBufferView.h
    Include/Atom/RHI/MultiDeviceStreamBufferView.h
    Source/RHI/Buffer.cpp
    Source/RHI/MultiDeviceBuffer.cpp
    Source/RHI/BufferView.cpp
    Source/RHI/IndexBufferView.cpp
    Source/RHI/MultiDeviceIndexBufferView.cpp
    Source/RHI/StreamBufferView.cpp
    Source/RHI/MultiDeviceStreamBufferView.cpp
    Include/Atom/RHI/BufferPool.h
    Include/Atom/RHI/MultiDeviceBufferPool.h
    Include/Atom/RHI/BufferPoolBase.h
    Include/Atom/RHI/MultiDeviceBufferPoolBase.h
    Source/RHI/BufferPool.cpp
    Source/RHI/MultiDeviceBufferPool.cpp
    Source/RHI/BufferPoolBase.cpp
    Source/RHI/MultiDeviceBufferPoolBase.cpp
    Include/Atom/RHI/CommandList.h
    Include/Atom/RHI/CommandListValidator.h
    Include/Atom/RHI/CommandListStates.h
    Include/Atom/RHI/CopyItem.h
    Include/Atom/RHI/MultiDeviceCopyItem.h
    Include/Atom/RHI/ConstantsData.h
    Include/Atom/RHI/DispatchItem.h
    Include/Atom/RHI/MultiDeviceDispatchItem.h
    Include/Atom/RHI/DrawFilterTagRegistry.h
    Include/Atom/RHI/DrawItem.h
    Include/Atom/RHI/MultiDeviceDrawItem.h
    Include/Atom/RHI/DrawList.h
    Include/Atom/RHI/DrawListTagRegistry.h
    Include/Atom/RHI/DrawListContext.h
    Include/Atom/RHI/DrawPacket.h
    Include/Atom/RHI/DrawPacketBuilder.h
    Include/Atom/RHI/IndirectArguments.h
    Include/Atom/RHI/MultiDeviceIndirectArguments.h
    Source/RHI/CommandList.cpp
    Source/RHI/CommandListValidator.cpp
    Source/RHI/ConstantsData.cpp
    Source/RHI/DrawList.cpp
    Source/RHI/DrawListContext.cpp
    Source/RHI/DrawPacket.cpp
    Source/RHI/DrawPacketBuilder.cpp
    Include/Atom/RHI/Device.h
    Include/Atom/RHI/DeviceBusTraits.h
    Include/Atom/RHI/DeviceObject.h
    Include/Atom/RHI/MultiDeviceObject.h
    Include/Atom/RHI/CommandQueue.h
    Include/Atom/RHI/ValidationLayer.h
    Source/RHI/Device.cpp
    Source/RHI/DeviceObject.cpp
    Source/RHI/MultiDeviceObject.cpp
    Source/RHI/CommandQueue.cpp
    Source/RHI/ValidationLayer.cpp
    Include/Atom/RHI/Factory.h
    Source/RHI/Factory.cpp
    Include/Atom/RHI/FactoryManagerBus.h
    Include/Atom/RHI/Fence.h
    Include/Atom/RHI/MultiDeviceFence.h
    Source/RHI/Fence.cpp
    Source/RHI/MultiDeviceFence.cpp
    Include/Atom/RHI/BufferFrameAttachment.h
    Include/Atom/RHI/FrameAttachment.h
    Include/Atom/RHI/ImageFrameAttachment.h
    Include/Atom/RHI/SwapChainFrameAttachment.h
    Source/RHI/BufferFrameAttachment.cpp
    Source/RHI/FrameAttachment.cpp
    Source/RHI/ImageFrameAttachment.cpp
    Source/RHI/SwapChainFrameAttachment.cpp
    Include/Atom/RHI/FrameGraph.h
    Include/Atom/RHI/FrameGraphAttachmentInterface.h
    Include/Atom/RHI/FrameGraphAttachmentDatabase.h
    Include/Atom/RHI/FrameGraphBuilder.h
    Include/Atom/RHI/FrameGraphCompileContext.h
    Include/Atom/RHI/FrameGraphCompiler.h
    Include/Atom/RHI/FrameGraphExecuteContext.h
    Include/Atom/RHI/FrameGraphExecuteGroup.h
    Include/Atom/RHI/FrameGraphExecuter.h
    Include/Atom/RHI/FrameGraphLogger.h
    Include/Atom/RHI/FrameGraphInterface.h
    Source/RHI/FrameGraph.cpp
    Source/RHI/FrameGraphAttachmentDatabase.cpp
    Source/RHI/FrameGraphCompileContext.cpp
    Source/RHI/FrameGraphCompiler.cpp
    Source/RHI/FrameGraphExecuteContext.cpp
    Source/RHI/FrameGraphExecuteGroup.cpp
    Source/RHI/FrameGraphExecuter.cpp
    Source/RHI/FrameGraphLogger.cpp
    Include/Atom/RHI/FrameEventBus.h
    Include/Atom/RHI/FrameScheduler.h
    Source/RHI/FrameScheduler.cpp
    Include/Atom/RHI/Image.h
    Include/Atom/RHI/MultiDeviceImage.h
    Include/Atom/RHI/ImageView.h
    Source/RHI/Image.cpp
    Source/RHI/MultiDeviceImage.cpp
    Source/RHI/ImageView.cpp
    Include/Atom/RHI/ImagePool.h
    Include/Atom/RHI/MultiDeviceImagePool.h
    Include/Atom/RHI/ImagePoolBase.h
    Include/Atom/RHI/MultiDeviceImagePoolBase.h
    Include/Atom/RHI/StreamingImagePool.h
    Include/Atom/RHI/MultiDeviceStreamingImagePool.h
    Source/RHI/ImagePool.cpp
    Source/RHI/MultiDeviceImagePool.cpp
    Source/RHI/ImagePoolBase.cpp
    Source/RHI/MultiDeviceImagePoolBase.cpp
    Source/RHI/StreamingImagePool.cpp
    Source/RHI/MultiDeviceStreamingImagePool.cpp
    Include/Atom/RHI/IndirectBufferSignature.h
    Include/Atom/RHI/MultiDeviceIndirectBufferSignature.h
    Include/Atom/RHI/IndirectBufferView.h
    Include/Atom/RHI/MultiDeviceIndirectBufferView.h
    Include/Atom/RHI/IndirectBufferWriter.h
    Include/Atom/RHI/MultiDeviceIndirectBufferWriter.h
    Source/RHI/IndirectBufferSignature.cpp
    Source/RHI/MultiDeviceIndirectBufferSignature.cpp
    Source/RHI/IndirectBufferView.cpp
    Source/RHI/MultiDeviceIndirectBufferView.cpp
    Source/RHI/IndirectBufferWriter.cpp
    Source/RHI/MultiDeviceIndirectBufferWriter.cpp
    Include/Atom/RHI/Object.h
    Include/Atom/RHI/ObjectCache.h
    Include/Atom/RHI/ObjectCollector.h
    Include/Atom/RHI/ObjectPool.h
    Source/RHI/Object.cpp
    Include/Atom/RHI/PageTileAllocator.h
    Include/Atom/RHI/PageTiles.h
    Source/RHI/PageTileAllocator.cpp
    Include/Atom/RHI/PhysicalDevice.h
    Source/RHI/PhysicalDevice.cpp
    Include/Atom/RHI/PipelineLibrary.h
    Include/Atom/RHI/MultiDevicePipelineLibrary.h
    Include/Atom/RHI/PipelineState.h
    Include/Atom/RHI/MultiDevicePipelineState.h
    Include/Atom/RHI/PipelineStateCache.h
    Include/Atom/RHI/PipelineStateDescriptor.h
    Source/RHI/PipelineLibrary.cpp
    Source/RHI/MultiDevicePipelineLibrary.cpp
    Source/RHI/PipelineState.cpp
    Source/RHI/MultiDevicePipelineState.cpp
    Source/RHI/PipelineStateCache.cpp
    Source/RHI/PipelineStateDescriptor.cpp
    Include/Atom/RHI/Query.h
    Include/Atom/RHI/MultiDeviceQuery.h
    Source/RHI/Query.cpp
    Source/RHI/MultiDeviceQuery.cpp
    Include/Atom/RHI/QueryPool.h
    Include/Atom/RHI/MultiDeviceQueryPool.h
    Include/Atom/RHI/QueryPoolSubAllocator.h
    Source/RHI/QueryPool.cpp
    Source/RHI/MultiDeviceQueryPool.cpp
    Source/RHI/QueryPoolSubAllocator.cpp
    Include/Atom/RHI/Resource.h
    Include/Atom/RHI/MultiDeviceResource.h
    Include/Atom/RHI/ResourceInvalidateBus.h
    Include/Atom/RHI/ResourceView.h
    Source/RHI/Resource.cpp
    Source/RHI/MultiDeviceResource.cpp
    Source/RHI/ResourceView.cpp
    Include/Atom/RHI/ResourcePool.h
    Include/Atom/RHI/MultiDeviceResourcePool.h
    Include/Atom/RHI/ResourcePoolDatabase.h
    Source/RHI/ResourcePool.cpp
    Source/RHI/MultiDeviceResourcePool.cpp
    Source/RHI/ResourcePoolDatabase.cpp
    Include/Atom/RHI/MemoryAllocation.h
    Include/Atom/RHI/MemorySubAllocator.h
    Include/Atom/RHI/MemoryLinearSubAllocator.h
    Include/Atom/RHI/Scope.h
    Include/Atom/RHI/ScopeProducer.h
    Include/Atom/RHI/ScopeProducerEmpty.h
    Include/Atom/RHI/ScopeProducerFunction.h
    Source/RHI/Scope.cpp
    Source/RHI/ScopeProducer.cpp
    Include/Atom/RHI/BufferScopeAttachment.h
    Include/Atom/RHI/ImageScopeAttachment.h
    Include/Atom/RHI/ResolveScopeAttachment.h
    Include/Atom/RHI/ScopeAttachment.h
    Source/RHI/BufferScopeAttachment.cpp
    Source/RHI/ImageScopeAttachment.cpp
    Source/RHI/ResolveScopeAttachment.cpp
    Source/RHI/ScopeAttachment.cpp
    Include/Atom/RHI/ShaderResourceGroup.h    
    Include/Atom/RHI/MultiDeviceShaderResourceGroup.h
    Include/Atom/RHI/ShaderResourceGroupData.h
    Include/Atom/RHI/MultiDeviceShaderResourceGroupData.h
    Include/Atom/RHI/ShaderResourceGroupDebug.h
    Include/Atom/RHI/ShaderResourceGroupInvalidateRegistry.h
    Include/Atom/RHI/ShaderResourceGroupPool.h
    Include/Atom/RHI/MultiDeviceShaderResourceGroupPool.h
    Source/RHI/ShaderResourceGroup.cpp
    Source/RHI/MultiDeviceShaderResourceGroup.cpp
    Source/RHI/ShaderResourceGroupData.cpp
    Source/RHI/MultiDeviceShaderResourceGroupData.cpp
    Source/RHI/ShaderResourceGroupDebug.cpp
    Source/RHI/ShaderResourceGroupInvalidateRegistry.cpp
    Source/RHI/ShaderResourceGroupPool.cpp
    Source/RHI/MultiDeviceShaderResourceGroupPool.cpp
    Include/Atom/RHI/MemoryStatisticsBuilder.h
    Include/Atom/RHI/MemoryStatisticsBus.h
    Source/RHI/MemoryStatisticsBuilder.cpp
    Include/Atom/RHI/SwapChain.h
    Source/RHI/SwapChain.cpp
    Include/Atom/RHI/RHISystem.h
    Include/Atom/RHI/RHISystemInterface.h
    Source/RHI/RHISystem.cpp
    Include/Atom/RHI/ThreadLocalContext.h
    Include/Atom/RHI/AsyncWorkQueue.h
    Source/RHI/AsyncWorkQueue.cpp
    Include/Atom/RHI/AliasedHeap.h
    Source/RHI/AliasedHeap.cpp
    Include/Atom/RHI/AliasedAttachmentAllocator.h
    Include/Atom/RHI/AliasingBarrierTracker.h
    Source/RHI/AliasingBarrierTracker.cpp
    Include/Atom/RHI/TileAllocator.h
    Include/Atom/RHI/TileAllocator.inl
    Include/Atom/RHI/TransientAttachmentPool.h
    Source/RHI/TransientAttachmentPool.cpp
    Include/Atom/RHI/RHIUtils.h
    Source/RHI/RHIUtils.cpp
    Include/Atom/RHI/RayTracingAccelerationStructure.h
    Include/Atom/RHI/RayTracingPipelineState.h
    Include/Atom/RHI/RayTracingShaderTable.h
    Include/Atom/RHI/RayTracingBufferPools.h
    Include/Atom/RHI/DispatchRaysItem.h
    Include/Atom/RHI/MultiDeviceDispatchRaysItem.h
    Source/RHI/RayTracingAccelerationStructure.cpp
    Source/RHI/RayTracingPipelineState.cpp
    Source/RHI/RayTracingShaderTable.cpp
    Source/RHI/RayTracingBufferPools.cpp
    Include/Atom/RHI/interval_map.h
    Include/Atom/RHI/ImageProperty.h
    Include/Atom/RHI/BufferProperty.h
    Include/Atom/RHI/TagBitRegistry.h
    Include/Atom/RHI/TagRegistry.h
    Include/Atom/RHI/XRRenderingInterface.h
    Include/Atom/RHI/DispatchRaysIndirectBuffer.h
)
