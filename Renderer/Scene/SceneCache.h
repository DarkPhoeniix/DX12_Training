#pragma once

#include "Scene/Camera.h"

namespace Core
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

        std::shared_ptr<Core::ResourceTable> GetTextureTable() const;

        std::shared_ptr<Core::ResourceTable> GetLightsTable() const;
        Core::Resource& GetLightsSRV();

        // TODO: ...
        void SetCamera(Camera* camera);
        Camera* GetCamera() const;

    private:
        std::shared_ptr<Core::ResourceTable> _texturesTable;

        std::shared_ptr<Core::ResourceTable> _lightsTable;
        Core::Resource _lightsView;

        Camera* _camera;
        
        std::shared_ptr<Core::Resource> _gpuDesc;
    };
} // namespace SceneLayer
