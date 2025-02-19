#pragma once

#include "DescriptorHeap.h"

namespace dx12
{
    class CommandList;

    // Wrapper for a DirectX 12 texture resource.
    class Texture : public Resource
    {
    public:
        // Uploads the texture data from CPU to GPU using the given command list.
        void UploadToGPU(CommandList& commandList);

        // Set the descriptor heap associated with this texture.
        void SetDescriptorHeap(DescriptorHeap* descriptorHeap);
        // Get the descriptor heap associated with this texture.
        DescriptorHeap* GetDescriptorHeap() const;

        // Load a texture from a file and return a shared pointer to the Texture object.
        static std::shared_ptr<Texture> LoadFromFile(std::string filepath);

    private:
        // Intermediate resource used for staging data before uploading to the GPU.
        ComPtr<ID3D12Resource> _intermediateResource;
        // Scratch image container for storing raw texture data before upload.
        DirectX::ScratchImage _scratchImage;
        // Metadata describing the texture format, dimensions, and mip levels.
        DirectX::TexMetadata _metadata;

        // Pointer to the descriptor heap where this texture’s SRV (Shader Resource View) is stored.
        // TODO: redundant
        DescriptorHeap* _descriptorHeap;
    };
} // namespace dx12
