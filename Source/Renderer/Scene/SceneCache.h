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
        SceneCache(const SceneCache& other);
        SceneCache(SceneCache&& other) noexcept;
        ~SceneCache() = default;

        SceneCache& operator=(const SceneCache& other);
        SceneCache& operator=(SceneCache&& other) noexcept;

        std::shared_ptr<dx12::ResourceTable> GetLightsTable() const;

        TextureManager& GetTextureManager();

        void SetDeltaTime(float deltaTime);
        float GetDeltaTime() const;

        void Clear();

    private:
        TextureManager _textureManager;
        std::shared_ptr<dx12::ResourceTable> _lightsTable;

        float _deltaTime;
    };
} // namespace scene
