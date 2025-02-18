#pragma once

#include "Heap.h"
#include "Scene/Entity/Components/IComponent.h"
#include "Texture.h"

namespace scene
{
    class Skybox : public IComponent
    {
    public:
        Skybox();

        std::shared_ptr<dx12::Texture> SkydomeTexture;

        dx12::DescriptorHeap DescHeap;
        dx12::Heap TexHeap;
    };
} // namespace scene
