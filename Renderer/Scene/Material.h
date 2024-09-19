#pragma once

#include "DXObjects/Heap.h"
#include "DXObjects/Texture.h"

namespace Core
{
    class GraphicsCommandList;
    class ResourceTable;
} // namespace Core

class Material
{
public:
    Core::Texture* Albedo() const;
    Core::Texture* NormalMap() const;
    Core::Texture* Metalness() const;

    UINT AlbedoIndex(Core::ResourceTable* resourceTable) const;
    UINT NormalMapIndex(Core::ResourceTable* resourceTable) const;
    UINT MetalnessIndex(Core::ResourceTable* resourceTable) const;

    void UploadToGPU(Core::GraphicsCommandList& commandList, Core::ResourceTable* resourceTable);

    static Material* LoadFromFile(const std::string& filepath);

private:
    std::shared_ptr<Core::Texture> _albedoTexture;
    std::shared_ptr<Core::Texture> _normalTexture;
    std::shared_ptr<Core::Texture> _metalnessTexture;
};
