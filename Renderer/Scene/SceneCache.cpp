#include "RendererPCH.h"

#include "SceneCache.h"

#include "ResourceTable.h"

namespace scene
{
    SceneCache::SceneCache()
    {
        _lightsTable = std::make_shared<dx12::ResourceTable>();
        _lightsTable->Init(4);
    }

    SceneCache::SceneCache(const SceneCache& other)
        : _textureManager(other._textureManager)
        , _lightsTable(other._lightsTable)
        , _deltaTime(other._deltaTime)
    {
    }

    SceneCache::SceneCache(SceneCache&& other) noexcept
        : _textureManager(std::move(other._textureManager))
        , _lightsTable(std::move(other._lightsTable))
        , _deltaTime(other._deltaTime)
    {
    }

    SceneCache& SceneCache::operator=(const SceneCache& other)
    {
        if (this != &other)
        {
            _textureManager = other._textureManager;
            _lightsTable = other._lightsTable;
            _deltaTime = other._deltaTime;
        }

        return *this;
    }

    SceneCache& SceneCache::operator=(SceneCache&& other) noexcept
    {
        if (this != &other)
        {
            _textureManager = std::move(other._textureManager);
            _lightsTable.swap(other._lightsTable);
            _deltaTime = other._deltaTime;
        }

        return *this;
    }

    std::shared_ptr<dx12::ResourceTable> SceneCache::GetLightsTable() const
    {
        return _lightsTable;
    }

    TextureManager& SceneCache::GetTextureManager()
    {
        return _textureManager;
    }

    void SceneCache::SetDeltaTime(float deltaTime)
    {
        _deltaTime = deltaTime;
    }
    
    float SceneCache::GetDeltaTime() const
    {
        return _deltaTime;
    }

    void SceneCache::Clear()
    {
        _textureManager.Clear();
        _deltaTime = 0.0f;
        _lightsTable->Reset();
    }
} // namespace scene
