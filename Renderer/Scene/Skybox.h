#pragma once

#include "DXObjects/Heap.h"
#include "DXObjects/DescriptorHeap.h"
#include "DXObjects/Texture.h"

namespace Core
{
    class CommandList;
} // namespace Core

namespace SceneLayer
{
    class Skybox
    {
    public:
        void Init();
        void Load(const std::string& filepath, Core::CommandList& commandList);

    //private:
        std::shared_ptr<Core::Texture> _skyboxTexture;

        Core::DescriptorHeap _descHeap;
        Core::Heap _heap;
    };
} // namespace SceneLayer
