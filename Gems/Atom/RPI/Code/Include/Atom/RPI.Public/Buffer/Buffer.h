/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Atom/RHI/MultiDeviceBuffer.h>
#include <Atom/RHI/MultiDeviceBufferPool.h>
#include <Atom/RHI/MultiDeviceFence.h>

#include <Atom/RHI.Reflect/AttachmentId.h>
#include <Atom/RHI.Reflect/Base.h>

#include <Atom/RPI.Reflect/Buffer/BufferAsset.h>

#include <AtomCore/Instance/InstanceData.h>

namespace AZ
{
    namespace RHI
    {
        struct BufferMapResponse;
    } // namespace RHI

    namespace RPI
    {
        class BufferPool;

        class Buffer final
            : public Data::InstanceData
        {
            friend class BufferSystem;

        public:
            AZ_INSTANCE_DATA(Buffer, "{389B818E-136A-44A4-8B6C-4766C0DDB644}");
            AZ_CLASS_ALLOCATOR(Buffer, AZ::SystemAllocator);

            ~Buffer() override;

            //! Instantiates or returns an existing buffer instance using its paired asset.
            static Data::Instance<Buffer> FindOrCreate(const Data::Asset<BufferAsset>& bufferAsset);
            
            //! Blocks until a streaming upload has completed (if one is currently in flight).
            void WaitForUpload();

            RHI::MultiDeviceBuffer* GetRHIBuffer();

            const RHI::MultiDeviceBuffer* GetRHIBuffer() const;

            const RHI::MultiDeviceBufferView* GetBufferView() const;

            //! Update buffer's content with sourceData at an offset of bufferByteOffset
            bool UpdateData(const void* sourceData, uint64_t sourceDataSizeInBytes, uint64_t bufferByteOffset = 0);

            //! Reallocate a new block of memory for this buffer. The previous allocated
            //! memory will be discarded once the GPU is done using it. This only works
            //! for buffers with host heap memory level.
            bool Orphan();

            //! Orphan the buffer then update the buffer's content with input sourceData
            //! This function is only used for buffer created in host such as dynamic buffer which content is rewritten every frame
            bool OrphanAndUpdateData(const void* sourceData, uint64_t sourceDataSizeInBytes);

            //! Maps all buffers in the underlying multi-device buffer and returns a vector
            //! with mapped addresses, one per device.
            AZStd::unordered_map<int, void*> Map(size_t byteCount, uint64_t byteOffset);
            void Unmap();

            //! Get attachment id if this buffer is used as scope attachment
            const RHI::AttachmentId& GetAttachmentId() const;
            
            //! Set default buffer view descriptor with specific structure
            template <class structureType>
            void SetAsStructured();

            //! Get view descriptor of this buffer
            const RHI::BufferViewDescriptor& GetBufferViewDescriptor() const;

            //! Get buffer size in bytes
            uint64_t GetBufferSize() const;

            //! Resize the current buffer. The data in the buffer won't be kept
            //! The buffer view descriptor and buffer view will be updated accordingly
            void Resize(uint64_t bufferSize);

        private:
            Buffer();

            // Standard init path from asset data.
            static Data::Instance<Buffer> CreateInternal(BufferAsset& bufferAsset);
            RHI::ResultCode Init(BufferAsset& bufferAsset);
            void InitBufferView();

            RHI::Ptr<RHI::MultiDeviceBuffer> m_rhiBuffer;
            RHI::Ptr<RHI::MultiDeviceBufferView> m_bufferView;
            RHI::MultiDeviceBufferPool* m_rhiBufferPool = nullptr;

            Data::Instance<BufferPool> m_bufferPool;

            // Only held until the streaming upload is complete.
            Data::Asset<BufferAsset> m_bufferAsset;

            // Tracks the streaming upload of the buffer.
            RHI::Ptr<RHI::MultiDeviceFence> m_streamFence;
            AZStd::mutex m_pendingUploadMutex;
            AZStd::atomic_int m_initialUploadCount{0};

            RHI::BufferViewDescriptor m_bufferViewDescriptor;

            RHI::AttachmentId m_attachmentId;
        };

        template <class structureType>
        void Buffer::SetAsStructured()
        {
            m_bufferViewDescriptor = RHI::BufferViewDescriptor::CreateStructured(0,
                aznumeric_cast<uint32_t>(m_rhiBuffer->GetDescriptor().m_byteCount / sizeof(structureType)), sizeof(structureType));
            m_bufferViewDescriptor.m_overrideBindFlags = m_rhiBuffer->GetDescriptor().m_bindFlags;
            InitBufferView();
        }

    } // namespace RPI
} // namespace AZ
