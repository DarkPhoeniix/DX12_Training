#pragma once

#include "Heap.h"
#include "ResourceTable.h"

namespace dx12
{
    class CommandList;
    class Texture;
}

namespace scene
{
    class TextureManager
    {
    public:
        void EnqueueTexture(const std::string& filepath);
        void UploadTextures();

        std::shared_ptr<dx12::Texture> GetTexture(const std::string& name) const;

    private:
        std::unordered_map<std::string, std::shared_ptr<dx12::Texture>> _textures;
        std::vector<std::string> _uploadQueue;

        dx12::ResourceTable _texturesTable;
        dx12::Heap _texturesHeap;
    };
}
