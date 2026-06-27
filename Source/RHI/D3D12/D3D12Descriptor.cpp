
#include "RHI_PCH.h"

#include "D3D12Descriptor.h"

namespace rhi::d3d12
{
    D3D12_CPU_DESCRIPTOR_HANDLE ToD3D12Handle(const CPUDescriptor& descriptor)
    {
        return D3D12_CPU_DESCRIPTOR_HANDLE{ descriptor.ptr };
    }

    CPUDescriptor ToRHIHandle(D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        CPUDescriptor result;
        result.ptr = descriptor.ptr;

        return result;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE ToD3D12Handle(const GPUDescriptor& descriptor)
    {
        return D3D12_GPU_DESCRIPTOR_HANDLE{ descriptor.ptr };
    }

    GPUDescriptor ToRHIHandle(D3D12_GPU_DESCRIPTOR_HANDLE descriptor)
    {
        GPUDescriptor result;
        result.ptr = descriptor.ptr;

        return result;
    }
} // namespace rhi::d3d12
