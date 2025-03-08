#pragma once

class CacheGPU;

namespace dx12
{
    class CommandList;
    class ResourceTable;
}

namespace scene
{
    class Scene;
}

namespace helpers
{
    void SetupSceneDataGPU(scene::Scene& scene, dx12::CommandList& commandList, CacheGPU* cache);
    void SetupLightDataGPU(scene::Scene& scene, dx12::CommandList& commandList, CacheGPU* cache, dx12::ResourceTable& resourceTable);
} // namespace helpers
