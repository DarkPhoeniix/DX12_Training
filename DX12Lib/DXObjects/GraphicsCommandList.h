#pragma once

#include "DXObjects/RootSignature.h"

class Viewport;

namespace Core
{
    class GraphicsCommandList
    {
    public:
        GraphicsCommandList();
        GraphicsCommandList(ComPtr<ID3D12GraphicsCommandList> DXCommandList);
        ~GraphicsCommandList();

        D3D12_COMMAND_LIST_TYPE GetCommandListType() const;

        ComPtr<ID3D12GraphicsCommandList> GetDXCommandList() const;
        ComPtr<ID3D12GraphicsCommandList>& GetDXCommandList();

        void TransitionBarrier(Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);
        void AliasingBarrier(const std::shared_ptr<Resource> & = nullptr, const std::shared_ptr<Resource>& afterResource = nullptr, bool flushBarriers = false);

        void CopyResource(Resource& sourceResource, Resource& destinationResource);

        // Input Assembly
        void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology);
        void SetVertexBuffer(uint32_t slot, const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView);
        void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& indexBufferView);

        // Output Merger
        void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetDescriptor, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilDescriptor);

        // Rasterizator State
        void SetViewport(const Viewport& viewport);
        void SetPipelineState(const RootSignature& rootSignature);
        void SetGraphicsRootSignature(const RootSignature& rootSignature);

        void ClearRTV(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const FLOAT color[4], Viewport* viewport = nullptr);
        void ClearDSV(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH, FLOAT depth = 1.0f, UINT8 stencil = 0, Viewport* viewport = nullptr);
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0);
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0, uint32_t startInstance = 0);

        void SetDescriptorHeaps(const std::vector<ID3D12DescriptorHeap*> descriptorHeaps);

        void SetConstant(UINT index, UINT data, UINT offset = 0);
        void SetConstants(UINT index, UINT numValues, const void* data, UINT offset = 0);
        void SetCBV(UINT index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
        void SetSRV(UINT index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
        void SetUAV(UINT index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
        void SetDescriptorTable(UINT index, D3D12_GPU_DESCRIPTOR_HANDLE descriptor);

        void Reset(ID3D12CommandAllocator* commandAllocator, ID3D12PipelineState* pipelineState);
        void Close();

    private:
        ComPtr<ID3D12GraphicsCommandList> _commandList;
    };
} // namespace Core
