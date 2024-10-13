#include "stdafx.h"

#include "SceneCache.h"

namespace SceneLayer
{
    SceneCache::SceneCache()
    {
        Core::HeapDescription tableHeapDesc;
        {
            tableHeapDesc.SetHeapType(D3D12_HEAP_TYPE_DEFAULT);
            tableHeapDesc.SetHeapFlags(D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES);
            tableHeapDesc.SetSize(_256MB);
            tableHeapDesc.SetMemoryPoolPreference(D3D12_MEMORY_POOL_UNKNOWN);
            tableHeapDesc.SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY_UNKNOWN);
            tableHeapDesc.SetVisibleNodeMask(1);
            tableHeapDesc.SetCreationNodeMask(1);
        }

        Core::DescriptorHeapDescription tableDescriptorHeapDesc;
        {
            tableDescriptorHeapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            tableDescriptorHeapDesc.SetNumDescriptors(32);
            tableDescriptorHeapDesc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
            tableDescriptorHeapDesc.SetNodeMask(1);
        }

        _texturesTable = std::make_shared<Core::ResourceTable>(tableDescriptorHeapDesc, tableHeapDesc);
    }

    SceneCache::~SceneCache()
    {
        _camera = nullptr;
    }

    std::shared_ptr<Core::ResourceTable> SceneCache::GetTextureTable() const
    {
        return _texturesTable;
    }

    void SceneCache::SetCamera(Camera* camera)
    {
        _camera = camera;
    }

    Camera* SceneCache::GetCamera() const
    {
        return _camera;
    }

    LightManager* SceneCache::GetLightManager()
    {
        return &_lightManager;
    }
} // namespace SceneLayer
