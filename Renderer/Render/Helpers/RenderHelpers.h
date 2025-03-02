#pragma once

class CacheGPU;

namespace dx12
{
    class CommandList;
}

namespace scene
{
    class Scene;
}

namespace helpers
{
    void SetupSceneDataGPU(scene::Scene& scene, dx12::CommandList& commandList, CacheGPU* cache);
} // namespace helpers
