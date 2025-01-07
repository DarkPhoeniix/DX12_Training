#pragma once

#include "Heap.h"
#include "Scene/Entity/Components/IComponent.h"
#include "Texture.h"

namespace SceneLayer
{
    struct Skybox : public IComponent
    {
        Skybox();

        std::shared_ptr<dx12::Texture> SkydomeTexture;

        dx12::DescriptorHeap DescHeap;
        dx12::Heap TexHeap;
    };
} // namespace SceneLayer
