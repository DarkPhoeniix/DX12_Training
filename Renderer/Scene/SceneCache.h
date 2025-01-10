#pragma once

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

        void SetTime(float time);
        float GetTime() const;

    private:
        std::shared_ptr<dx12::ResourceTable> _texturesTable;

        std::shared_ptr<dx12::ResourceTable> _lightsTable;

        float _currentTime;
        
        std::shared_ptr<dx12::Resource> _gpuDesc;
    };
} // namespace scene
