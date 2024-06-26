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
    Include/Atom/RHI/DeviceBuffer.h
    Include/Atom/RHI/Buffer.h
    Include/Atom/RHI/DeviceBufferView.h
    Include/Atom/RHI/DeviceIndexBufferView.h
    Include/Atom/RHI/IndexBufferView.h
    Include/Atom/RHI/DeviceStreamBufferView.h
    Include/Atom/RHI/StreamBufferView.h
    Source/RHI/DeviceBuffer.cpp
    Source/RHI/Buffer.cpp
    Source/RHI/DeviceBufferView.cpp
    Source/RHI/DeviceIndexBufferView.cpp
    Source/RHI/IndexBufferView.cpp
    Source/RHI/DeviceStreamBufferView.cpp
    Source/RHI/StreamBufferView.cpp
    Include/Atom/RHI/DeviceBufferPool.h
    Include/Atom/RHI/BufferPool.h
    Include/Atom/RHI/DeviceBufferPoolBase.h
    Include/Atom/RHI/BufferPoolBase.h
    Source/RHI/DeviceBufferPool.cpp
    Source/RHI/BufferPool.cpp
    Source/RHI/DeviceBufferPoolBase.cpp
    Source/RHI/BufferPoolBase.cpp
    Include/Atom/RHI/CommandList.h
    Include/Atom/RHI/CommandListValidator.h
    Include/Atom/RHI/CommandListStates.h
    Include/Atom/RHI/DeviceCopyItem.h
    Include/Atom/RHI/CopyItem.h
    Include/Atom/RHI/ConstantsData.h
    Include/Atom/RHI/DeviceDispatchItem.h
    Include/Atom/RHI/DispatchItem.h
    Include/Atom/RHI/DrawFilterTagRegistry.h
    Include/Atom/RHI/DeviceDrawItem.h
    Include/Atom/RHI/DrawItem.h
    Source/RHI/DrawItem.cpp
    Include/Atom/RHI/DrawList.h
    Include/Atom/RHI/DrawListTagRegistry.h
    Include/Atom/RHI/DrawListContext.h
    Include/Atom/RHI/DeviceDrawPacket.h
    Include/Atom/RHI/DrawPacket.h
    Include/Atom/RHI/DeviceDrawPacketBuilder.h
    Include/Atom/RHI/DrawPacketBuilder.h
    Include/Atom/RHI/DeviceIndirectArguments.h
    Include/Atom/RHI/IndirectArguments.h
    Source/RHI/CommandList.cpp
    Source/RHI/CommandListValidator.cpp
    Source/RHI/ConstantsData.cpp
    Source/RHI/DrawList.cpp
    Source/RHI/DrawListContext.cpp
    Source/RHI/DeviceDrawPacket.cpp
    Source/RHI/DrawPacket.cpp
    Source/RHI/DeviceDrawPacketBuilder.cpp
    Source/RHI/DrawPacketBuilder.cpp
    Source/RHI/DrawItem.cpp
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
    Include/Atom/RHI/DeviceFence.h
    Include/Atom/RHI/Fence.h
    Source/RHI/DeviceFence.cpp
    Source/RHI/Fence.cpp
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
    Include/Atom/RHI/DeviceImage.h
    Include/Atom/RHI/Image.h
    Include/Atom/RHI/DeviceImageView.h
    Source/RHI/DeviceImage.cpp
    Source/RHI/Image.cpp
    Source/RHI/DeviceImageView.cpp
    Include/Atom/RHI/DeviceImagePool.h
    Include/Atom/RHI/ImagePool.h
    Include/Atom/RHI/DeviceImagePoolBase.h
    Include/Atom/RHI/ImagePoolBase.h
    Include/Atom/RHI/DeviceStreamingImagePool.h
    Include/Atom/RHI/StreamingImagePool.h
    Source/RHI/DeviceImagePool.cpp
    Source/RHI/ImagePool.cpp
    Source/RHI/DeviceImagePoolBase.cpp
    Source/RHI/ImagePoolBase.cpp
    Source/RHI/DeviceStreamingImagePool.cpp
    Source/RHI/StreamingImagePool.cpp
    Include/Atom/RHI/DeviceIndirectBufferSignature.h
    Include/Atom/RHI/IndirectBufferSignature.h
    Include/Atom/RHI/DeviceIndirectBufferView.h
    Include/Atom/RHI/IndirectBufferView.h
    Include/Atom/RHI/DeviceIndirectBufferWriter.h
    Include/Atom/RHI/IndirectBufferWriter.h
    Source/RHI/DeviceIndirectBufferSignature.cpp
    Source/RHI/IndirectBufferSignature.cpp
    Source/RHI/DeviceIndirectBufferView.cpp
    Source/RHI/IndirectBufferView.cpp
    Source/RHI/DeviceIndirectBufferWriter.cpp
    Source/RHI/IndirectBufferWriter.cpp
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
    Include/Atom/RHI/DevicePipelineLibrary.h
    Include/Atom/RHI/PipelineLibrary.h
    Include/Atom/RHI/DevicePipelineState.h
    Include/Atom/RHI/PipelineState.h
    Include/Atom/RHI/PipelineStateCache.h
    Include/Atom/RHI/PipelineStateDescriptor.h
    Source/RHI/DevicePipelineLibrary.cpp
    Source/RHI/PipelineLibrary.cpp
    Source/RHI/DevicePipelineState.cpp
    Source/RHI/PipelineState.cpp
    Source/RHI/PipelineStateCache.cpp
    Source/RHI/PipelineStateDescriptor.cpp
    Include/Atom/RHI/DeviceQuery.h
    Include/Atom/RHI/Query.h
    Source/RHI/DeviceQuery.cpp
    Source/RHI/Query.cpp
    Include/Atom/RHI/DeviceQueryPool.h
    Include/Atom/RHI/QueryPool.h
    Include/Atom/RHI/QueryPoolSubAllocator.h
    Source/RHI/DeviceQueryPool.cpp
    Source/RHI/QueryPool.cpp
    Source/RHI/QueryPoolSubAllocator.cpp
    Include/Atom/RHI/DeviceResource.h
    Include/Atom/RHI/Resource.h
    Include/Atom/RHI/ResourceInvalidateBus.h
    Include/Atom/RHI/DeviceResourceView.h
    Source/RHI/DeviceResource.cpp
    Source/RHI/Resource.cpp
    Source/RHI/DeviceResourceView.cpp
    Include/Atom/RHI/DeviceResourcePool.h
    Include/Atom/RHI/ResourcePool.h
    Include/Atom/RHI/ResourcePoolDatabase.h
    Source/RHI/DeviceResourcePool.cpp
    Source/RHI/ResourcePool.cpp
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
    Include/Atom/RHI/DeviceShaderResourceGroup.h
    Include/Atom/RHI/ShaderResourceGroup.h
    Include/Atom/RHI/DeviceShaderResourceGroupData.h
    Include/Atom/RHI/ShaderResourceGroupData.h
    Include/Atom/RHI/ShaderResourceGroupDebug.h
    Include/Atom/RHI/ShaderResourceGroupInvalidateRegistry.h
    Include/Atom/RHI/DeviceShaderResourceGroupPool.h
    Include/Atom/RHI/ShaderResourceGroupPool.h
    Source/RHI/DeviceShaderResourceGroup.cpp
    Source/RHI/ShaderResourceGroup.cpp
    Source/RHI/DeviceShaderResourceGroupData.cpp
    Source/RHI/ShaderResourceGroupData.cpp
    Source/RHI/ShaderResourceGroupDebug.cpp
    Source/RHI/ShaderResourceGroupInvalidateRegistry.cpp
    Source/RHI/DeviceShaderResourceGroupPool.cpp
    Source/RHI/ShaderResourceGroupPool.cpp
    Include/Atom/RHI/MemoryStatisticsBuilder.h
    Include/Atom/RHI/MemoryStatisticsBus.h
    Source/RHI/MemoryStatisticsBuilder.cpp
    Include/Atom/RHI/DeviceSwapChain.h
    Include/Atom/RHI/SwapChain.h
    Source/RHI/DeviceSwapChain.cpp
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
    Include/Atom/RHI/DeviceTransientAttachmentPool.h
    Include/Atom/RHI/TransientAttachmentPool.h
    Source/RHI/DeviceTransientAttachmentPool.cpp
    Source/RHI/TransientAttachmentPool.cpp
    Include/Atom/RHI/RHIUtils.h
    Source/RHI/RHIUtils.cpp
    Include/Atom/RHI/DeviceRayTracingAccelerationStructure.h
    Include/Atom/RHI/RayTracingAccelerationStructure.h
    Include/Atom/RHI/DeviceRayTracingPipelineState.h
    Include/Atom/RHI/RayTracingPipelineState.h
    Include/Atom/RHI/DeviceRayTracingShaderTable.h
    Include/Atom/RHI/RayTracingShaderTable.h
    Include/Atom/RHI/DeviceRayTracingBufferPools.h
    Include/Atom/RHI/RayTracingBufferPools.h
    Include/Atom/RHI/DeviceDispatchRaysItem.h
    Include/Atom/RHI/DispatchRaysItem.h
    Source/RHI/DeviceRayTracingAccelerationStructure.cpp
    Source/RHI/RayTracingAccelerationStructure.cpp
    Source/RHI/DeviceRayTracingPipelineState.cpp
    Source/RHI/RayTracingPipelineState.cpp
    Source/RHI/DeviceRayTracingShaderTable.cpp
    Source/RHI/RayTracingShaderTable.cpp
    Source/RHI/DeviceRayTracingBufferPools.cpp
    Source/RHI/RayTracingBufferPools.cpp
    Include/Atom/RHI/interval_map.h
    Include/Atom/RHI/ImageProperty.h
    Include/Atom/RHI/BufferProperty.h
    Include/Atom/RHI/TagBitRegistry.h
    Include/Atom/RHI/TagRegistry.h
    Include/Atom/RHI/XRRenderingInterface.h
    Include/Atom/RHI/DeviceDispatchRaysIndirectBuffer.h
    Include/Atom/RHI/DispatchRaysIndirectBuffer.h
)
