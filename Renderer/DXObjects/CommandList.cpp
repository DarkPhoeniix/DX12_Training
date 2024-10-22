#include "stdafx.h"

#include "CommandList.h"

#include "DXObjects/DescriptorHeap.h"
#include "DXObjects/RootSignature.h"
#include "DXObjects/Heap.h"
#include "Scene/Nodes/Camera/Viewport.h"

namespace
{
    Core::CommandListType GetCmdListType(D3D12_COMMAND_LIST_TYPE commandListType)
    {
        if (commandListType == D3D12_COMMAND_LIST_TYPE_DIRECT) 
        {
            return Core::CommandListType::Graphics;
        }
        else if (commandListType == D3D12_COMMAND_LIST_TYPE_COMPUTE) 
        {
            return Core::CommandListType::Compute;
        }
        else if (commandListType == D3D12_COMMAND_LIST_TYPE_COPY) 
        {
            return Core::CommandListType::Copy;
        }

        return Core::CommandListType::Unknown;
    }
} // namespace unnamed

namespace Core
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

    void CommandList::SetPredication(Resource* buffer, UINT64 offset, D3D12_PREDICATION_OP operation)
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

    void CommandList::BeginQuery(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, UINT64 index)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        _commandList->BeginQuery(queryHeap.Get(), type, index);
    }

    void CommandList::ResolveQueryData(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, UINT64 index, Resource& destination, UINT64 offset)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        _commandList->ResolveQueryData(queryHeap.Get(), type, index, 1, destination.GetDXResource().Get(), offset);
    }

    void CommandList::EndQuery(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, UINT64 index)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        _commandList->EndQuery(queryHeap.Get(), type, index);
    }

    void CommandList::TransitionBarrier(Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource, bool flushBarriers)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.GetDXResource().Get(), resource.GetCurrentState(), stateAfter, subresource);
        _commandList->ResourceBarrier(1, &barrier);
        resource.SetCurrentState(stateAfter);
    }

    void CommandList::AliasingBarrier(const std::shared_ptr<Resource>& beforeResource, const std::shared_ptr<Resource>& afterResource, bool flushBarriers)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(beforeResource->GetDXResource().Get(), afterResource->GetDXResource().Get());
        _commandList->ResourceBarrier(1, &barrier);
    }

    void CommandList::CopyResource(Resource& sourceResource, Resource& destinationResource)
    {
        _commandList->CopyResource(destinationResource.GetDXResource().Get(), sourceResource.GetDXResource().Get());
    }

    void CommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        _commandList->IASetPrimitiveTopology(primitiveTopology);
    }

    void CommandList::SetVertexBuffer(uint32_t slot, const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        _commandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
    }

    void CommandList::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& indexBufferView)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        _commandList->IASetIndexBuffer(&indexBufferView);
    }

    void CommandList::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptor, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        UINT numTargets = renderTargetDescriptor ? 1 : 0;
        _commandList->OMSetRenderTargets(numTargets, renderTargetDescriptor, FALSE, depthStencilDescriptor);
    }

    void CommandList::SetRenderTargets(const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        _commandList->OMSetRenderTargets(renderTargetDescriptors.size(), renderTargetDescriptors.data(), FALSE, depthStencilDescriptor);
    }

    void CommandList::SetViewport(const SceneLayer::Viewport& viewport)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        CD3DX12_VIEWPORT vp = viewport.GetDXViewport();
        CD3DX12_RECT scissorRect = viewport.GetScissorRectangle();
        _commandList->RSSetViewports(1, &vp);
        _commandList->RSSetScissorRects(1, &scissorRect);
    }

    void CommandList::SetPipelineState(const RootSignature& rootSignature)
    {
        ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrond type of the command list");

        _commandList->SetPipelineState(rootSignature.GetPipelineState().Get());
    }

    void CommandList::SetRootSignature(const RootSignature& rootSignature)
    {
        ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrond type of the command list");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRootSignature(rootSignature.GetRootSignature().Get());
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRootSignature(rootSignature.GetRootSignature().Get());
        }

        ASSERT(false, "Wrond command list type for SetGraphicsRootSignature function");
    }

    void CommandList::ClearRTV(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const FLOAT color[4], SceneLayer::Viewport* viewport)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        // TODO: fix this ugly code
        UINT numRects = viewport ? 1 : 0;
        CD3DX12_RECT* rect = nullptr;
        if (viewport)
        {
            *rect = viewport->GetScissorRectangle();
        }
        _commandList->ClearRenderTargetView(renderTargetView, color, numRects, rect);
    }

    void CommandList::ClearDSV(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth, UINT8 stencil, SceneLayer::Viewport* viewport)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        // TODO: fix this ugly code
        UINT numRects = viewport ? 1 : 0;
        CD3DX12_RECT* rect = nullptr;
        if (viewport)
        {
            *rect = viewport->GetScissorRectangle();
        }
        _commandList->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, numRects, rect);
    }

    void CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        _commandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
    }

    void CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex, int32_t baseVertex, uint32_t startInstance)
    {
        ASSERT(_type == CommandListType::Graphics, "Wrond type of the command list");

        _commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
    }

    void CommandList::SetDescriptorHeaps(const std::vector<ID3D12DescriptorHeap*> descriptorHeaps)
    {
        ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrond type of the command list");

        _commandList->SetDescriptorHeaps(descriptorHeaps.size(), descriptorHeaps.data());
    }

    void CommandList::SetConstant(UINT index, UINT data, UINT offset)
    {
        ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrond type of the command list");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRoot32BitConstant(index, data, offset);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRoot32BitConstant(index, data, offset);
        }

        ASSERT(false, "Wrond command list type for SetConstant function");
    }

    void CommandList::SetConstants(UINT index, UINT numValues, const void* data, UINT offset)
    {
        ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrond type of the command list");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRoot32BitConstants(index, numValues, data, offset);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRoot32BitConstants(index, numValues, data, offset);
        }

        ASSERT(false, "Wrond command list type for SetConstants function");
    }

    void CommandList::SetCBV(UINT index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrond type of the command list");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRootConstantBufferView(index, bufferLocation);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRootConstantBufferView(index, bufferLocation);
        }

        ASSERT(false, "Wrond command list type for SetCBV function");
    }

    void CommandList::SetSRV(UINT index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrond type of the command list");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRootShaderResourceView(index, bufferLocation);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRootShaderResourceView(index, bufferLocation);
        }

        ASSERT(false, "Wrond command list type for SetSRV function");
    }

    void CommandList::SetUAV(UINT index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrond type of the command list");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRootUnorderedAccessView(index, bufferLocation);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRootUnorderedAccessView(index, bufferLocation);
        }

        ASSERT(false, "Wrond command list type for SetUAV function");
    }

    void CommandList::SetDescriptorTable(UINT index, D3D12_GPU_DESCRIPTOR_HANDLE descriptor)
    {
        ASSERT(_type == CommandListType::Graphics || _type == CommandListType::Compute, "Wrond type of the command list");

        if (_type == CommandListType::Graphics)
        {
            _commandList->SetGraphicsRootDescriptorTable(index, descriptor);
        }
        else if (_type == CommandListType::Compute)
        {
            _commandList->SetComputeRootDescriptorTable(index, descriptor);
        }

        ASSERT(false, "Wrond command list type for SetDescriptorTable function");
    }

    void CommandList::Reset(ID3D12CommandAllocator* commandAllocator, ID3D12PipelineState* pipelineState)
    {
        _commandList->Reset(commandAllocator, pipelineState);
    }

    void CommandList::Close()
    {
        _commandList->Close();
    }
} // namespace Core
