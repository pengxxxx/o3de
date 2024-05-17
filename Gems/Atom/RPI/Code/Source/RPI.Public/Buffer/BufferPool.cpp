/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */


#include <Atom/RPI.Public/Buffer/BufferPool.h>
#include <Atom/RPI.Reflect/ResourcePoolAsset.h>

#include <Atom/RHI/Factory.h>
#include <Atom/RHI/SingleDeviceBufferPool.h>

#include <Atom/RHI.Reflect/BufferPoolDescriptor.h>

#include <AtomCore/Instance/InstanceDatabase.h>

namespace AZ
{
    namespace RPI
    {
        Data::Instance<BufferPool> BufferPool::FindOrCreate(const Data::Asset<ResourcePoolAsset>& resourcePoolAsset)
        {
            return Data::InstanceDatabase<BufferPool>::Instance().FindOrCreate(
                Data::InstanceId::CreateFromAsset(resourcePoolAsset),
                resourcePoolAsset);
        }

        Data::Instance<BufferPool> BufferPool::CreateInternal(RHI::Device& device, ResourcePoolAsset& poolAsset)
        {
            Data::Instance<BufferPool> bufferPool = aznew BufferPool();
            RHI::ResultCode resultCode = bufferPool->Init(device, poolAsset);
            if (resultCode == RHI::ResultCode::Success)
            {
                return bufferPool;
            }

            return nullptr;
        }

        RHI::ResultCode BufferPool::Init(RHI::Device& device, ResourcePoolAsset& poolAsset)
        {
            RHI::Ptr<RHI::SingleDeviceBufferPool> bufferPool = RHI::Factory::Get().CreateBufferPool();
            if (!bufferPool)
            {
                AZ_Error("RPI::BufferPool", false, "Failed to create RHI::SingleDeviceBufferPool");
                return RHI::ResultCode::Fail;
            }

            RHI::BufferPoolDescriptor *desc = azrtti_cast<RHI::BufferPoolDescriptor*>(poolAsset.GetPoolDescriptor().get());
            if (!desc)
            {
                AZ_Error("RPI::BufferPool", false, "The resource pool asset does not contain a buffer pool descriptor.");
                return RHI::ResultCode::Fail;
            }

            bufferPool->SetName(AZ::Name{ poolAsset.GetPoolName() });
            RHI::ResultCode resultCode = bufferPool->Init(device, *desc);
            if (resultCode == RHI::ResultCode::Success)
            {
                m_pool = bufferPool;
            }

            return resultCode;
        }

        const RHI::SingleDeviceBufferPool* BufferPool::GetRHIPool() const
        {
            return m_pool.get();
        }

        RHI::SingleDeviceBufferPool* BufferPool::GetRHIPool()
        {
            return m_pool.get();
        }
    }
}
