#pragma once

#include "Heap.h"
#include "DescriptorHeap.h"
#include "Texture.h"

namespace dx12
{
    class CommandList;
} // namespace Core

namespace SceneLayer
{
    class Skybox
    {
    public:
        void Init();
        void Load(const std::string& filepath, dx12::CommandList& commandList);

    //private:
        std::shared_ptr<dx12::Texture> _skyboxTexture;

        dx12::DescriptorHeap _descHeap;
        dx12::Heap _heap;
    };
} // namespace SceneLayer
