#pragma once

#include "RHI/Buffer.h"
#include "RHI/Texture.h"
#include "Renderer/Core/DescriptorHeapManager.h"
#include "RenderGraph/Interfaces.h"

class ResourceTable : public rg::IDescriptorProvider
{
public:
    ResourceTable(const ResourceTable&) = delete;
    ResourceTable(ResourceTable&&) noexcept = default;
    ~ResourceTable() = default;

    ResourceTable& operator=(const ResourceTable&) = delete;
    ResourceTable& operator=(ResourceTable&&) noexcept = default;

    static void Create(rhi::Device* device);
    static void Destroy();

    static ResourceTable& Get();

    void Reset();
    void ResetTransientResources();

    rhi::DescriptorHeap* GetShaderResourcesDescriptorHeap() const;

    void CreateStaticResourceView(rhi::ResourceID resourceID, rhi::ResourceViewType viewType) override;
    void CreateTransientResourceView(rhi::ResourceID resourceID, rhi::ResourceViewType viewType);

    std::uint32_t GetBindlessIndex(rhi::ResourceID resourceID, rhi::ResourceViewType viewType) override;

    rhi::CPUDescriptor GetDescriptor(rhi::ResourceID resourceID, rhi::ResourceViewType viewType) override;

private:
    ResourceTable(rhi::Device* device);

    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticBufferCBVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticBufferSRVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticBufferUAVs;

    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticTextureRTVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticTextureDSVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticTextureSRVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticTextureUAVs;

    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientBufferCBVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientBufferSRVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientBufferUAVs;

    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientTextureRTVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientTextureDSVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientTextureSRVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientTextureUAVs;

    rhi::Device* _device;

    static std::unique_ptr<ResourceTable> _instance;
};
