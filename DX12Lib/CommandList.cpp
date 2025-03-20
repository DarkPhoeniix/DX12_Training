#include "DX12LibPCH.h"

#include "CommandList.h"

#include "PipelineState.h"
#include "ResourceBarrier.h"
#include "Scene/Entity/Components/Camera.h"

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

    CommandList::CommandList(ComPtr<ID3D12GraphicsCommandList> DXCommandList)
        : _commandList(DXCommandList)
        , _type(GetCmdListType(DXCommandList->GetType()))
    {
    }

    CommandList::~CommandList()
    {
        _commandList = nullptr;
    }

    CommandListType CommandList::GetCommandListType() const
    {
        return _type;
    }

    void CommandList::SetDXCommandList(ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        _commandList = commandList;
        _type = GetCmdListType(commandList->GetType());
    }

    ComPtr<ID3D12GraphicsCommandList> CommandList::GetDXCommandList() const
    {
        return _commandList;
    }

    ComPtr<ID3D12GraphicsCommandList>& CommandList::GetDXCommandList()
    {
        return _commandList;
    }

    void CommandList::SetPredication(Resource* buffer, std::uint64_t offset, D3D12_PREDICATION_OP operation)
    {
        if (buffer)
        {
            _commandList->SetPredication(buffer->GetDXResource().Get(), offset, operation);
        }
        else
        {
            _commandList->SetPredication(nullptr, offset, operation);
        }
    }

    void CommandList::BeginQuery(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, std::uint32_t index)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        _commandList->BeginQuery(queryHeap.Get(), type, index);
    }

    void CommandList::ResolveQueryData(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, std::uint32_t index, Resource& destination, std::uint64_t offset)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        _commandList->ResolveQueryData(queryHeap.Get(), type, index, 1, destination.GetDXResource().Get(), offset);
    }

    void CommandList::EndQuery(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, std::uint32_t index)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        _commandList->EndQuery(queryHeap.Get(), type, index);
    }

    void CommandList::TransitionBarrier(ResourceBarrier& barrier)
    {
        CD3DX12_RESOURCE_BARRIER dxBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            barrier.Resource->GetDXResource().Get(),
            barrier.BeforeState, 
            barrier.AfterState, 
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
        barrier.Resource->SetCurrentState(barrier.AfterState);

        _commandList->ResourceBarrier(1, &dxBarrier);
    }

    void CommandList::TransitionBarriers(std::vector<ResourceBarrier>& barriers)
    {
        size_t numBarriers = barriers.size();
        std::vector<CD3DX12_RESOURCE_BARRIER> dxBarriers(numBarriers);

        for (size_t i = 0; i < numBarriers; ++i)
        {
            dxBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
                barriers[i].Resource->GetDXResource().Get(),
                barriers[i].BeforeState,
                barriers[i].AfterState,
                D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
            barriers[i].Resource->SetCurrentState(barriers[i].AfterState);
        }

        _commandList->ResourceBarrier(numBarriers, dxBarriers.data());
    }

    void CommandList::TransitionBarrier(Resource& resource, D3D12_RESOURCE_STATES stateAfter, std::uint32_t subresource)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.GetDXResource().Get(), resource.GetCurrentState(), stateAfter, subresource);
        _commandList->ResourceBarrier(1, &barrier);
        resource.SetCurrentState(stateAfter);
    }

    void CommandList::AliasingBarrier(const std::shared_ptr<Resource>& beforeResource, const std::shared_ptr<Resource>& afterResource)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(beforeResource->GetDXResource().Get(), afterResource->GetDXResource().Get());
        _commandList->ResourceBarrier(1, &barrier);
    }

    void CommandList::CopyResource(Resource& sourceResource, Resource& destinationResource)
    {
        _commandList->CopyResource(destinationResource.GetDXResource().Get(), sourceResource.GetDXResource().Get());
    }

    void CommandList::CopyBufferRegion(Resource& sourceResource, Resource& destinationResource, uint32_t numBytes, uint32_t sourceOffset, uint32_t destinationOffset)
    {
        _commandList->CopyBufferRegion(destinationResource.GetDXResource().Get(), destinationOffset, sourceResource.GetDXResource().Get(), sourceOffset, numBytes);
    }

    void CommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        _commandList->IASetPrimitiveTopology(primitiveTopology);
    }

    void CommandList::SetVertexBuffer(uint32_t slot, const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        _commandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
    }

    void CommandList::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& indexBufferView)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        _commandList->IASetIndexBuffer(&indexBufferView);
    }

    void CommandList::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptor, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        UINT numTargets = renderTargetDescriptor ? 1 : 0;
        _commandList->OMSetRenderTargets(numTargets, renderTargetDescriptor, FALSE, depthStencilDescriptor);
    }

    void CommandList::SetRenderTargets(const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        _commandList->OMSetRenderTargets(static_cast<std::uint32_t>(renderTargetDescriptors.size()), renderTargetDescriptors.data(), FALSE, depthStencilDescriptor);
    }

    void CommandList::SetViewport(const scene::Viewport& viewport)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        CD3DX12_VIEWPORT vp = viewport.GetDXViewport();
        CD3DX12_RECT scissorRect = viewport.GetScissorRectangle();
        _commandList->RSSetViewports(1, &vp);
        _commandList->RSSetScissorRects(1, &scissorRect);
    }

    void CommandList::SetPipelineState(const PipelineState& rootSignature)
    {
        if (ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrong type of the command list"))
        {
            return;
        }

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

    void CommandList::ClearRTV(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float color[4], scene::Viewport* viewport)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        std::uint32_t numRectangles = viewport ? 1 : 0;
        CD3DX12_RECT* rectangle = viewport ? &viewport->GetScissorRectangle() : nullptr;

        _commandList->ClearRenderTargetView(renderTargetView, color, numRectangles, rectangle);
    }

    void CommandList::ClearDSV(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags, float depth, std::uint8_t stencil, scene::Viewport* viewport)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        std::uint32_t numRectangles = viewport ? 1 : 0;
        CD3DX12_RECT* rectangle = viewport ? &viewport->GetScissorRectangle() : nullptr;

        _commandList->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, numRectangles, rectangle);
    }

    void CommandList::Draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t startVertex, std::uint32_t startInstance)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        _commandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
    }

    void CommandList::DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t startIndex, std::uint32_t baseVertex, std::uint32_t startInstance)
    {
        if (ASSERT(_type == CommandListType::Graphics, "Wrong type of the command list"))
        {
            return;
        }

        _commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
    }

    void CommandList::Dispatch(std::uint32_t xThreadGroupsCount, std::uint32_t yThreadGroupsCount, std::uint32_t zThreadGroupsCount)
    {
        if (ASSERT(_type == CommandListType::Compute, "Wrong type of the command list"))
        {
            return;
        }

        _commandList->Dispatch(xThreadGroupsCount, yThreadGroupsCount, zThreadGroupsCount);
    }

    void CommandList::ExecuteIndirect(ComPtr<ID3D12CommandSignature> cmdSignature, std::uint32_t maxCommandCount, Resource& argumentBuffer, std::shared_ptr<Resource> countBuffer, std::uint32_t argumentBufferOffset, std::uint32_t countBufferOffset)
    {
        ID3D12Resource* counter = countBuffer ? countBuffer->GetDXResource().Get() : nullptr;
        _commandList->ExecuteIndirect(cmdSignature.Get(), maxCommandCount, argumentBuffer.GetDXResource().Get(), argumentBufferOffset, counter, countBufferOffset);
    }

    void CommandList::SetDescriptorHeaps(const std::vector<ID3D12DescriptorHeap*> descriptorHeaps)
    {
        if (ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrong type of the command list"))
        {
            return;
        }

        _commandList->SetDescriptorHeaps(static_cast<std::uint32_t>(descriptorHeaps.size()), descriptorHeaps.data());
    }

    void CommandList::SetConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset)
    {
        if (ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrong type of the command list"))
        {
            return;
        }

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
        if (ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrong type of the command list"))
        {
            return;
        }

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
        if (ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrong type of the command list"))
        {
            return;
        }

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
        if (ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrong type of the command list"))
        {
            return;
        }

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
        if (ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrong type of the command list"))
        {
            return;
        }

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
        if (ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrong type of the command list"))
        {
            return;
        }

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
        _commandList->Reset(commandAllocator, pipelineState);
    }

    void CommandList::Close()
    {
        helpers::throwIfFailed(_commandList->Close());
    }

    void CommandList::SetName(const std::string& name)
    {
        _name = name;

        std::wstring tmp(_name.begin(), _name.end());
        _commandList->SetName(tmp.c_str());
    }

    std::string CommandList::GetName() const
    {
        return _name;
    }
} // namespace dx12
