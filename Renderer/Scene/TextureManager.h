#pragma once

#include "Heap.h"
#include "ResourceTable.h"

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

        void EnqueueTexture(const std::string& filepath);
        void UploadTextures(dx12::CommandList& commandList);
        void CleanIntermediates();

        std::shared_ptr<dx12::Texture> GetTexture(const std::string& name) const;

        dx12::ResourceTable& GetTextureTable();

    private:
        std::unordered_map<std::string, std::shared_ptr<dx12::Texture>> _textures;
        std::set<std::string> _uploadQueue; // use set to remove duplicates

        dx12::ResourceTable _texturesTable;
        dx12::Heap _texturesHeap;
        std::unordered_map<std::string, dx12::Resource> _intermediateResources;
    };
}
