/*
 * Copyright 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_SLSI_GRALLOC2WRAPPER_H
#define ANDROID_SLSI_GRALLOC2WRAPPER_H

#include <string>

#include <android/hardware/graphics/allocator/2.0/IAllocator.h>
#include <android/hardware/graphics/allocator/4.0/IAllocator.h>
#include <android/hardware/graphics/mapper/2.0/IMapper.h>
#include <android/hardware/graphics/mapper/4.0/IMapper.h>
#include <utils/StrongPointer.h>

namespace android {

namespace GrallocWrapper {

using hardware::graphics::common::V1_0::BufferUsage;
using hardware::graphics::common::V1_0::PixelFormat;
using hardware::graphics::allocator::V4_0::IAllocator;
using hardware::graphics::mapper::V2_0::YCbCrLayout;
using hardware::graphics::mapper::V4_0::BufferDescriptor;
using hardware::graphics::mapper::V4_0::Error;
using hardware::graphics::mapper::V4_0::IMapper;

using namespace hardware::graphics::mapper;

// A wrapper to IMapper
class Mapper {
public:
    Mapper();

    Error createDescriptor(
            const V2_0::IMapper::BufferDescriptorInfo& descriptorInfo,
            V2_0::BufferDescriptor* outDescriptor) const;

    // Import a buffer that is from another HAL, another process, or is
    // cloned.
    //
    // The returned handle must be freed with freeBuffer.
    Error importBuffer(const hardware::hidl_handle& rawHandle,
            buffer_handle_t* outBufferHandle) const;

    void freeBuffer(buffer_handle_t bufferHandle) const;

    // The ownership of acquireFence is always transferred to the callee, even
    // on errors.
    Error lock(buffer_handle_t bufferHandle, uint64_t usage,
            const V2_0::IMapper::Rect& accessRegion,
            int acquireFence, void** outData) const;

    // The ownership of acquireFence is always transferred to the callee, even
    // on errors.
    Error lock(buffer_handle_t bufferHandle, uint64_t usage,
            const V2_0::IMapper::Rect& accessRegion,
            int acquireFence, YCbCrLayout* outLayout) const;

    // unlock returns a fence sync object (or -1) and the fence sync object is
    // owned by the caller
    int unlock(buffer_handle_t bufferHandle) const;

private:
    sp<IMapper> mMapper;
};

// A wrapper to IAllocator
class Allocator {
public:
    // An allocator relies on a mapper, and that mapper must be alive at all
    // time.
    Allocator(const Mapper& mapper);

    std::string dumpDebugInfo() const;

    /*
     * The returned buffers are already imported and must not be imported
     * again.  outBufferHandles must point to a space that can contain at
     * least "count" buffer_handle_t.
     */
    Error allocate(V2_0::BufferDescriptor descriptor, uint32_t count,
            uint32_t* outStride, buffer_handle_t* outBufferHandles) const;

    Error allocate(V2_0::BufferDescriptor descriptor,
            uint32_t* outStride, buffer_handle_t* outBufferHandle) const
    {
        return allocate(descriptor, 1, outStride, outBufferHandle);
    }

    Error allocate(const V2_0::IMapper::BufferDescriptorInfo& descriptorInfo, uint32_t count,
            uint32_t* outStride, buffer_handle_t* outBufferHandles) const
    {
        V2_0::BufferDescriptor descriptor;
        Error error = mMapper.createDescriptor(descriptorInfo, &descriptor);
        if (error == Error::NONE) {
            error = allocate(descriptor, count, outStride, outBufferHandles);
        }
        return error;
    }

    Error allocate(const V2_0::IMapper::BufferDescriptorInfo& descriptorInfo,
            uint32_t* outStride, buffer_handle_t* outBufferHandle) const
    {
        return allocate(descriptorInfo, 1, outStride, outBufferHandle);
    }

private:
    const Mapper& mMapper;
    sp<IAllocator> mAllocator;
};

} // namespace GrallocWrapper

} // namespace android

#endif
