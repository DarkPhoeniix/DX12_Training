#include "stdafx.h"

#include "Material.h"

const Core::Texture& Material::Albedo() const
{
    return *_albedoTexture;
}

Core::Texture& Material::Albedo()
{
    return *_albedoTexture;
}

const Core::Texture& Material::NormalMap() const
{
    return *_normalTexture;
}

Core::Texture& Material::NormalMap()
{
    return *_normalTexture;
}

Material* Material::LoadMaterial(const std::string& filepath)
{
    // TODO: assert

    std::ifstream file(filepath, std::ios_base::in | std::ios_base::binary);
    Json::Value mat;
    file >> mat;

    Material* material = new Material();

    material->_albedoTexture = Core::Texture::LoadFromFile(mat["Albedo"].asCString());
    material->_normalTexture = Core::Texture::LoadFromFile(mat["Normal"].asCString());

    return material;
}
