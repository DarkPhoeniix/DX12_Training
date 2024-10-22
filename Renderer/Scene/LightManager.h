#pragma once

#include "DXObjects/ResourceTable.h"

namespace Core
{
    class CommandList;
} // namespace Core

namespace SceneLayer
{
    class DirectionalLight;
    class PointLight;

    class LightManager
    {
    public:
        LightManager();
        ~LightManager() = default;

        void SetupLightsCompute(Core::CommandList& commandList);
        void SetupLights(Core::CommandList& commandList);

        UINT GetLightsNum() const;

        void AddDirectionalLight(std::shared_ptr<DirectionalLight> directionalLight);
        void AddPointLight(std::shared_ptr<PointLight> pointLight);

    private:
        std::shared_ptr<Core::Resource> CreateGPUResource();

        std::vector<std::shared_ptr<DirectionalLight>> _directionalLights;
        std::vector<std::shared_ptr<PointLight>> _pointLights;

        Core::Heap _lightsHeap;
        Core::DescriptorHeap _descHeap;
        std::shared_ptr<Core::Resource> _lightsView;
        std::vector<std::shared_ptr<Core::Resource>> _internalResources;
    };
} // namespace SceneLayer
