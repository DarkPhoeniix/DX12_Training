#pragma once

#include "ResourceDescription.h"

namespace dx12
{
    struct RenderTargetView;
    struct DepthStencilView;
    struct ConstantBufferView;
    struct ShaderResourceView;
    struct UnorderedAccessView;

    class Resource
    {
    public:
        Resource();
        Resource(ResourceDescription resourceDesc);
        virtual ~Resource();

        void InitFromDXResource(ComPtr<ID3D12Resource> resource);
        ComPtr<ID3D12Resource> GetDXResource() const;
        ComPtr<ID3D12Resource>& GetDXResource();

        void SetName(const std::string& name);
        std::string GetName() const;

        void SetResourceDescription(const ResourceDescription& resourceDesc);
        ResourceDescription GetResourceDescription() const;

        void SetCurrentState(D3D12_RESOURCE_STATES state);
        D3D12_RESOURCE_STATES GetCurrentState() const;

        const D3D12_RESOURCE_ALLOCATION_INFO& GetAllocationInfo() const;

        D3D12_GPU_VIRTUAL_ADDRESS OffsetGPU(unsigned int offset) const;
        void* Map();
        void* Map(uint32_t offset, uint32_t end);

        void Reset();

        ComPtr<ID3D12Resource> CreateCommitedResource(D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST);
        ComPtr<ID3D12Resource> CreateCommitedResource(const ResourceDescription& resourceDesc, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST);
        ComPtr<ID3D12Resource> CreatePlacedResource(ComPtr<ID3D12Heap> heap, unsigned int offset, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST);
        ComPtr<ID3D12Resource> CreatePlacedResource(const ResourceDescription& resourceDesc, ComPtr<ID3D12Heap> heap, unsigned int offset, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST);

        RenderTargetView GetAsRTV();
        DepthStencilView GetAsDSV();
        ConstantBufferView GetAsCBV();
        ShaderResourceView GetAsSRV();
        UnorderedAccessView GetAsUAV();

    protected:
        ComPtr<ID3D12Resource> _resource;
        std::string _name;

        ResourceDescription _resourceDesc;

        D3D12_RESOURCE_STATES _initialState;
        D3D12_RESOURCE_STATES _currentState;

        D3D12_RESOURCE_ALLOCATION_INFO _allocationInfo;
    };

    struct RenderTargetView : public D3D12_RENDER_TARGET_VIEW_DESC
    {
        Resource* Owner;
    };

    struct DepthStencilView : public D3D12_DEPTH_STENCIL_VIEW_DESC
    {
        Resource* Owner;
    };

    struct ConstantBufferView : public D3D12_CONSTANT_BUFFER_VIEW_DESC
    {
        Resource* Owner;
    };

    struct ShaderResourceView : public D3D12_SHADER_RESOURCE_VIEW_DESC
    {
        Resource* Owner;
    };

    struct UnorderedAccessView : public D3D12_UNORDERED_ACCESS_VIEW_DESC
    {
        Resource* Owner;
    };
} // namespace dx12
