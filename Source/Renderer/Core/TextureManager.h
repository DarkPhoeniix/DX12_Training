#pragma once

#include "Heap.h"
#include "../DX12Lib/ResourceTable.h"

#include <set>

namespace dx12
{
    class CommandList;
    class Texture;
}

namespace DirectX
{
    class ScratchImage;
}

namespace scene
{
    class TextureManager
    {
    public:
        TextureManager();
        TextureManager(const TextureManager& other);
        TextureManager(TextureManager&& other) noexcept;

        TextureManager& operator=(const TextureManager& other);
        TextureManager& operator=(TextureManager&& other) noexcept;

        void EnqueueTexture(const std::string& filepath);
        void UploadTextures(dx12::CommandList& commandList);
        void CleanIntermediates();

        void Clear();

        void AddTexture(std::shared_ptr<dx12::Resource> texture, dx12::ResourceViewType viewType);
        std::shared_ptr<dx12::Resource> GetTexture(const std::string& name) const;

        dx12::ResourceTable& GetTextureTable();

    private:
        std::unordered_map<std::string, std::shared_ptr<dx12::Resource>> _textures;
        std::set<std::string> _uploadQueue; // use set to remove duplicates

        dx12::ResourceTable _texturesTable;
        dx12::Heap _texturesHeap;
        std::unordered_map<std::string, std::shared_ptr<dx12::Resource>> _intermediateResources;
    };
}
