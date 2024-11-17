#include "stdafx.h"

#include "SceneCache.h"

#include "ResourceTable.h"
#include "Render/GPUStructs/GPULightDesc.h"

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
            lightsHeapDesc.SetSize(_32MB);
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

        static uint32_t LIGHTS_NUM = 64;

        dx12::ResourceDescription lightsViewDesc;
        {
            lightsViewDesc.SetResourceType(dx12::EResourceType::Dynamic | dx12::EResourceType::Buffer);
            lightsViewDesc.SetSize({ sizeof(GPULightDesc) * LIGHTS_NUM, 1 });
            lightsViewDesc.SetFormat(DXGI_FORMAT_UNKNOWN);
            lightsViewDesc.SetDepthOrArraySize(1);
        }

        _lightsView.SetResourceDescription(lightsViewDesc);
        _lightsView.CreateCommitedResource(D3D12_RESOURCE_STATE_GENERIC_READ);

        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc;
        {
            SRVDesc.Buffer.FirstElement = 0;
            SRVDesc.Buffer.NumElements = LIGHTS_NUM;
            SRVDesc.Buffer.StructureByteStride = sizeof(GPULightDesc);
            SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        }

        dx12::Device::GetDXDevice()->CreateShaderResourceView(_lightsView.GetDXResource().Get(), &SRVDesc, _lightsTable->GetDescriptorHeap().GetHeapStartCPUHandle());
    }

    SceneCache::~SceneCache()
    {
        _camera = nullptr;
    }

    std::shared_ptr<dx12::ResourceTable> SceneCache::GetTextureTable() const
    {
        return _texturesTable;
    }

    std::shared_ptr<dx12::ResourceTable> SceneCache::GetLightsTable() const
    {
        return _lightsTable;
    }

    dx12::Resource& SceneCache::GetLightsSRV()
    {
        return _lightsView;
    }

    void SceneCache::SetCamera(Camera* camera)
    {
        _camera = camera;
    }

    Camera* SceneCache::GetCamera() const
    {
        return _camera;
    }
} // namespace SceneLayer
