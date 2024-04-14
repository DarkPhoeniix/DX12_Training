#pragma once

#include "DXObjects/DescriptorHeap.h"
#include "DXObjects/Resource.h"
#include "DirectXTex/DirectXTex.h"

class Texture : public Resource
{
public:
    Texture();
    ~Texture();

    void UploadToGPU(ComPtr<ID3D12GraphicsCommandList2> commandList);

    void SetDescriptorHeap(DescriptorHeap* descriptorHeap);
    DescriptorHeap* GetDescriptorHeap() const;

    void SetDXDevice(ComPtr<ID3D12Device2> device);
    ComPtr<ID3D12Device2> GetDXDevice() const;

    static std::shared_ptr<Texture> LoadFromFile(const std::string& filepath);

private:
    ComPtr<ID3D12Resource> _intermediateResource;
    DirectX::ScratchImage _scratchImage;
    DirectX::TexMetadata _metadata;

    DescriptorHeap* _descritptorHeap;
};
