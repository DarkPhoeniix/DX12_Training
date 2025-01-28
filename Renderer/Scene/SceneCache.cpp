#include "RendererPCH.h"

#include "SceneCache.h"

#include "ResourceTable.h"

namespace
{
    constexpr uint32_t MAX_LIGHTS_NUM = 64;
}

namespace scene
{
    SceneCache::SceneCache()
    {
        {
            dx12::HeapDescription desc;
            desc.SetHeapType(D3D12_HEAP_TYPE_DEFAULT);
            desc.SetHeapFlags(D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES);
            desc.SetSize(_256MB * 4);
            desc.SetMemoryPoolPreference(D3D12_MEMORY_POOL_UNKNOWN);
            desc.SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY_UNKNOWN);
            desc.SetVisibleNodeMask(1);
            desc.SetCreationNodeMask(1);

            _texturesHeap.Create(desc);
        }

        _texturesTable = std::make_shared<dx12::ResourceTable>();
        _texturesTable->Init(64);

        _lightsTable = std::make_shared<dx12::ResourceTable>();
        _lightsTable->Init(4);
    }

    std::shared_ptr<dx12::ResourceTable> SceneCache::GetTextureTable() const
    {
        return _texturesTable;
    }

    std::shared_ptr<dx12::ResourceTable> SceneCache::GetLightsTable() const
    {
        return _lightsTable;
    }

    dx12::Heap& SceneCache::GetTextureHeap()
    {
        return _texturesHeap;
    }

    void SceneCache::SetTime(float time)
    {
        _currentTime = time;
    }

    float SceneCache::GetTime() const
    {
        return _currentTime;
    }
} // namespace scene
