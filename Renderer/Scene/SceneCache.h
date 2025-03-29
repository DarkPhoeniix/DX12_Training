#pragma once

#include "Heap.h"
#include "TextureManager.h"

namespace dx12
{
    class ResourceTable;
}

namespace scene
{
    class SceneCache
    {
    public:
        SceneCache();
        SceneCache(const SceneCache&) = delete;
        SceneCache(SceneCache&&) = default;
        ~SceneCache() = default;

        SceneCache& operator=(const SceneCache&) = delete;
        SceneCache& operator=(SceneCache&&) = default;

        std::shared_ptr<dx12::ResourceTable> GetLightsTable() const;

        TextureManager& GetTextureManager();

        void SetDeltaTime(float deltaTime);
        float GetDeltaTime() const;

    private:
        TextureManager _textureManager;
        std::shared_ptr<dx12::ResourceTable> _lightsTable;

        float _deltaTime;
    };
} // namespace scene
