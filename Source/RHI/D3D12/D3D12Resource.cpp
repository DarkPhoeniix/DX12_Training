
#include "RHI_PCH.h"

#include "D3D12Resource.h"

#include "D3D12Helpers.h"

#include "ResourceIdGenerator.h"
#include "Buffer.h"
#include "Texture.h"

// TODO: allocation info

namespace rhi::d3d12
{
    namespace
    {
        D3D12_HEAP_PROPERTIES CreateHeapProperties(ResourceUsage usage)
        {
            D3D12_HEAP_PROPERTIES heapDesc = 
            {
                .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
                .CreationNodeMask = 1,
                .VisibleNodeMask = 1
            };

            switch (usage)
            {
                case ResourceUsage::Upload:
                    heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
                    break;
                case ResourceUsage::GPUUpload:
                    heapDesc.Type = D3D12_HEAP_TYPE_GPU_UPLOAD;
                    break;
                case ResourceUsage::Readback:
                    heapDesc.Type = D3D12_HEAP_TYPE_READBACK;
                    break;
                default:
                    heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
                    break;
            }

            return heapDesc;
        }
    } // namespace unnamed

    D3D12Resource::D3D12Resource(rhi::Device* device, const BufferDescription& description, const void* initialData, const std::string& name)
        : _device(device)
        , _ID(rhi::ResourceIdGenerator::GenerateID())
        , _initialState(rhi::ResourceState::Common)
        , _currentState(rhi::ResourceState::Common)
        , _stride(description.Stride)
        , _uavCounterOffset(-1)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        D3D12_RESOURCE_DESC resourceDesc =
        {
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = description.Size,
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = GetDXGIFormat(description.Format),
            .SampleDesc = { 1, 0 },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = GetD3D12ResourceFlags(description.Flags)
        };

        D3D12_HEAP_PROPERTIES heapDesc = CreateHeapProperties(description.Usage);

        CreateResource(resourceDesc, heapDesc, initialData);
    }

    D3D12Resource::D3D12Resource(rhi::Device* device, const TextureDescription& description, const void* initialData, const std::string& name)
        : _device(device)
        , _ID(rhi::ResourceIdGenerator::GenerateID())
        , _initialState(rhi::ResourceState::Common)
        , _currentState(rhi::ResourceState::Common)
        , _stride(0)
        , _uavCounterOffset(-1)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        D3D12_RESOURCE_DESC resourceDesc =
        {
            .Dimension = GetD3D12ResourceDimension(description.Dimension),
            .Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
            .Width = description.Width,
            .Height = description.Height,
            .DepthOrArraySize = description.DepthOrArraySize,
            .MipLevels = description.MipLevels,
            .Format = GetDXGIFormat(description.Format),
            .SampleDesc = { 1, 0 },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = GetD3D12ResourceFlags(description.Flags)
        };

        D3D12_HEAP_PROPERTIES heapDesc = CreateHeapProperties(description.Usage);

        CreateResource(resourceDesc, heapDesc, initialData);
    }

    D3D12Resource::D3D12Resource(rhi::Device* device, ID3D12Resource* resource, const std::string& name)
        : _device(device)
        , _resource(resource)
        , _description(resource->GetDesc())
        , _ID(rhi::ResourceIdGenerator::GenerateID())
        , _initialState(rhi::ResourceState::Common)
        , _currentState(rhi::ResourceState::Common)
        , _stride(0)
        , _uavCounterOffset(-1)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Resource::D3D12Resource(D3D12Resource&& other) noexcept
    {
        NOT_IMPLEMENTED();
    }

    D3D12Resource& D3D12Resource::operator=(D3D12Resource&& other) noexcept
    {
        NOT_IMPLEMENTED();
        return *this;
    }

    const ResourceID& D3D12Resource::GetID() const
    {
        return _ID;
    }

    void* D3D12Resource::Map(std::uint32_t begin, std::uint32_t end)
    {
        void* data = nullptr;

        D3D12_RANGE range;
        range.Begin = begin;
        range.End = end;
        _resource->Map(0, &range, &data);

        return data;
    }

    void D3D12Resource::Unmap()
    {
        D3D12_RANGE range;
        range.Begin = 0;
        range.End = 0;

        _resource->Unmap(0, &range);
    }

    std::uint64_t D3D12Resource::GetVirtualAddress() const
    {
        return _resource->GetGPUVirtualAddress();
    }

    ResourceState D3D12Resource::GetInitialState() const
    {
        return _initialState;
    }

    ResourceState D3D12Resource::GetCurrentState() const
    {
        return _currentState;
    }

    const AllocationInfo& D3D12Resource::GetAllocationInfo() const
    {
        return _allocationInfo;
    }

    RenderTargetView D3D12Resource::GetAsRTV()
    {
        NOT_IMPLEMENTED();
        RenderTargetView view = {};

        view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        view.Texture2D.MipSlice = 0;
        view.Owner = shared_from_this();

        return view;
    }

    DepthStencilView D3D12Resource::GetAsDSV()
    {
        NOT_IMPLEMENTED();
        DepthStencilView view = {};

        if (_description.DepthOrArraySize == 6)
        {
            view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            view.Texture2DArray.ArraySize = _description.DepthOrArraySize;
        }
        else
        {
            view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        }
        view.Texture2D.MipSlice = 0;
        view.Owner = shared_from_this();

        return view;
    }

    ConstantBufferView D3D12Resource::GetAsCBV()
    {
        NOT_IMPLEMENTED();
        ConstantBufferView view = {};

        // TODO: add CBV to resource
        view.Owner = shared_from_this();
        view.BufferLocation = this->GetVirtualAddress();
        view.SizeInBytes = _description.Width * _description.Height;

        return view;
    }

    ShaderResourceView D3D12Resource::GetAsSRV()
    {
        NOT_IMPLEMENTED();
        ShaderResourceView view = {};

        view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        view.Format = _description.Format;
        view.Owner = shared_from_this();

        if (_description.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            view.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            view.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            view.Buffer.FirstElement = 0;
            view.Buffer.StructureByteStride = _stride;
            view.Buffer.NumElements = _description.Width / _description.Height;
        }
        else
        {
            if (_description.DepthOrArraySize == 6)
            {
                view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                view.TextureCube.MipLevels = _description.MipLevels;
                view.TextureCube.MostDetailedMip = 0;
                view.TextureCube.ResourceMinLODClamp = 0.0f;
            }
            else if (_description.DepthOrArraySize > 1)
            {
                view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                view.Texture2DArray.ArraySize = _description.DepthOrArraySize;
                view.Texture2DArray.MipLevels = _description.MipLevels;
                view.Texture2DArray.FirstArraySlice = 0;
                view.Texture2DArray.MostDetailedMip = 0;
                view.Texture2DArray.PlaneSlice = 0;
                view.Texture2DArray.ResourceMinLODClamp = 0.0f;
            }
            else
            {
                view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                view.Texture2D.MipLevels = _description.MipLevels;
                view.Texture2D.MostDetailedMip = 0;
                view.Texture2D.PlaneSlice = 0;
                view.Texture2D.ResourceMinLODClamp = 0.0f;
            }
        }

        if (_description.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
        {
            view.Format = DXGI_FORMAT_R32_FLOAT;
        }

        return view;
    }

    UnorderedAccessView D3D12Resource::GetAsUAV()
    {
        NOT_IMPLEMENTED();
        UnorderedAccessView view = {};

        view.Format = _description.Format;
        view.Owner = shared_from_this();

        if (_description.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            view.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            view.Buffer.StructureByteStride = _stride;
            view.Buffer.NumElements = _description.Width / _description.Height;
            if (_uavCounterOffset != -1)
            {
                view.Buffer.CounterOffsetInBytes = _uavCounterOffset;
            }
        }
        else
        {
            if (_description.DepthOrArraySize == 1)
            {
                view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                view.Texture2D.MipSlice = 0;
                view.Texture2D.PlaneSlice = 0;
            }
            else
            {
                view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                view.Texture2DArray.ArraySize = _description.DepthOrArraySize;
                view.Texture2DArray.FirstArraySlice = 0;
                view.Texture2DArray.MipSlice = 0;
                view.Texture2DArray.PlaneSlice = 0;
            }
        }

        return view;
    }

    void* D3D12Resource::GetNative() const
    {
        return static_cast<void*>(_resource.Get());
    }

    void D3D12Resource::CreateResource(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_HEAP_PROPERTIES& heapProperties, const void* initialData)
    {
        // TODO: initialData handling (upload heap or staging buffer)

        _currentState = _initialState;

        ID3D12Device* d3d12NativeDevice = static_cast<ID3D12Device*>(_device->GetNative());
        d3d12NativeDevice->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            GetD3D12ResourceState(_initialState),
            nullptr,
            IID_PPV_ARGS(&_resource));

        SetD3D12Name(_resource.Get(), _name);
    }
} // namespace rhi::d3d12
