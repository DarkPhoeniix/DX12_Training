#include "stdafx.h"

#include "Skybox.h"

#include "CommandList.h"

namespace SceneLayer
{
    void Skybox::Init()
    {
        {
            dx12::DescriptorHeapDescription desc;
            desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            desc.SetNumDescriptors(10);
            desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

            _descHeap.SetDescription(desc);
            _descHeap.SetName("Skybox descriptor heap");
            _descHeap.Create();
        }

        {
            dx12::HeapDescription desc;
            desc.SetSize(_256MB * 2);
            desc.SetHeapType(D3D12_HEAP_TYPE_DEFAULT);
            desc.SetHeapFlags(D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES);

            _heap.SetDescription(desc);
            _heap.SetName("Skybox heap");
            _heap.Create();
        }
    }

    void Skybox::Load(const std::string& filepath, dx12::CommandList& commandList)
    {
        // Open and read Json file
        std::ifstream in(filepath, std::ios_base::in | std::ios_base::binary);
        Json::Value root;
        in >> root;

        _skyboxTexture = dx12::Texture::LoadFromFile(root["Skybox"].asCString());

        _skyboxTexture->SetDescriptorHeap(&_descHeap);

        _descHeap.PlaceResource(_skyboxTexture.get());
        _heap.PlaceResource(*_skyboxTexture);

        _skyboxTexture->UploadToGPU(commandList);

        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        SRVDesc.Texture2D.MipLevels = 1;
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        dx12::Device::GetDXDevice()->CreateShaderResourceView(_skyboxTexture->GetDXResource().Get(), &SRVDesc, _descHeap.GetResourceCPUHandle(_skyboxTexture.get()));

    }
} // namespace SceneLayer
