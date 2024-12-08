#pragma once

#include "Scene/Camera.h"

namespace dx12
{
    class ResourceTable;
}

namespace SceneLayer
{
    class SceneCache
    {
    public:
        SceneCache();
        ~SceneCache();

        std::shared_ptr<dx12::ResourceTable> GetTextureTable() const;

        std::shared_ptr<dx12::ResourceTable> GetLightsTable() const;
        dx12::Resource& GetLightsSRV();

        // TODO: ...
        void SetCamera(Camera* camera);
        Camera* GetCamera() const;

        void SetTime(float time);
        float GetTime() const;

    private:
        std::shared_ptr<dx12::ResourceTable> _texturesTable;

        std::shared_ptr<dx12::ResourceTable> _lightsTable;
        dx12::Resource _lightsView;

        Camera* _camera;

        float _currentTime;
        
        std::shared_ptr<dx12::Resource> _gpuDesc;
    };
} // namespace SceneLayer
