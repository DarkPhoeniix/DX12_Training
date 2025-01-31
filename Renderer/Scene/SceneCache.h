#pragma once

#include "Heap.h"

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

        std::shared_ptr<dx12::ResourceTable> GetTextureTable() const;
        std::shared_ptr<dx12::ResourceTable> GetLightsTable() const;

        dx12::Heap& GetTextureHeap();

        void SetTime(float time);
        float GetTime() const;

    private:
        std::shared_ptr<dx12::ResourceTable> _texturesTable;
        std::shared_ptr<dx12::ResourceTable> _lightsTable;

        dx12::Heap _texturesHeap;

        float _currentTime;
    };
} // namespace scene
