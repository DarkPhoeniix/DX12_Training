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
} // namespace scene
