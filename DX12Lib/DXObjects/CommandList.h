#pragma once

//class CommandList
//{
//public:
//    D3D12_COMMAND_LIST_TYPE GetCommandListType() const;
//    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetD3D12CommandList() const;
//    void TransitionBarrier(const std::shared_ptr<Resource>& resource, D3D12_RESOURCE_STATES stateAfter,
//        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);
//    void AliasingBarrier(const std::shared_ptr<Resource> & = nullptr,
//        const std::shared_ptr<Resource>& afterResource = nullptr, bool flushBarriers = false);
//    void CopyResource(const std::shared_ptr<Resource>& dstRes, const std::shared_ptr<Resource>& srcRes);
//    void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology);
//    void SetVertexBuffer(uint32_t slot, const std::shared_ptr<VertexBuffer>& vertexBufferView);
//    void SetViewport(const D3D12_VIEWPORT& viewport);
//    void SetScissorRect(const D3D12_RECT& scissorRect);
//    void SetPipelineState(const std::shared_ptr<PipelineStateObject>& pipelineState);
//    void SetGraphicsRootSignature(const std::shared_ptr<RootSignature>& rootSignature);
//    void SetComputeRootSignature(const std::shared_ptr<RootSignature>& rootSignature);
//    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0);
//    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0,
//        uint32_t startInstance = 0);
//    void Dispatch(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1);
//
//private:
//    ComPtr<ID3D12CommandList> _commandList;
//};
