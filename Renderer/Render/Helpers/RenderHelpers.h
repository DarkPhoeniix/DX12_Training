#pragma once

class CacheGPU;

namespace dx12
{
    class CommandList;
}

namespace SceneLayer
{
    class Scene;
}

namespace Helpers
{
    void SetupSceneDataGPU(SceneLayer::Scene& scene, dx12::CommandList& commandList, CacheGPU* frameCache);
} // namespace Helpers
