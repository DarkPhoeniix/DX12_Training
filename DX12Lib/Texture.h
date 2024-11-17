#pragma once

#include "DescriptorHeap.h"
#include "Resource.h"

namespace dx12
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

        static std::shared_ptr<Texture> LoadFromFile(std::string filepath);

    private:
        ComPtr<ID3D12Resource> _intermediateResource;
        DirectX::ScratchImage _scratchImage;
        DirectX::TexMetadata _metadata;

        DescriptorHeap* _descritptorHeap;
    };
} // namespace dx12
