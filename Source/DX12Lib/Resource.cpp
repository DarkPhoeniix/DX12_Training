#include "DX12LibPCH.h"

#include "Resource.h"

namespace dx12
{
    D3D12_RESOURCE_STATES GetResourceState(ResourceState state)
    {
        D3D12_RESOURCE_STATES d3dState = D3D12_RESOURCE_STATE_COMMON;

        if (HasFlag(state, ResourceState::Common))                  d3dState |= D3D12_RESOURCE_STATE_COMMON;
        if (HasFlag(state, ResourceState::VertexAndConstantBuffer)) d3dState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        if (HasFlag(state, ResourceState::IndexBuffer))             d3dState |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        if (HasFlag(state, ResourceState::RenderTarget))            d3dState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        if (HasFlag(state, ResourceState::UnorderedAccess))         d3dState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        if (HasFlag(state, ResourceState::DepthWrite))              d3dState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
        if (HasFlag(state, ResourceState::DepthRead))               d3dState |= D3D12_RESOURCE_STATE_DEPTH_READ;
        if (HasFlag(state, ResourceState::AllShaderResource))    d3dState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (HasFlag(state, ResourceState::NonPixelShaderResource))  d3dState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (HasFlag(state, ResourceState::PixelShaderResource))     d3dState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        if (HasFlag(state, ResourceState::CopyDest))                d3dState |= D3D12_RESOURCE_STATE_COPY_DEST;
        if (HasFlag(state, ResourceState::CopySource))              d3dState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        if (HasFlag(state, ResourceState::ResolveDest))             d3dState |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
        if (HasFlag(state, ResourceState::ResolveSource))           d3dState |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
        if (HasFlag(state, ResourceState::IndirectArgument))        d3dState |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        if (HasFlag(state, ResourceState::Present))                 d3dState |= D3D12_RESOURCE_STATE_PRESENT;

        return d3dState;
    }

    D3D12_BARRIER_SYNC GetSyncFlags(ResourceState state)
    {
        D3D12_BARRIER_SYNC flags = D3D12_BARRIER_SYNC_NONE;

        if (HasFlag(state, ResourceState::VertexAndConstantBuffer)) flags |= D3D12_BARRIER_SYNC_ALL_SHADING;
        //if (HasFlag(state, ResourceState::IndexBuffer))             flags |= D3D12_BARRIER_SYNC_INDEX_INPUT;
        if (HasFlag(state, ResourceState::RenderTarget))            flags |= D3D12_BARRIER_SYNC_RENDER_TARGET;
        if (HasFlag(state, ResourceState::UnorderedAccess))         flags |= D3D12_BARRIER_SYNC_ALL_SHADING;
        if (HasAnyFlag(state, ResourceState::AllDSV))               flags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
        if (HasFlag(state, ResourceState::NonPixelShaderResource))  flags |= D3D12_BARRIER_SYNC_NON_PIXEL_SHADING;
        if (HasFlag(state, ResourceState::PixelShaderResource))     flags |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
        if (HasAnyFlag(state, ResourceState::AllCopy))              flags |= D3D12_BARRIER_SYNC_COPY;
        if (HasAnyFlag(state, ResourceState::AllResolve))           flags |= D3D12_BARRIER_SYNC_RESOLVE;
        if (HasFlag(state, ResourceState::IndirectArgument))        flags |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
        if (HasFlag(state, ResourceState::Split))                   flags |= D3D12_BARRIER_SYNC_SPLIT;
        if (flags == D3D12_BARRIER_SYNC_NONE)                       flags = D3D12_BARRIER_SYNC_ALL; // Fallback to ALL if no specific sync is set

        return flags;
    }

    D3D12_BARRIER_ACCESS GetAccessFlags(ResourceState state)
    {
        D3D12_BARRIER_ACCESS flags = D3D12_BARRIER_ACCESS_COMMON;

        if (HasFlag(state, ResourceState::Common))                  flags |= D3D12_BARRIER_ACCESS_COMMON;
        //if (HasFlag(state, ResourceState::VertexAndConstantBuffer)) flags |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER | D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
        //if (HasFlag(state, ResourceState::IndexBuffer))             flags |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
        if (HasFlag(state, ResourceState::RenderTarget))            flags |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
        if (HasFlag(state, ResourceState::UnorderedAccess))         flags |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
        if (HasFlag(state, ResourceState::DepthWrite))              flags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
        if (HasFlag(state, ResourceState::DepthRead))               flags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
        if (HasAnyFlag(state, ResourceState::AllShaderResource))    flags |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
        if (HasFlag(state, ResourceState::CopyDest))                flags |= D3D12_BARRIER_ACCESS_COPY_DEST;
        if (HasFlag(state, ResourceState::CopySource))              flags |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
        if (HasFlag(state, ResourceState::IndirectArgument))        flags |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
        if (HasFlag(state, ResourceState::ResolveDest))             flags |= D3D12_BARRIER_ACCESS_RESOLVE_DEST;
        if (HasFlag(state, ResourceState::ResolveSource))           flags |= D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;

        return flags;
    }

    D3D12_BARRIER_LAYOUT GetLayout(ResourceState state)
    {
        // TODO: it would be better to use queue-specific layouts here

        if (HasFlag(state, ResourceState::RenderTarget))            return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
        if (HasFlag(state, ResourceState::UnorderedAccess))         return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
        if (HasFlag(state, ResourceState::DepthWrite))              return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
        if (HasFlag(state, ResourceState::DepthRead))               return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
        if (HasAnyFlag(state, ResourceState::AllShaderResource))    return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
        if (HasFlag(state, ResourceState::CopyDest))                return D3D12_BARRIER_LAYOUT_COPY_DEST;
        if (HasFlag(state, ResourceState::CopySource))              return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
        if (HasFlag(state, ResourceState::ResolveDest))             return D3D12_BARRIER_LAYOUT_RESOLVE_DEST;
        if (HasFlag(state, ResourceState::ResolveSource))           return D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
        if (HasFlag(state, ResourceState::Present))                 return D3D12_BARRIER_LAYOUT_PRESENT;

        FAIL("Unsupported resource state for layout conversion: %u", static_cast<std::uint32_t>(state));
        return D3D12_BARRIER_LAYOUT_UNDEFINED;
    }

    Resource::Resource(const std::string& name)
        : _ID(InvalidResourceID)
        , _resource(nullptr)
        , _resourceDesc()
        , _currentState(ResourceState::Common)
        , _initialState(ResourceState::Common)
        , _allocationInfo()
        , _uavCounterOffset(std::uint32_t(-1))
        , _name(name)
    {
    }

    Resource::Resource(const std::string& name, const ResourceDescription& resourceDesc)
        : _ID(InvalidResourceID)
        , _resource(nullptr)
        , _resourceDesc(resourceDesc)
        , _currentState(ResourceState::Common)
        , _initialState(ResourceState::Common)
        , _allocationInfo()
        , _uavCounterOffset(std::uint32_t(-1))
        , _name(name)
    {
    }

    Resource::Resource(const std::string& name, ComPtr<ID3D12Resource> resource)
        : _ID(InvalidResourceID)
        , _resource(resource)
        , _resourceDesc(resource->GetDesc())
        , _currentState(ResourceState::Common)
        , _initialState(ResourceState::Common)
        , _allocationInfo()
        , _uavCounterOffset(std::uint32_t(-1))
    {
        SetName(name);
    }

    Resource::Resource(const Resource& other)
        : _ID(other._ID)
        , _resource(other._resource)
        , _resourceDesc(other._resourceDesc)
        , _currentState(other._currentState)
        , _initialState(other._initialState)
        , _allocationInfo(other._allocationInfo)
        , _uavCounterOffset(other._uavCounterOffset)
    {
    }

    Resource::Resource(Resource&& other) noexcept
        : _ID(other._ID)
        , _resource(std::move(other._resource))
        , _resourceDesc(std::move(other._resourceDesc))
        , _currentState(other._currentState)
        , _initialState(other._initialState)
        , _allocationInfo(other._allocationInfo)
        , _uavCounterOffset(other._uavCounterOffset)
    {
    }

    Resource::~Resource()
    {
        _resource = nullptr;
    }

    Resource& Resource::operator=(const Resource& other)
    {
        if (this != &other)
        {
            _resource = other._resource;
            _resourceDesc = other._resourceDesc;
            _currentState = other._currentState;
            _initialState = other._initialState;
            _allocationInfo = other._allocationInfo;
            _uavCounterOffset = other._uavCounterOffset;
        }

        return *this;
    }

    Resource& Resource::operator=(Resource&& other) noexcept
    {
        if (this != &other)
        {
            _resource = std::move(other._resource);
            _resourceDesc = std::move(other._resourceDesc);
            _currentState = other._currentState;
            _initialState = other._initialState;
            _allocationInfo = other._allocationInfo;
            _uavCounterOffset = other._uavCounterOffset;
        }

        return *this;
    }

    ComPtr<ID3D12Resource> Resource::GetDXResource() const
    {
        return _resource;
    }

    ComPtr<ID3D12Resource>& Resource::GetDXResource()
    {
        return _resource;
    }

    const ResourceID& Resource::GetID() const
    {
        return _ID;
    }

    void Resource::SetName(const std::string& name)
    {
        _name = name;
        if (_resource)
        {
            std::wstring tmp(_name.begin(), _name.end());
            _resource->SetName(tmp.c_str());
        }
    }

    const std::string& Resource::GetName() const
    {
        return _name;
    }

    void Resource::SetResourceDescription(const ResourceDescription& resourceDesc)
    {
        _resourceDesc = resourceDesc;
    }

    ResourceDescription Resource::GetResourceDescription() const
    {
        return _resourceDesc;
    }

    [[nodiscard]] ResourceState Resource::GetInitialState() const
    {
        return _initialState;
    }

    void Resource::SetCurrentState(ResourceState state)
    {
        _currentState = state;
    }

    ResourceState Resource::GetCurrentState() const
    {
        return _currentState;
    }

    D3D12_RESOURCE_ALLOCATION_INFO Resource::GetAllocationInfo() const
    {
        D3D12_RESOURCE_DESC desc = _resourceDesc.CreateDXResourceDescription();
        return dx12::Device::GetDXDevice()->GetResourceAllocationInfo(0, 1, &desc);
    }

    void Resource::Unmap()
    {
        D3D12_RANGE range;
        range.Begin = 0;
        range.End = 0;

        _resource->Unmap(0, &range);
    }

    void Resource::Reset()
    {
        ASSERT(_resource, "Trying to reset an nullptr resource.");
        _resource.Reset();
    }

    D3D12_GPU_VIRTUAL_ADDRESS Resource::OffsetGPU(std::uint64_t offset) const
    {
        ASSERT(_resource, "Trying to get GPU pointer for an nullptr resource.");

        D3D12_GPU_VIRTUAL_ADDRESS result = _resource->GetGPUVirtualAddress();
        result += offset;

        return result;
    }

    ComPtr<ID3D12Resource> Resource::CreateCommitedResource(ResourceState initialState)
    {
        _initialState = initialState;
        _currentState = _initialState;

        D3D12_HEAP_PROPERTIES heapDesc = {};
        {
            memset(&heapDesc, 0, sizeof(D3D12_HEAP_PROPERTIES));

            heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapDesc.CreationNodeMask = 1;
            heapDesc.VisibleNodeMask = 1;

            if (_resourceDesc.IsType(ResourceType::Dynamic))
            {
                heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
                _initialState = ResourceState::GenericRead;
                _currentState = _initialState;
            }
            else if (_resourceDesc.IsType(ResourceType::ReadBack))
            {
                heapDesc.Type = D3D12_HEAP_TYPE_READBACK;
            }
            else
            {
                heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
            }
        }

        // need to RTT and DSV
        D3D12_RESOURCE_DESC resourceDesc = _resourceDesc.CreateDXResourceDescription();
        D3D12_CLEAR_VALUE* clearValue = _resourceDesc.GetClearValue().get();
        HRESULT result = dx12::Device::GetDXDevice()->CreateCommittedResource(
            &heapDesc,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            GetResourceState(_initialState),
            clearValue,
            IID_PPV_ARGS(&_resource));
        CHECK(result, "Failed to create committed resource.");

        std::wstring temp(_name.begin(), _name.end());
        _resource->SetName(temp.c_str());

        return _resource;
    }

    ComPtr<ID3D12Resource> Resource::CreatePlacedResource(ComPtr<ID3D12Heap> heap, std::uint64_t offset, ResourceState initialState)
    {
        _initialState = initialState;
        _currentState = _initialState;

        D3D12_RESOURCE_DESC resourceDesc = _resourceDesc.CreateDXResourceDescription();
        D3D12_CLEAR_VALUE* clearValue = _resourceDesc.GetClearValue().get();

        HRESULT result = dx12::Device::GetDXDevice()->CreatePlacedResource(
            heap.Get(),
            offset,
            &resourceDesc,
            GetResourceState(_initialState),
            clearValue,
            IID_PPV_ARGS(&_resource));
        CHECK(result, "Failed to create placed resource.");

        std::wstring temp(_name.begin(), _name.end());
        _resource->SetName(temp.c_str());

        return _resource;
    }

    RenderTargetView Resource::GetAsRTV()
    {
        RenderTargetView view = {};

        view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        view.Texture2D.MipSlice = 0;
        view.Owner = shared_from_this();

        return view;
    }

    DepthStencilView Resource::GetAsDSV()
    {
        DepthStencilView view = {};

        if (_resourceDesc.GetDepthOrArraySize() == 6)
        {
            view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            view.Texture2DArray.ArraySize = _resourceDesc.GetDepthOrArraySize();
        }
        else
        {
            view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        }
        view.Texture2D.MipSlice = 0;
        view.Owner = shared_from_this();

        return view;
    }

    ConstantBufferView Resource::GetAsCBV()
    {
        ConstantBufferView view = {};

        // TODO: add CBV to resource
        view.Owner = shared_from_this();
        view.BufferLocation = this->OffsetGPU();
        view.SizeInBytes = _resourceDesc.GetSize().x * _resourceDesc.GetSize().y;

        return view;
    }

    ShaderResourceView Resource::GetAsSRV()
    {
        ShaderResourceView view = {};

        view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        view.Format = _resourceDesc.GetFormat();
        view.Owner = shared_from_this();

        ResourceType type = _resourceDesc.GetResourceType();
        if ((type & ResourceType::Buffer) != ResourceType::None)
        {
            view.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            view.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            view.Buffer.FirstElement = 0;
            view.Buffer.StructureByteStride = _resourceDesc.GetStride();
            view.Buffer.NumElements = _resourceDesc.GetSize().x / _resourceDesc.GetStride();
        }
        else if ((type & ResourceType::Texture) != ResourceType::None)
        {
            if (_resourceDesc.GetDepthOrArraySize() == 6)
            {
                view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                view.TextureCube.MipLevels = _resourceDesc.GetMipLevels();
                view.TextureCube.MostDetailedMip = 0;
                view.TextureCube.ResourceMinLODClamp = 0.0f;
            }
            else if (_resourceDesc.GetDepthOrArraySize() > 1)
            {
                view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                view.Texture2DArray.ArraySize = _resourceDesc.GetDepthOrArraySize();
                view.Texture2DArray.MipLevels = _resourceDesc.GetMipLevels();
                view.Texture2DArray.FirstArraySlice = 0;
                view.Texture2DArray.MostDetailedMip = 0;
                view.Texture2DArray.PlaneSlice = 0;
                view.Texture2DArray.ResourceMinLODClamp = 0.0f;
            }
            else
            {
                view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                view.Texture2D.MipLevels = _resourceDesc.GetMipLevels();
                view.Texture2D.MostDetailedMip = 0;
                view.Texture2D.PlaneSlice = 0;
                view.Texture2D.ResourceMinLODClamp = 0.0f;
            }
        }

        if ((type & ResourceType::DepthStencil) != ResourceType::None)
        {
            view.Format = DXGI_FORMAT_R32_FLOAT;
        }

        return view;
    }

    UnorderedAccessView Resource::GetAsUAV()
    {
        UnorderedAccessView view = {};

        view.Format = _resourceDesc.GetFormat();
        view.Owner = shared_from_this();

        ResourceType type = _resourceDesc.GetResourceType();
        if ((type & ResourceType::Buffer) != ResourceType::None)
        {
            view.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            view.Buffer.StructureByteStride = _resourceDesc.GetStride();
            view.Buffer.NumElements = _resourceDesc.GetSize().x / _resourceDesc.GetStride();
            if (_resourceDesc.GetUAVCounterOffset() != -1)
            {
                view.Buffer.CounterOffsetInBytes = _resourceDesc.GetUAVCounterOffset();
            }
        }
        else if ((type & ResourceType::Texture) != ResourceType::None)
        {
            if (_resourceDesc.GetDepthOrArraySize() == 1)
            {
                view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                view.Texture2D.MipSlice = 0;
                view.Texture2D.PlaneSlice = 0;
            }
            else
            {
                view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                view.Texture2DArray.ArraySize = _resourceDesc.GetDepthOrArraySize();
                view.Texture2DArray.FirstArraySlice = 0;
                view.Texture2DArray.MipSlice = 0;
                view.Texture2DArray.PlaneSlice = 0;
            }
        }

        return view;
    }
} // namespace dx12
