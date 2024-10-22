#pragma once

#include "DXObjects/DescriptorHeap.h"
#include "DXObjects/Resource.h"
#include "DirectXTex/DirectXTex.h"

namespace Core
{
    class CommandList;

    class Texture : public Resource
    {
    public:
        Texture();
        ~Texture();

        void UploadToGPU(CommandList& commandList);

        void SetDescriptorHeap(DescriptorHeap* descriptorHeap);
        DescriptorHeap* GetDescriptorHeap() const;

        static std::shared_ptr<Texture> CreateTexture(const DirectX::XMVECTOR& color);
        static std::shared_ptr<Texture> LoadFromFile(std::string filepath);

    private:
        ComPtr<ID3D12Resource> _intermediateResource;
        DirectX::ScratchImage _scratchImage;
        DirectX::TexMetadata _metadata;

        DescriptorHeap* _descritptorHeap;
    };
} // namespace Core
