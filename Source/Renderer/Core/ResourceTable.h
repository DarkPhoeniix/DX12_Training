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

    void CreateStaticResourceView(std::shared_ptr<rhi::Buffer> buffer, rhi::ResourceViewType viewType) override;
    void CreateStaticResourceView(std::shared_ptr<rhi::Texture> texture, rhi::ResourceViewType viewType) override;
    void CreateTransientResourceView(std::shared_ptr<rhi::Buffer> buffer, rhi::ResourceViewType viewType);
    void CreateTransientResourceView(std::shared_ptr<rhi::Texture> texture, rhi::ResourceViewType viewType);

    std::uint32_t GetBindlessIndex(rhi::ResourceID resourceID, rhi::ResourceViewType viewType) override;

    rhi::CPUDescriptor GetDescriptor(rhi::ResourceID resourceID, rhi::ResourceViewType viewType) override;

private:
    ResourceTable(rhi::Device* device);

    std::unordered_map<rhi::ResourceID, DescriptorHandle>& GetStaticResourceMap(rhi::ResourceViewType viewType);
    std::unordered_map<rhi::ResourceID, DescriptorHandle>& GetTransientResourceMap(rhi::ResourceViewType viewType);
    DescriptorHandle FindHandle(rhi::ResourceID resourceID, rhi::ResourceViewType viewType);

    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticResourceRTVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticResourceDSVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticResourceCBVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticResourceSRVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _staticResourceUAVs;

    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientResourceRTVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientResourceDSVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientResourceCBVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientResourceSRVs;
    std::unordered_map<rhi::ResourceID, DescriptorHandle> _transientResourceUAVs;

    rhi::Device* _device;

    static std::unique_ptr<ResourceTable> _instance;
};
