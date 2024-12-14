#pragma once

#include "Texture.h"
#include "Heap.h"
#include "Scene/ECS/Components.h"

struct COMPONENT Skybox : public IComponent
{
    Skybox();

    std::shared_ptr<dx12::Texture> SkydomeTexture;

    dx12::DescriptorHeap DescHeap;
    dx12::Heap TexHeap;
};
