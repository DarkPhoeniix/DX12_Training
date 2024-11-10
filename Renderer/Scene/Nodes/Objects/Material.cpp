#include "stdafx.h"

#include "Material.h"

#include "DXObjects/ResourceTable.h"

namespace SceneLayer
{
    Core::Texture* Material::Albedo() const
    {
        return _albedoTexture.get();
    }

    Core::Texture* Material::NormalMap() const
    {
        return _normalTexture.get();
    }

    Core::Texture* Material::Metalness() const
    {
        return _metalnessTexture.get();
    }

    Core::Texture* Material::Roughness() const
    {
        return _roughnessTexture.get();
    }

    UINT Material::AlbedoIndex(Core::ResourceTable* resourceTable) const
    {
        return resourceTable->GetResourceIndex(_albedoTexture->GetName());
    }

    UINT Material::NormalMapIndex(Core::ResourceTable* resourceTable) const
    {
        return resourceTable->GetResourceIndex(_normalTexture->GetName());
    }

    UINT Material::MetalnessIndex(Core::ResourceTable* resourceTable) const
    {
        return resourceTable->GetResourceIndex(_metalnessTexture->GetName());
    }

    UINT Material::RoughnessIndex(Core::ResourceTable* resourceTable) const
    {
        return resourceTable->GetResourceIndex(_roughnessTexture->GetName());
    }

    void Material::UploadToGPU(Core::CommandList& commandList, Core::ResourceTable* resourceTable)
    {
        ASSERT(resourceTable, "Resource table is nullptr");

        if (resourceTable->AddResource(_albedoTexture.get()))
        {
            _albedoTexture->SetDescriptorHeap(&resourceTable->GetDescriptorHeap());
            _albedoTexture->UploadToGPU(commandList);
        }

        if (resourceTable->AddResource(_normalTexture.get()))
        {
            _normalTexture->SetDescriptorHeap(&resourceTable->GetDescriptorHeap());
            _normalTexture->UploadToGPU(commandList);
        }

        if (resourceTable->AddResource(_metalnessTexture.get()))
        {
            _metalnessTexture->SetDescriptorHeap(&resourceTable->GetDescriptorHeap());
            _metalnessTexture->UploadToGPU(commandList);
        }

        if (resourceTable->AddResource(_roughnessTexture.get()))
        {
            _roughnessTexture->SetDescriptorHeap(&resourceTable->GetDescriptorHeap());
            _roughnessTexture->UploadToGPU(commandList);
        }
    }

    Material* Material::LoadFromFile(const std::string& filepath)
    {
        if (ASSERT(std::filesystem::exists(filepath), std::format("Material doesn't exist: {}", filepath)))
        {
            return nullptr;
        }

        std::ifstream file(filepath, std::ios_base::in | std::ios_base::binary);
        Json::Value materialData;
        file >> materialData;

        Material* material = new Material();

        material->_albedoTexture = Core::Texture::LoadFromFile(materialData["Albedo"].asCString());
        material->_normalTexture = Core::Texture::LoadFromFile(materialData["Normal"].asCString());
        material->_metalnessTexture = Core::Texture::LoadFromFile(materialData["Metalness"].asCString());
        material->_roughnessTexture = Core::Texture::LoadFromFile(materialData["Roughness"].asCString());

        return material;
    }
} // namespace SceneLayer
