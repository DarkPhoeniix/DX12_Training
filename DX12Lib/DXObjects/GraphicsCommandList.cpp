#include "stdafx.h"

#include "GraphicsCommandList.h"

#include "DXObjects/Resource.h"
#include "DXObjects/RootSignature.h"
#include "Scene/Viewport.h"

namespace Core
{
    GraphicsCommandList::GraphicsCommandList()
        : _commandList(nullptr)
    {
    }

    GraphicsCommandList::GraphicsCommandList(ComPtr<ID3D12GraphicsCommnadList2> DXCommandList)
        : _commandList(DXCommandList)
    {
    }

    GraphicsCommandList::~GraphicsCommandList()
    {
        _commandList = nullptr;
    }

    D3D12_COMMAND_LIST_TYPE Core::GraphicsCommandList::GetCommandListType() const
    {
        return _commandList->GetType();
    }

    ComPtr<ID3D12GraphicsCommandList2> Core::GraphicsCommandList::GetDXCommandList() const
    {
        return _commandList;
    }

    void Core::GraphicsCommandList::TransitionBarrier(const std::shared_ptr<Resource>& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource, bool flushBarriers)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource->GetDXResource().Get(), resource->GetCurrentState(), stateAfter, subresource);
        _commandList->ResourceBarrier(1, &barrier);
    }

    void Core::GraphicsCommandList::AliasingBarrier(const std::shared_ptr<Resource>& beforeResource, const std::shared_ptr<Resource>& afterResource, bool flushBarriers)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(beforeResource->GetDXResource().Get(), afterResource->GetDXResource().Get());
        _commandList->ResourceBarrier(1, &barrier);
    }

    void Core::GraphicsCommandList::CopyResource(const std::shared_ptr<Resource>& dstRes, const std::shared_ptr<Resource>& srcRes)
    {
        TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_COPY_DEST);
        TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_COPY_SOURCE);

        _commandList->CopyResource(dstRes->GetDXResource().Get(), srcRes->GetDXResource().Get());
    }

    void Core::GraphicsCommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
    {
        _commandList->IASetPrimitiveTopology(primitiveTopology);
    }

    void Core::GraphicsCommandList::SetVertexBuffer(uint32_t slot, const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
    {
        _commandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
    }

    void Core::GraphicsCommandList::SetViewport(const Viewport& viewport)
    {
        CD3DX12_VIEWPORT vp = viewport.GetDXViewport();
        CD3DX12_RECT scissorRect = viewport.GetScissorRectangle();
        _commandList->RSSetViewports(1, &vp);
        _commandList->RSSetScissorRects(1, &scissorRect);
    }

    void Core::GraphicsCommandList::SetPipelineState(const std::shared_ptr<RootSignature>& rootSignature)
    {
        _commandList->SetPipelineState(rootSignature->GetPipelineState().Get());
    }

    void Core::GraphicsCommandList::SetGraphicsRootSignature(const std::shared_ptr<RootSignature>& rootSignature)
    {
        _commandList->SetGraphicsRootSignature(rootSignature->GetRootSignature().Get());
    }

    void Core::GraphicsCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
    {
        _commandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
    }

    void Core::GraphicsCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex, int32_t baseVertex, uint32_t startInstance)
    {
        _commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
    }
} // namespace Core
