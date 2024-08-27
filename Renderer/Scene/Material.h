#pragma once

#include "DXObjects/Heap.h"
#include "DXObjects/Texture.h"

class Material
{
public:
    const Core::Texture& Albedo() const;
    Core::Texture& Albedo();

    const Core::Texture& NormalMap() const;
    Core::Texture& NormalMap();

    static Material* LoadMaterial(const std::string& filepath);

private:
    std::shared_ptr<Core::Texture> _albedoTexture;
    std::shared_ptr<Core::Texture> _normalTexture;
};
