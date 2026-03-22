#pragma once

#include "Descriptor.h"

namespace rhi::d3d12
{
    D3D12_CPU_DESCRIPTOR_HANDLE ToD3D12Handle(rhi::CPUDescriptor& descriptor);
    rhi::CPUDescriptor ToRHIHandle(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);
    D3D12_GPU_DESCRIPTOR_HANDLE ToD3D12Handle(rhi::GPUDescriptor& descriptor);
    rhi::GPUDescriptor ToRHIHandle(D3D12_GPU_DESCRIPTOR_HANDLE descriptor);
} // namespace rhi::d3d12
