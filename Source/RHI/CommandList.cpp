
#include "RHI_PCH.h"

#include "CommandList.h"

#include "PipelineState.h"
#include "ResourceBarrier.h"

namespace
{
    dx12::CommandListType GetCmdListType(D3D12_COMMAND_LIST_TYPE commandListType)
    {
        if (commandListType == D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            return dx12::CommandListType::Graphics;
        }
        else if (commandListType == D3D12_COMMAND_LIST_TYPE_COMPUTE)
        {
            return dx12::CommandListType::Compute;
        }
        else if (commandListType == D3D12_COMMAND_LIST_TYPE_COPY)
        {
            return dx12::CommandListType::Copy;
        }

        LOG_CRITICAL("Unsupported command list type.");
        return dx12::CommandListType::Unknown;
    }
} // namespace unnamed

namespace dx12
{
    CommandList::CommandList()
        : _commandList(nullptr)
        , _type(CommandListType::Unknown)
    {
    }

    CommandList::CommandList(ComPtr<ID3D12GraphicsCommandList7> DXCommandList)
        : _commandList(DXCommandList)
        , _type(GetCmdListType(DXCommandList->GetType()))
    {
    }

    CommandList::CommandList(const CommandList& other)
        : _commandList(other._commandList)
        , _type(other._type)
        , _name(other._name)
    {
    }

    CommandList::CommandList(CommandList&& other) noexcept
        : _commandList(other._commandList)
        , _type(other._type)
        , _name(other._name)
    {
        if (this != &other)
        {
            other._commandList = nullptr;
        }
    }

    CommandList::~CommandList()
    {
        _commandList = nullptr;
    }

    CommandList& CommandList::operator=(const CommandList& other)
    {
        if (this != &other)
        {
            _commandList = other._commandList;
            _type = other._type;
            _name = other._name;
        }

        return *this;
    }

    CommandList& CommandList::operator=(CommandList&& other) noexcept
    {
        if (this != &other)
        {
            _commandList = other._commandList;
            _type = other._type;
            _name = other._name;

            other._commandList = nullptr;
        }

        return *this;
    }

    CommandListType CommandList::GetCommandListType() const
    {
        return _type;
    }

    void CommandList::SetDXCommandList(ComPtr<ID3D12GraphicsCommandList7> commandList)
    {
        _commandList = commandList;
        _type = GetCmdListType(commandList->GetType());
    }

    ComPtr<ID3D12GraphicsCommandList7> CommandList::GetDXCommandList() const
    {
        return _commandList;
    }

    ComPtr<ID3D12GraphicsCommandList7>& CommandList::GetDXCommandList()
    {
        return _commandList;
    }

    void CommandList::SetPredication(std::shared_ptr<Resource> buffer, std::uint64_t offset, D3D12_PREDICATION_OP operation)
    {
        _commandList->SetPredication(buffer ? buffer->GetDXResource().Get() : nullptr, offset, operation);
    }

    void CommandList::BeginQuery(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, std::uint32_t index)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->BeginQuery(queryHeap.Get(), type, index);
    }

    void CommandList::ResolveQueryData(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, std::uint32_t index, std::shared_ptr<Resource> destination, std::uint64_t offset)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->ResolveQueryData(queryHeap.Get(), type, index, 1, destination->GetDXResource().Get(), offset);
    }

    void CommandList::ResolveQueryData(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, std::uint32_t index, std::uint32_t numQueries, std::shared_ptr<Resource> destination, std::uint64_t offset)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->ResolveQueryData(queryHeap.Get(), type, index, numQueries, destination->GetDXResource().Get(), offset);
    }

    void CommandList::EndQuery(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, std::uint32_t index)
    {
        FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics.");
        FAIL(queryHeap, "Query heap is null.");
        
        _commandList->EndQuery(queryHeap.Get(), type, index);
    }

    void CommandList::TransitionBarrier(const ResourceBarrier& barrier)
    {
        FAIL(!barrier.TargetResource.expired(), "Resource is null.");
        ASSERT(barrier.BeforeState != barrier.AfterState, "BeforeState and AfterState are the same.");

        if (std::shared_ptr<Resource> resource = barrier.TargetResource.lock())
        {
            if (Device::IsEnhancedBarriersSupported())
            {
                if ((resource->GetResourceDescription().GetResourceType() & ResourceType::Texture) != ResourceType::None)
                {
                    CD3DX12_TEXTURE_BARRIER textureBarrier(
                        GetSyncFlags(barrier.BeforeState),
                        GetSyncFlags(barrier.AfterState),
                        GetAccessFlags(barrier.BeforeState),
                        GetAccessFlags(barrier.AfterState),
                        GetLayout(barrier.BeforeState),
                        GetLayout(barrier.AfterState),
                        resource->GetDXResource().Get(),
                        CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),       // All subresources
                        D3D12_TEXTURE_BARRIER_FLAG_NONE);

                    D3D12_BARRIER_GROUP barrierGroup[] = { CD3DX12_BARRIER_GROUP(1, &textureBarrier) };

                    _commandList->Barrier(1, barrierGroup);
                }
                else
                {
                    CD3DX12_BUFFER_BARRIER bufferBarrier(
                        GetSyncFlags(barrier.BeforeState),
                        GetSyncFlags(barrier.AfterState),
                        GetAccessFlags(barrier.BeforeState),
                        GetAccessFlags(barrier.AfterState),
                        resource->GetDXResource().Get());

                    D3D12_BARRIER_GROUP barrierGroup[] = { CD3DX12_BARRIER_GROUP(1, &bufferBarrier) };

                    _commandList->Barrier(1, barrierGroup);
                }
            }
            else
            {
                CD3DX12_RESOURCE_BARRIER dxBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                    resource->GetDXResource().Get(),
                    GetResourceState(barrier.BeforeState),
                    GetResourceState(barrier.AfterState),
                    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

                _commandList->ResourceBarrier(1, &dxBarrier);
            }
            resource->SetCurrentState(barrier.AfterState);
        }
    }

    void CommandList::TransitionBarriers(const std::vector<ResourceBarrier>& barriers)
    {
        ASSERT(!barriers.empty(), "Barriers vector is empty.");

        std::uint32_t numBarriers = static_cast<std::uint32_t>(barriers.size());

        if (Device::IsEnhancedBarriersSupported())
        {
            std::vector<CD3DX12_TEXTURE_BARRIER> textureBarriers;
            std::vector<CD3DX12_BUFFER_BARRIER> bufferBarriers;

            for (size_t i = 0; i < numBarriers; ++i)
            {
                if (std::shared_ptr<Resource> resource = barriers[i].TargetResource.lock())
                {
                    const ResourceBarrier& barrier = barriers[i];
                    if (HasFlag(resource->GetResourceDescription().GetResourceType(), ResourceType::Texture))
                    {
                        textureBarriers.emplace_back(GetSyncFlags(barrier.BeforeState),
                            GetSyncFlags(barrier.AfterState),
                            GetAccessFlags(barrier.BeforeState),
                            GetAccessFlags(barrier.AfterState),
                            GetLayout(barrier.BeforeState),
                            GetLayout(barrier.AfterState),
                            resource->GetDXResource().Get(),
                            CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),       // All subresources
                            D3D12_TEXTURE_BARRIER_FLAG_NONE);

                        resource->SetCurrentState(barriers[i].AfterState);
                    }
                    else
                    {
                        bufferBarriers.emplace_back(
                            GetSyncFlags(barrier.BeforeState),
                            GetSyncFlags(barrier.AfterState),
                            GetAccessFlags(barrier.BeforeState),
                            GetAccessFlags(barrier.AfterState),
                            resource->GetDXResource().Get());

                        resource->SetCurrentState(barriers[i].AfterState);
                    }
                }
            }

            D3D12_BARRIER_GROUP barrierGroups[] =
            {
                CD3DX12_BARRIER_GROUP(bufferBarriers.size(), bufferBarriers.data()),
                CD3DX12_BARRIER_GROUP(textureBarriers.size(), textureBarriers.data()),
            };

            _commandList->Barrier(2, barrierGroups);
        }
        else
        {
            std::vector<CD3DX12_RESOURCE_BARRIER> dxBarriers(numBarriers);

            for (size_t i = 0; i < numBarriers; ++i)
            {
                ASSERT(!barriers[i].TargetResource.expired(), "Resource in barrier is null.");
                if (std::shared_ptr<Resource> resource = barriers[i].TargetResource.lock())
                {
                    dxBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
                        resource->GetDXResource().Get(),
                        GetResourceState(barriers[i].BeforeState),
                        GetResourceState(barriers[i].AfterState),
                        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
                    resource->SetCurrentState(barriers[i].AfterState);
                }
            }

            _commandList->ResourceBarrier(numBarriers, dxBarriers.data());
        }
    }

    void CommandList::TransitionBarrier(Resource& resource, ResourceState stateAfter, std::uint32_t subresource)
    {
        ASSERT(resource.GetCurrentState() != stateAfter, "Current state and state after are the same.");

        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.GetDXResource().Get(), GetResourceState(resource.GetCurrentState()), GetResourceState(stateAfter), subresource);
        _commandList->ResourceBarrier(1, &barrier);
        resource.SetCurrentState(stateAfter);
    }

    void CommandList::AliasingBarrier(std::shared_ptr<Resource> beforeResource, std::shared_ptr<Resource> afterResource)
    {
        FAIL(beforeResource || afterResource, "Resources are null.");
        ASSERT(beforeResource != afterResource, "Before and after resources are the same.");

        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(beforeResource->GetDXResource().Get(), afterResource->GetDXResource().Get());
        _commandList->ResourceBarrier(1, &barrier);
    }

    void CommandList::UAVBarrier(std::shared_ptr<Resource> resource)
    {
        FAIL(resource, "Resource is null.");

        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(resource->GetDXResource().Get());
        _commandList->ResourceBarrier(1, &barrier);
    }

    void CommandList::CopyResource(Resource& sourceResource, Resource& destinationResource)
    {
        ASSERT(sourceResource.GetDXResource() != destinationResource.GetDXResource(), "Source and destination resources are the same.");

        _commandList->CopyResource(destinationResource.GetDXResource().Get(), sourceResource.GetDXResource().Get());
    }

    void CommandList::CopyBufferRegion(Resource& sourceResource, Resource& destinationResource, uint32_t numBytes, uint32_t sourceOffset, uint32_t destinationOffset)
    {
        ASSERT(sourceResource.GetDXResource() != destinationResource.GetDXResource(), "Source and destination resources are the same.");
        ASSERT(sourceOffset + numBytes <= sourceResource.GetAllocationInfo().SizeInBytes, "Source offset and size exceed source resource size.");
        ASSERT(destinationOffset + numBytes <= destinationResource.GetAllocationInfo().SizeInBytes, "Destination offset and size exceed destination resource size.");

        _commandList->CopyBufferRegion(destinationResource.GetDXResource().Get(), destinationOffset, sourceResource.GetDXResource().Get(), sourceOffset, numBytes);
    }

    void CommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->IASetPrimitiveTopology(primitiveTopology);
    }

    void CommandList::SetVertexBuffer(uint32_t slot, const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
    }

    void CommandList::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& indexBufferView)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->IASetIndexBuffer(&indexBufferView);
    }

    void CommandList::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptor, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
    {
        UINT numTargets = renderTargetDescriptor ? 1 : 0;
        _commandList->OMSetRenderTargets(numTargets, renderTargetDescriptor, FALSE, depthStencilDescriptor);
    }

    void CommandList::SetRenderTargets(const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->OMSetRenderTargets(static_cast<std::uint32_t>(renderTargetDescriptors.size()), renderTargetDescriptors.data(), FALSE, depthStencilDescriptor);
    }

    void CommandList::SetViewport(const CD3DX12_VIEWPORT& viewport, const CD3DX12_RECT& scissorRectangle)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->RSSetViewports(1, &viewport);
        _commandList->RSSetScissorRects(1, &scissorRectangle);
    }

    void CommandList::SetPipelineState(const PipelineState& rootSignature)
    {
        FAIL((_type == CommandListType::Graphics) || (_type == CommandListType::Compute), "Command list type is not Graphics or Compute.")

            _commandList->SetPipelineState(rootSignature.GetPipelineState().Get());

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRootSignature(rootSignature.GetRootSignature().Get());
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRootSignature(rootSignature.GetRootSignature().Get());
        }
    }

    void CommandList::ClearRTV(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float color[4], CD3DX12_RECT* rectangle)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->ClearRenderTargetView(renderTargetView, color, rectangle ? 1 : 0, rectangle);
    }

    void CommandList::ClearDSV(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags, float depth, std::uint8_t stencil, CD3DX12_RECT* rectangle)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, rectangle ? 1 : 0, rectangle);
    }

    void CommandList::Draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t startVertex, std::uint32_t startInstance)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
    }

    void CommandList::DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t startIndex, std::uint32_t baseVertex, std::uint32_t startInstance)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
    }

    void CommandList::Dispatch(std::uint32_t xThreadGroupsCount, std::uint32_t yThreadGroupsCount, std::uint32_t zThreadGroupsCount)
    {
        FAIL(_type == CommandListType::Compute, "Command list type is not Compute.");

        _commandList->Dispatch(xThreadGroupsCount, yThreadGroupsCount, zThreadGroupsCount);
    }

    void CommandList::ExecuteIndirect(const CommandSignature& commandSignature, std::uint32_t maxCommandCount, Resource& argumentBuffer, std::shared_ptr<Resource> countBuffer, std::uint32_t argumentBufferOffset, std::uint32_t countBufferOffset)
    {
        ID3D12Resource* counter = countBuffer ? countBuffer->GetDXResource().Get() : nullptr;
        _commandList->ExecuteIndirect(commandSignature.GetDXCommandSignature().Get(), maxCommandCount, argumentBuffer.GetDXResource().Get(), argumentBufferOffset, counter, countBufferOffset);
    }

    void CommandList::SetDescriptorHeaps(const std::vector<ID3D12DescriptorHeap*> descriptorHeaps)
    {
        ASSERT(descriptorHeaps.size() != 0, "Descriptor heaps vector is empty.");

        _commandList->SetDescriptorHeaps(static_cast<std::uint32_t>(descriptorHeaps.size()), descriptorHeaps.data());
    }

    void CommandList::SetConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset)
    {
        FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRoot32BitConstant(index, data, offset);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRoot32BitConstant(index, data, offset);
        }
    }

    void CommandList::SetConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset)
    {
        FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRoot32BitConstants(index, numValues, data, offset);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRoot32BitConstants(index, numValues, data, offset);
        }
    }

    void CommandList::SetCBV(std::uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRootConstantBufferView(index, bufferLocation);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRootConstantBufferView(index, bufferLocation);
        }
    }

    void CommandList::SetSRV(std::uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRootShaderResourceView(index, bufferLocation);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRootShaderResourceView(index, bufferLocation);
        }
    }

    void CommandList::SetUAV(std::uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRootUnorderedAccessView(index, bufferLocation);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRootUnorderedAccessView(index, bufferLocation);
        }
    }

    void CommandList::SetDescriptorTable(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE descriptor)
    {
        FAIL(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Graphics or Compute.");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRootDescriptorTable(index, descriptor);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRootDescriptorTable(index, descriptor);
        }
    }

    void CommandList::Reset(ID3D12CommandAllocator* commandAllocator, ID3D12PipelineState* pipelineState)
    {
        FAIL(commandAllocator, "Command allocator is null.");

        HRESULT result = _commandList->Reset(commandAllocator, pipelineState);
        CHECK(result, "Failed to reset command list.");
    }

    void CommandList::Close()
    {
        HRESULT result = _commandList->Close();
        CHECK(result, "Failed to close command list.");
    }

    void CommandList::SetName(const std::string& name)
    {
        _name = name;

        if (_commandList)
        {
            std::wstring tmp(_name.begin(), _name.end());
            _commandList->SetName(tmp.c_str());
        }
    }

    std::string CommandList::GetName() const
    {
        return _name;
    }
} // namespace dx12
