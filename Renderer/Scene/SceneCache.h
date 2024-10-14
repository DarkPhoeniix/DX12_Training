#pragma once

#include "Scene/LightManager.h"
#include "Scene/Nodes/Camera/Camera.h"

namespace SceneLayer
{
    class SceneCache
    {
    public:
        SceneCache();
        ~SceneCache();

        std::shared_ptr<Core::ResourceTable> GetTextureTable() const;

        LightManager* GetLightManager();

        // TODO: ...
        void SetCamera(Camera* camera);
        Camera* GetCamera() const;

    private:
        std::shared_ptr<Core::ResourceTable> _texturesTable;

        LightManager _lightManager;

        Camera* _camera;
        
        std::shared_ptr<Core::Resource> _sceneGPUData;
    };
} // namespace SceneLayer
