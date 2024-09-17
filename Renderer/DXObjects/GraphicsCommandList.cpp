#include "stdafx.h"

#include "GraphicsCommandList.h"

#include "DXObjects/DescriptorHeap.h"
#include "DXObjects/RootSignature.h"
#include "DXObjects/Heap.h"
#include "Scene/Viewport.h"

namespace Core
{
    GraphicsCommandList::GraphicsCommandList()
        : _commandList(nullptr)
    {
    }

    GraphicsCommandList::GraphicsCommandList(ComPtr<ID3D12GraphicsCommandList> DXCommandList)
        : _commandList(DXCommandList)
    {
    }

    GraphicsCommandList::~GraphicsCommandList()
    {
        _commandList = nullptr;
    }

    D3D12_COMMAND_LIST_TYPE GraphicsCommandList::GetCommandListType() const
    {
        return _commandList->GetType();
    }

    ComPtr<ID3D12GraphicsCommandList> GraphicsCommandList::GetDXCommandList() const
    {
        return _commandList;
    }

    ComPtr<ID3D12GraphicsCommandList>& GraphicsCommandList::GetDXCommandList()
    {
        return _commandList;
    }

    void GraphicsCommandList::SetPredication(Resource* buffer, UINT64 offset, D3D12_PREDICATION_OP operation)
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

    void GraphicsCommandList::BeginQuery(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, UINT64 index)
    {
        // TODO: implement QueryHeap class
        _commandList->BeginQuery(queryHeap.Get(), type, index);
    }

    void GraphicsCommandList::ResolveQueryData(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, UINT64 index, Resource& destination, UINT64 offset)
    {
        _commandList->ResolveQueryData(queryHeap.Get(), type, index, 1, destination.GetDXResource().Get(), offset);
    }

    void GraphicsCommandList::EndQuery(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, UINT64 index)
    {
        // TODO: implement QueryHeap class
        _commandList->EndQuery(queryHeap.Get(), type, index);
    }

    void GraphicsCommandList::TransitionBarrier(Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource, bool flushBarriers)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.GetDXResource().Get(), resource.GetCurrentState(), stateAfter, subresource);
        _commandList->ResourceBarrier(1, &barrier);
        resource.SetCurrentState(stateAfter);
    }

    void GraphicsCommandList::AliasingBarrier(const std::shared_ptr<Resource>& beforeResource, const std::shared_ptr<Resource>& afterResource, bool flushBarriers)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(beforeResource->GetDXResource().Get(), afterResource->GetDXResource().Get());
        _commandList->ResourceBarrier(1, &barrier);
    }

    void GraphicsCommandList::CopyResource(Resource& sourceResource, Resource& destinationResource)
    {
        //TransitionBarrier(destinationResource, D3D12_RESOURCE_STATE_COPY_DEST);
        //TransitionBarrier(sourceResource, D3D12_RESOURCE_STATE_COPY_SOURCE);

        _commandList->CopyResource(destinationResource.GetDXResource().Get(), sourceResource.GetDXResource().Get());
    }

    void GraphicsCommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
    {
        _commandList->IASetPrimitiveTopology(primitiveTopology);
    }

    void GraphicsCommandList::SetVertexBuffer(uint32_t slot, const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
    {
        _commandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
    }

    void GraphicsCommandList::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& indexBufferView)
    {
        _commandList->IASetIndexBuffer(&indexBufferView);
    }

    void GraphicsCommandList::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptor, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
    {
        UINT numTargets = renderTargetDescriptor ? 1 : 0;
        _commandList->OMSetRenderTargets(numTargets, renderTargetDescriptor, FALSE, depthStencilDescriptor);
    }

    void GraphicsCommandList::SetViewport(const Viewport& viewport)
    {
        CD3DX12_VIEWPORT vp = viewport.GetDXViewport();
        CD3DX12_RECT scissorRect = viewport.GetScissorRectangle();
        _commandList->RSSetViewports(1, &vp);
        _commandList->RSSetScissorRects(1, &scissorRect);
    }

    void GraphicsCommandList::SetPipelineState(const RootSignature& rootSignature)
    {
        _commandList->SetPipelineState(rootSignature.GetPipelineState().Get());
    }

    void GraphicsCommandList::SetGraphicsRootSignature(const RootSignature& rootSignature)
    {
        _commandList->SetGraphicsRootSignature(rootSignature.GetRootSignature().Get());
    }

    void GraphicsCommandList::ClearRTV(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const FLOAT color[4], Viewport* viewport)
    {
        // TODO: fix this ugly code
        UINT numRects = viewport ? 1 : 0;
        CD3DX12_RECT* rect = nullptr;
        if (viewport)
        {
            *rect = viewport->GetScissorRectangle();
        }
        _commandList->ClearRenderTargetView(renderTargetView, color, numRects, rect);
    }

    void GraphicsCommandList::ClearDSV(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth, UINT8 stencil, Viewport* viewport)
    {
        // TODO: fix this ugly code
        UINT numRects = viewport ? 1 : 0;
        CD3DX12_RECT* rect = nullptr;
        if (viewport)
        {
            *rect = viewport->GetScissorRectangle();
        }
        _commandList->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, numRects, rect);
    }

    void GraphicsCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
    {
        _commandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
    }

    void GraphicsCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex, int32_t baseVertex, uint32_t startInstance)
    {
        _commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
    }

    void GraphicsCommandList::SetDescriptorHeaps(const std::vector<ID3D12DescriptorHeap*> descriptorHeaps)
    {
        _commandList->SetDescriptorHeaps(descriptorHeaps.size(), descriptorHeaps.data());
    }

    void GraphicsCommandList::SetConstant(UINT index, UINT data, UINT offset)
    {   
        _commandList->SetGraphicsRoot32BitConstant(index, data, offset);
    }

    void GraphicsCommandList::SetConstants(UINT index, UINT numValues, const void* data, UINT offset)
    {
        _commandList->SetGraphicsRoot32BitConstants(index, numValues, data, offset);
    }

    void GraphicsCommandList::SetCBV(UINT index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        _commandList->SetGraphicsRootConstantBufferView(index, bufferLocation);
    }

    void GraphicsCommandList::SetSRV(UINT index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        _commandList->SetGraphicsRootShaderResourceView(index, bufferLocation);
    }

    void GraphicsCommandList::SetUAV(UINT index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        _commandList->SetGraphicsRootUnorderedAccessView(index, bufferLocation);
    }

    void GraphicsCommandList::SetDescriptorTable(UINT index, D3D12_GPU_DESCRIPTOR_HANDLE descriptor)
    {
        _commandList->SetGraphicsRootDescriptorTable(index, descriptor);
    }

    void GraphicsCommandList::Reset(ID3D12CommandAllocator* commandAllocator, ID3D12PipelineState* pipelineState)
    {
        _commandList->Reset(commandAllocator, pipelineState);
    }

    void GraphicsCommandList::Close()
    {
        _commandList->Close();
    }
} // namespace Core
