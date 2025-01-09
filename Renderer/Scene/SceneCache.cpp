#include "RendererPCH.h"

#include "SceneCache.h"

#include "ResourceTable.h"
#include "Render/Helpers/GPUStructs.h"

namespace
{
    constexpr uint32_t MAX_LIGHTS_NUM = 64;
}

namespace SceneLayer
{
    SceneCache::SceneCache()
    {
        dx12::HeapDescription texturesHeapDesc;
        {
            texturesHeapDesc.SetHeapType(D3D12_HEAP_TYPE_DEFAULT);
            texturesHeapDesc.SetHeapFlags(D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES);
            texturesHeapDesc.SetSize(_256MB * 4);
            texturesHeapDesc.SetMemoryPoolPreference(D3D12_MEMORY_POOL_UNKNOWN);
            texturesHeapDesc.SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY_UNKNOWN);
            texturesHeapDesc.SetVisibleNodeMask(1);
            texturesHeapDesc.SetCreationNodeMask(1);
        }

        dx12::DescriptorHeapDescription texturesDescriptorHeapDesc;
        {
            texturesDescriptorHeapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            texturesDescriptorHeapDesc.SetNumDescriptors(64);
            texturesDescriptorHeapDesc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
            texturesDescriptorHeapDesc.SetNodeMask(1);
        }

        _texturesTable = std::make_shared<dx12::ResourceTable>(texturesDescriptorHeapDesc, texturesHeapDesc);

        dx12::HeapDescription lightsHeapDesc;
        {
            lightsHeapDesc.SetHeapType(D3D12_HEAP_TYPE_UPLOAD);
            lightsHeapDesc.SetHeapFlags(D3D12_HEAP_FLAG_NONE);
            lightsHeapDesc.SetSize(_4MB);
            lightsHeapDesc.SetMemoryPoolPreference(D3D12_MEMORY_POOL_UNKNOWN);
            lightsHeapDesc.SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY_UNKNOWN);
            lightsHeapDesc.SetVisibleNodeMask(1);
            lightsHeapDesc.SetCreationNodeMask(1);
        }

        dx12::DescriptorHeapDescription lightsDescriptorHeapDesc;
        {
            lightsDescriptorHeapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            lightsDescriptorHeapDesc.SetNumDescriptors(1);
            lightsDescriptorHeapDesc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        }

        _lightsTable = std::make_shared<dx12::ResourceTable>(lightsDescriptorHeapDesc, lightsHeapDesc);
    }

    std::shared_ptr<dx12::ResourceTable> SceneCache::GetTextureTable() const
    {
        return _texturesTable;
    }

    std::shared_ptr<dx12::ResourceTable> SceneCache::GetLightsTable() const
    {
        return _lightsTable;
    }

    void SceneCache::SetTime(float time)
    {
        _currentTime = time;
    }

    float SceneCache::GetTime() const
    {
        return _currentTime;
    }
} // namespace SceneLayer
