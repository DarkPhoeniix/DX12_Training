#pragma once

class Resource;
class RootSignature;
class Viewport;

namespace Core
{
    class GraphicsCommandList
    {
    public:
        GraphicsCommandList();
        GraphicsCommandList(ComPtr<ID3D12GraphicsCommnadList2> DXCommandList);
        ~GraphicsCommandList();

        D3D12_COMMAND_LIST_TYPE GetCommandListType() const;

        ComPtr<ID3D12GraphicsCommandList2> GetDXCommandList() const;

        void TransitionBarrier(const std::shared_ptr<Resource>& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);
        void AliasingBarrier(const std::shared_ptr<Resource> & = nullptr, const std::shared_ptr<Resource>& afterResource = nullptr, bool flushBarriers = false);

        void CopyResource(const std::shared_ptr<Resource>& dstRes, const std::shared_ptr<Resource>& srcRes);

        void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology);
        void SetVertexBuffer(uint32_t slot, const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView);
        void SetViewport(const Viewport& viewport);
        void SetPipelineState(const std::shared_ptr<RootSignature>& rootSignature);
        void SetGraphicsRootSignature(const std::shared_ptr<RootSignature>& rootSignature);

        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0);
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0, uint32_t startInstance = 0);

    private:
        ComPtr<ID3D12GraphicsCommandList2> _commandList;
    };
} // namespace Core
