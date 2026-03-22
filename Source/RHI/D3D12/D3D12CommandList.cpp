
#include "RHI_PCH.h"

#include "D3D12CommandList.h"

#include "Buffer.h"
#include "Texture.h"
#include "QueryHeap.h"
#include "PipelineState.h"

namespace rhi::d3d12
{
    D3D12CommandList::D3D12CommandList(rhi::Device* device, rhi::CommandListType type, const std::string& name)
        : _device(device)
        , _type(type)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES

    {
        ID3D12Device* nativeDevice = D3D12Cast<ID3D12Device>(device->GetNative());

        D3D12_COMMAND_LIST_TYPE d3d12Type = GetD3D12CommandListType(_type);

        nativeDevice->CreateCommandAllocator(d3d12Type, IID_PPV_ARGS(&_commandAllocator));
        nativeDevice->CreateCommandList(0, d3d12Type, _commandAllocator.Get(), nullptr, IID_PPV_ARGS(&_commandList));

        SetD3D12Name(_commandList.Get(), name);
    }

    D3D12CommandList::D3D12CommandList(D3D12CommandList&& other) noexcept
        : _commandList(other._commandList)
        , _type(other._type)
#if ENABLE_DEBUG_NAMES
        , _name(other._name)
#endif // ENABLE_DEBUG_NAMES
    {
        if (this != &other)
        {
            other._commandList = nullptr;
        }
    }
     
    D3D12CommandList::~D3D12CommandList()
    {
    }

    D3D12CommandList& D3D12CommandList::operator=(D3D12CommandList&& other) noexcept
    {
        if (this != &other)
        {
            _commandList = other._commandList;
            _type = other._type;
#if ENABLE_DEBUG_NAMES
            _name = other._name;
#endif // ENABLE_DEBUG_NAMES

            other._commandList = nullptr;
        }

        return *this;
    }

    rhi::CommandListType D3D12CommandList::GetCommandListType() const
    {
        return _type;
    }

    void D3D12CommandList::SetPredication(Buffer& buffer, std::uint64_t offset, PredicationOperation operation)
    {
        ID3D12Resource* nativeResource = D3D12Cast<ID3D12Resource>(buffer.GetNative());

        _commandList->SetPredication(nativeResource, offset, GetD3D12PredicationOp(operation));
    }

    void D3D12CommandList::BeginQuery(QueryHeap& queryHeap, QueryType type, std::uint32_t index)
    {
        ID3D12QueryHeap* nativeQueryHeap = D3D12Cast<ID3D12QueryHeap>(queryHeap.GetNative());

        _commandList->BeginQuery(nativeQueryHeap, GetD3D12QueryType(type), index);
    }

    void D3D12CommandList::ResolveQueryData(QueryHeap& queryHeap, QueryType type, std::uint32_t index, std::shared_ptr<Buffer> destination, std::uint64_t offset)
    {
        ID3D12QueryHeap* nativeQueryHeap = D3D12Cast<ID3D12QueryHeap>(queryHeap.GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destination->GetNative());

        _commandList->ResolveQueryData(nativeQueryHeap, GetD3D12QueryType(type), index, 1, nativeDestination, offset);
    }

    void D3D12CommandList::ResolveQueryData(QueryHeap& queryHeap, QueryType type, std::uint32_t index, std::uint32_t numQueries, std::shared_ptr<Buffer> destination, std::uint64_t offset)
    {
        ID3D12QueryHeap* nativeQueryHeap = D3D12Cast<ID3D12QueryHeap>(queryHeap.GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destination->GetNative());

        _commandList->ResolveQueryData(nativeQueryHeap, GetD3D12QueryType(type), index, numQueries, nativeDestination, offset);
    }

    void D3D12CommandList::EndQuery(QueryHeap& queryHeap, QueryType type, std::uint32_t index)
    {
        ID3D12QueryHeap* nativeQueryHeap = D3D12Cast<ID3D12QueryHeap>(queryHeap.GetNative());

        _commandList->EndQuery(nativeQueryHeap, GetD3D12QueryType(type), index);
    }

    void D3D12CommandList::TransitionBarriers(const std::vector<BufferBarrier>& barrier)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::UAVBarrier(const rhi::BufferBarrier& barrier)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::UAVBarrier(const rhi::TextureBarrier& barrier)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::CopyBuffer(Buffer& sourceResource, Buffer& destinationResource)
    {
        ID3D12Resource* nativeSource = D3D12Cast<ID3D12Resource>(sourceResource.GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destinationResource.GetNative());

        _commandList->CopyResource(nativeDestination, nativeSource);
    }

    void D3D12CommandList::CopyBufferRegion(Buffer& sourceResource, Buffer& destinationResource, uint32_t numBytes, uint32_t sourceOffset, uint32_t destinationOffset)
    {
        ID3D12Resource* nativeSource = D3D12Cast<ID3D12Resource>(sourceResource.GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destinationResource.GetNative());

        _commandList->CopyBufferRegion(nativeDestination, destinationOffset, nativeSource, sourceOffset, numBytes);
    }

    void D3D12CommandList::CopyTexture(Texture& sourceResource, Texture& destinationResource)
    {
        ID3D12Resource* nativeSource = D3D12Cast<ID3D12Resource>(sourceResource.GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destinationResource.GetNative());

        _commandList->CopyResource(nativeDestination, nativeSource);
    }

    void D3D12CommandList::CopyTextureRegion(Texture& sourceResource, Texture& destinationResource, uint32_t numBytes, uint32_t sourceOffset, uint32_t destinationOffset)
    {
        ID3D12Resource* nativeSource = D3D12Cast<ID3D12Resource>(sourceResource.GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destinationResource.GetNative());

        _commandList->CopyBufferRegion(nativeDestination, destinationOffset, nativeSource, sourceOffset, numBytes);
    }

    void D3D12CommandList::SetPipelineState(const rhi::PipelineState& pipelineState)
    {
        FAIL((_type == CommandListType::Graphics) || (_type == CommandListType::Compute), "Command list type is not Graphics or Compute.");

        ID3D12PipelineState* nativePipelineState = D3D12Cast<ID3D12PipelineState>(pipelineState.GetNative());

        _commandList->SetPipelineState(nativePipelineState);

        NOT_IMPLEMENTED();
        // TODO: set root signature
        //if (_type == CommandListType::Graphics)
        //{
        //    _commandList->SetGraphicsRootSignature(pipelineState.GetRootSignature().Get());
        //}
        //else if (_type == CommandListType::Compute)
        //{
        //    _commandList->SetComputeRootSignature(pipelineState.GetRootSignature().Get());
        //}
    }

    void D3D12CommandList::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
    {
        _commandList->IASetPrimitiveTopology(GetD3D12PrimitiveTopology(primitiveTopology));
    }

    void D3D12CommandList::SetVertexBuffer(std::uint32_t slot, const VertexBufferView& vertexBufferView)
    {
        D3D12_VERTEX_BUFFER_VIEW vbv =
        {
            .BufferLocation = vertexBufferView.BufferLocation,
            .SizeInBytes = vertexBufferView.SizeInBytes,
            .StrideInBytes = vertexBufferView.StrideInBytes
        };
        _commandList->IASetVertexBuffers(slot, 1, &vbv);
    }

    void D3D12CommandList::SetIndexBuffer(const IndexBufferView& indexBufferView)
    {
        D3D12_INDEX_BUFFER_VIEW ibv =
        {
            .BufferLocation = indexBufferView.BufferLocation,
            .SizeInBytes = indexBufferView.SizeInBytes,
            .Format = GetDXGIFormat(indexBufferView.Format)
        };
        _commandList->IASetIndexBuffer(&ibv);
    }

    void D3D12CommandList::SetRenderTarget(rhi::CPUDescriptor* renderTargetDescriptor, rhi::CPUDescriptor* depthStencilDescriptor)
    {
        std::uint32_t numTargets = renderTargetDescriptor ? 1 : 0;
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::SetRenderTargets(const std::vector<rhi::CPUDescriptor> renderTargetDescriptors, rhi::CPUDescriptor* depthStencilDescriptor)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::SetViewport(const Viewport& viewport, const ScissorRect& scissorRectangle)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::ClearRTV(rhi::CPUDescriptor renderTargetView, const float color[4], ScissorRect* rectangle)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::ClearDSV(rhi::CPUDescriptor depthStencilView, rhi::ClearFlags clearFlags, float depth, std::uint8_t stencil, ScissorRect* rectangle)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::Draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t startVertex, std::uint32_t startInstance)
    {
        FAIL(_type == rhi::CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
    }

    void D3D12CommandList::DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t startIndex, std::uint32_t baseVertex, std::uint32_t startInstance)
    {
        FAIL(_type == rhi::CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
    }

    void D3D12CommandList::Dispatch(std::uint32_t xThreadGroupsCount, std::uint32_t yThreadGroupsCount, std::uint32_t zThreadGroupsCount)
    {
        FAIL(_type == rhi::CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Compute.");

        _commandList->Dispatch(xThreadGroupsCount, yThreadGroupsCount, zThreadGroupsCount);
    }

    void D3D12CommandList::ExecuteIndirect(const rhi::CommandSignature& commandSignature, std::uint32_t maxCommandCount, std::shared_ptr<Buffer> argumentBuffer, std::shared_ptr<Buffer> countBuffer, std::uint32_t argumentBufferOffset, std::uint32_t countBufferOffset)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::SetDescriptorHeaps(const std::vector<rhi::DescriptorHeap>& descriptorHeaps)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::SetConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset)
    {
        switch (_type)
        {
        case rhi::CommandListType::Graphics:
            _commandList->SetGraphicsRoot32BitConstant(index, data, offset);
            break;
        case rhi::CommandListType::Compute:
            _commandList->SetComputeRoot32BitConstant(index, data, offset);
            break;
        default:
            UNREACHABLE("Unsupported command list type.");
            break;
        }
    }

    void D3D12CommandList::SetConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset)
    {
        switch (_type)
        {
        case rhi::CommandListType::Graphics:
            _commandList->SetGraphicsRoot32BitConstants(index, numValues, data, offset);
            break;
        case rhi::CommandListType::Compute:
            _commandList->SetComputeRoot32BitConstants(index, numValues, data, offset);
            break;
        default:
            UNREACHABLE("Unsupported command list type.");
            break;
        }
    }

    void D3D12CommandList::SetCBV(std::uint32_t index, std::uint64_t bufferLocation)
    {
        switch (_type)
        {
        case rhi::CommandListType::Graphics:
            _commandList->SetGraphicsRootConstantBufferView(index, bufferLocation);
            break;
        case rhi::CommandListType::Compute:
            _commandList->SetComputeRootConstantBufferView(index, bufferLocation);
            break;
        default:
            UNREACHABLE("Unsupported command list type.");
            break;
        }
    }

    void D3D12CommandList::SetSRV(std::uint32_t index, std::uint64_t bufferLocation)
    {
        switch (_type)
        {
        case rhi::CommandListType::Graphics:
            _commandList->SetGraphicsRootShaderResourceView(index, bufferLocation);
            break;
        case rhi::CommandListType::Compute:
            _commandList->SetComputeRootShaderResourceView(index, bufferLocation);
            break;
        default:
            UNREACHABLE("Unsupported command list type.");
            break;
        }
    }

    void D3D12CommandList::SetUAV(std::uint32_t index, std::uint64_t bufferLocation)
    {
        switch (_type)
        {
        case rhi::CommandListType::Graphics:
            _commandList->SetGraphicsRootUnorderedAccessView(index, bufferLocation);
            break;
        case rhi::CommandListType::Compute:
            _commandList->SetComputeRootUnorderedAccessView(index, bufferLocation);
            break;
        default:
            UNREACHABLE("Unsupported command list type.");
            break;
        }
    }

    void D3D12CommandList::SetDescriptorTable(std::uint32_t index, rhi::GPUDescriptor descriptor)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::Reset(CommandAllocator& commandAllocator, rhi::PipelineState* pipelineState)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12CommandList::Close()
    {
        HRESULT result = _commandList->Close();
        CHECK(result, "Failed to close command list.");
    }

    void* D3D12CommandList::GetNative() const
    {
        return static_cast<void*>(_commandList.Get());
    }

    //void D3D12CommandList::TransitionBarrier(const ResourceBarrier& barrier)
    //{
    //    FAIL(!barrier.TargetResource.expired(), "Resource is null.");
    //    ASSERT(barrier.BeforeState != barrier.AfterState, "BeforeState and AfterState are the same.");

    //    if (std::shared_ptr<Resource> resource = barrier.TargetResource.lock())
    //    {
    //        if (D3D12Device::IsEnhancedBarriersSupported())
    //        {
    //            if ((resource->GetResourceDescription().GetResourceType() & ResourceType::Texture) != ResourceType::None)
    //            {
    //                CD3DX12_TEXTURE_BARRIER textureBarrier(
    //                    GetSyncFlags(barrier.BeforeState),
    //                    GetSyncFlags(barrier.AfterState),
    //                    GetAccessFlags(barrier.BeforeState),
    //                    GetAccessFlags(barrier.AfterState),
    //                    GetLayout(barrier.BeforeState),
    //                    GetLayout(barrier.AfterState),
    //                    resource->GetDXResource().Get(),
    //                    CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),       // All subresources
    //                    D3D12_TEXTURE_BARRIER_FLAG_NONE);

    //                D3D12_BARRIER_GROUP barrierGroup[] = { CD3DX12_BARRIER_GROUP(1, &textureBarrier) };

    //                _commandList->Barrier(1, barrierGroup);
    //            }
    //            else
    //            {
    //                CD3DX12_BUFFER_BARRIER bufferBarrier(
    //                    GetSyncFlags(barrier.BeforeState),
    //                    GetSyncFlags(barrier.AfterState),
    //                    GetAccessFlags(barrier.BeforeState),
    //                    GetAccessFlags(barrier.AfterState),
    //                    resource->GetDXResource().Get());

    //                D3D12_BARRIER_GROUP barrierGroup[] = { CD3DX12_BARRIER_GROUP(1, &bufferBarrier) };

    //                _commandList->Barrier(1, barrierGroup);
    //            }
    //        }
    //        else
    //        {
    //            CD3DX12_RESOURCE_BARRIER dxBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
    //                resource->GetDXResource().Get(),
    //                GetResourceState(barrier.BeforeState),
    //                GetResourceState(barrier.AfterState),
    //                D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    //            _commandList->ResourceBarrier(1, &dxBarrier);
    //        }
    //        resource->SetCurrentState(barrier.AfterState);
    //    }
    //}

    //void D3D12CommandList::TransitionBarriers(const std::vector<ResourceBarrier>& barriers)
    //{
    //    ASSERT(!barriers.empty(), "Barriers vector is empty.");

    //    std::uint32_t numBarriers = static_cast<std::uint32_t>(barriers.size());

    //    if (D3D12Device::IsEnhancedBarriersSupported())
    //    {
    //        std::vector<CD3DX12_TEXTURE_BARRIER> textureBarriers;
    //        std::vector<CD3DX12_BUFFER_BARRIER> bufferBarriers;

    //        for (size_t i = 0; i < numBarriers; ++i)
    //        {
    //            if (std::shared_ptr<Resource> resource = barriers[i].TargetResource.lock())
    //            {
    //                const ResourceBarrier& barrier = barriers[i];
    //                if (HasFlag(resource->GetResourceDescription().GetResourceType(), ResourceType::Texture))
    //                {
    //                    textureBarriers.emplace_back(GetSyncFlags(barrier.BeforeState),
    //                        GetSyncFlags(barrier.AfterState),
    //                        GetAccessFlags(barrier.BeforeState),
    //                        GetAccessFlags(barrier.AfterState),
    //                        GetLayout(barrier.BeforeState),
    //                        GetLayout(barrier.AfterState),
    //                        resource->GetDXResource().Get(),
    //                        CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),       // All subresources
    //                        D3D12_TEXTURE_BARRIER_FLAG_NONE);

    //                    resource->SetCurrentState(barriers[i].AfterState);
    //                }
    //                else
    //                {
    //                    bufferBarriers.emplace_back(
    //                        GetSyncFlags(barrier.BeforeState),
    //                        GetSyncFlags(barrier.AfterState),
    //                        GetAccessFlags(barrier.BeforeState),
    //                        GetAccessFlags(barrier.AfterState),
    //                        resource->GetDXResource().Get());

    //                    resource->SetCurrentState(barriers[i].AfterState);
    //                }
    //            }
    //        }

    //        D3D12_BARRIER_GROUP barrierGroups[] =
    //        {
    //            CD3DX12_BARRIER_GROUP(bufferBarriers.size(), bufferBarriers.data()),
    //            CD3DX12_BARRIER_GROUP(textureBarriers.size(), textureBarriers.data()),
    //        };

    //        _commandList->Barrier(2, barrierGroups);
    //    }
    //    else
    //    {
    //        std::vector<CD3DX12_RESOURCE_BARRIER> dxBarriers(numBarriers);

    //        for (size_t i = 0; i < numBarriers; ++i)
    //        {
    //            ASSERT(!barriers[i].TargetResource.expired(), "Resource in barrier is null.");
    //            if (std::shared_ptr<Resource> resource = barriers[i].TargetResource.lock())
    //            {
    //                dxBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
    //                    resource->GetDXResource().Get(),
    //                    GetResourceState(barriers[i].BeforeState),
    //                    GetResourceState(barriers[i].AfterState),
    //                    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    //                resource->SetCurrentState(barriers[i].AfterState);
    //            }
    //        }

    //        _commandList->ResourceBarrier(numBarriers, dxBarriers.data());
    //    }
    //}

    //void D3D12CommandList::TransitionBarrier(Resource& resource, ResourceState stateAfter, std::uint32_t subresource)
    //{
    //    ASSERT(resource.GetCurrentState() != stateAfter, "Current state and state after are the same.");

    //    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.GetDXResource().Get(), GetResourceState(resource.GetCurrentState()), GetResourceState(stateAfter), subresource);
    //    _commandList->ResourceBarrier(1, &barrier);
    //    resource.SetCurrentState(stateAfter);
    //}

    //void D3D12CommandList::AliasingBarrier(std::shared_ptr<Resource> beforeResource, std::shared_ptr<Resource> afterResource)
    //{
    //    FAIL(beforeResource || afterResource, "Resources are null.");
    //    ASSERT(beforeResource != afterResource, "Before and after resources are the same.");

    //    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(beforeResource->GetDXResource().Get(), afterResource->GetDXResource().Get());
    //    _commandList->ResourceBarrier(1, &barrier);
    //}

    //void D3D12CommandList::UAVBarrier(std::shared_ptr<Resource> resource)
    //{
    //    FAIL(resource, "Resource is null.");

    //    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(resource->GetDXResource().Get());
    //    _commandList->ResourceBarrier(1, &barrier);
    //}

    //void D3D12CommandList::CopyResource(Resource& sourceResource, Resource& destinationResource)
    //{
    //    ASSERT(sourceResource.GetDXResource() != destinationResource.GetDXResource(), "Source and destination resources are the same.");

    //    _commandList->CopyResource(destinationResource.GetDXResource().Get(), sourceResource.GetDXResource().Get());
    //}

    //void D3D12CommandList::CopyBufferRegion(Resource& sourceResource, Resource& destinationResource, uint32_t numBytes, uint32_t sourceOffset, uint32_t destinationOffset)
    //{
    //    ASSERT(sourceResource.GetDXResource() != destinationResource.GetDXResource(), "Source and destination resources are the same.");
    //    ASSERT(sourceOffset + numBytes <= sourceResource.GetAllocationInfo().SizeInBytes, "Source offset and size exceed source resource size.");
    //    ASSERT(destinationOffset + numBytes <= destinationResource.GetAllocationInfo().SizeInBytes, "Destination offset and size exceed destination resource size.");

    //    _commandList->CopyBufferRegion(destinationResource.GetDXResource().Get(), destinationOffset, sourceResource.GetDXResource().Get(), sourceOffset, numBytes);
    //}

    //void D3D12CommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
    //{
    //    FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

    //    _commandList->IASetPrimitiveTopology(primitiveTopology);
    //}

    //void D3D12CommandList::SetVertexBuffer(uint32_t slot, const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
    //{
    //    FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

    //    _commandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
    //}

    //void D3D12CommandList::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& indexBufferView)
    //{
    //    FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

    //    _commandList->IASetIndexBuffer(&indexBufferView);
    //}

    //void D3D12CommandList::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptor, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
    //{
    //    UINT numTargets = renderTargetDescriptor ? 1 : 0;
    //    _commandList->OMSetRenderTargets(numTargets, renderTargetDescriptor, FALSE, depthStencilDescriptor);
    //}

    //void D3D12CommandList::SetRenderTargets(const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
    //{
    //    FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

    //    _commandList->OMSetRenderTargets(static_cast<std::uint32_t>(renderTargetDescriptors.size()), renderTargetDescriptors.data(), FALSE, depthStencilDescriptor);
    //}

    //void D3D12CommandList::SetViewport(const CD3DX12_VIEWPORT& viewport, const CD3DX12_RECT& scissorRectangle)
    //{
    //    FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

    //    _commandList->RSSetViewports(1, &viewport);
    //    _commandList->RSSetScissorRects(1, &scissorRectangle);
    //}

    //void D3D12CommandList::ClearRTV(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float color[4], CD3DX12_RECT* rectangle)
    //{
    //    FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

    //    _commandList->ClearRenderTargetView(renderTargetView, color, rectangle ? 1 : 0, rectangle);
    //}

    //void D3D12CommandList::ClearDSV(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags, float depth, std::uint8_t stencil, CD3DX12_RECT* rectangle)
    //{
    //    FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

    //    _commandList->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, rectangle ? 1 : 0, rectangle);
    //}

    //void D3D12CommandList::Draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t startVertex, std::uint32_t startInstance)
    //{
    //    FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

    //    _commandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
    //}

    //void D3D12CommandList::DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t startIndex, std::uint32_t baseVertex, std::uint32_t startInstance)
    //{
    //    FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

    //    _commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
    //}

    //void D3D12CommandList::Dispatch(std::uint32_t xThreadGroupsCount, std::uint32_t yThreadGroupsCount, std::uint32_t zThreadGroupsCount)
    //{
    //    FAIL(_type == CommandListType::Compute, "Command list type is not Compute.");

    //    _commandList->Dispatch(xThreadGroupsCount, yThreadGroupsCount, zThreadGroupsCount);
    //}

    //void D3D12CommandList::ExecuteIndirect(const CommandSignature& commandSignature, std::uint32_t maxCommandCount, Resource& argumentBuffer, std::shared_ptr<Resource> countBuffer, std::uint32_t argumentBufferOffset, std::uint32_t countBufferOffset)
    //{
    //    ID3D12Resource* counter = countBuffer ? countBuffer->GetDXResource().Get() : nullptr;
    //    _commandList->ExecuteIndirect(commandSignature.GetDXCommandSignature().Get(), maxCommandCount, argumentBuffer.GetDXResource().Get(), argumentBufferOffset, counter, countBufferOffset);
    //}

    //void D3D12CommandList::SetDescriptorHeaps(const std::vector<ID3D12DescriptorHeap*> descriptorHeaps)
    //{
    //    ASSERT(descriptorHeaps.size() != 0, "Descriptor heaps vector is empty.");

    //    _commandList->SetDescriptorHeaps(static_cast<std::uint32_t>(descriptorHeaps.size()), descriptorHeaps.data());
    //}

    //void D3D12CommandList::SetConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset)
    //{
    //    FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

    //    if (_type == CommandListType::Graphics)
    //    {
    //        _commandList->SetGraphicsRoot32BitConstant(index, data, offset);
    //    }
    //    else if (_type == CommandListType::Compute)
    //    {
    //        _commandList->SetComputeRoot32BitConstant(index, data, offset);
    //    }
    //}

    //void D3D12CommandList::SetConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset)
    //{
    //    FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

    //    if (_type == CommandListType::Graphics)
    //    {
    //        _commandList->SetGraphicsRoot32BitConstants(index, numValues, data, offset);
    //    }
    //    else if (_type == CommandListType::Compute)
    //    {
    //        _commandList->SetComputeRoot32BitConstants(index, numValues, data, offset);
    //    }
    //}

    //void D3D12CommandList::SetCBV(std::uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    //{
    //    FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

    //    if (_type == CommandListType::Graphics)
    //    {
    //        _commandList->SetGraphicsRootConstantBufferView(index, bufferLocation);
    //    }
    //    else if (_type == CommandListType::Compute)
    //    {
    //        _commandList->SetComputeRootConstantBufferView(index, bufferLocation);
    //    }
    //}

    //void D3D12CommandList::SetSRV(std::uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    //{
    //    FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

    //    if (_type == CommandListType::Graphics)
    //    {
    //        _commandList->SetGraphicsRootShaderResourceView(index, bufferLocation);
    //    }
    //    else if (_type == CommandListType::Compute)
    //    {
    //        _commandList->SetComputeRootShaderResourceView(index, bufferLocation);
    //    }
    //}

    //void D3D12CommandList::SetUAV(std::uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    //{
    //    FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

    //    if (_type == CommandListType::Graphics)
    //    {
    //        _commandList->SetGraphicsRootUnorderedAccessView(index, bufferLocation);
    //    }
    //    else if (_type == CommandListType::Compute)
    //    {
    //        _commandList->SetComputeRootUnorderedAccessView(index, bufferLocation);
    //    }
    //}

    //void D3D12CommandList::SetDescriptorTable(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE descriptor)
    //{
    //    FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

    //    if (_type == CommandListType::Graphics)
    //    {
    //        _commandList->SetGraphicsRootDescriptorTable(index, descriptor);
    //    }
    //    else if (_type == CommandListType::Compute)
    //    {
    //        _commandList->SetComputeRootDescriptorTable(index, descriptor);
    //    }
    //}

    //void D3D12CommandList::Reset(ID3D12CommandAllocator* commandAllocator, ID3D12PipelineState* pipelineState)
    //{
    //    FAIL(commandAllocator, "Command allocator is null.");

    //    HRESULT result = _commandList->Reset(commandAllocator, pipelineState);
    //    CHECK(result, "Failed to reset command list.");
    //}

    //void D3D12CommandList::Close()
    //{
    //    HRESULT result = _commandList->Close();
    //    CHECK(result, "Failed to close command list.");
    //}

    //void D3D12CommandList::SetName(const std::string& name)
    //{
    //    _name = name;

    //    if (_commandList)
    //    {
    //        std::wstring tmp(_name.begin(), _name.end());
    //        _commandList->SetName(tmp.c_str());
    //    }
    //}

    //std::string D3D12CommandList::GetName() const
    //{
    //    return _name;
    //}
} // namespace rhi::d3d12
