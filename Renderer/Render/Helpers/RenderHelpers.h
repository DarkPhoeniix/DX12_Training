#pragma once

class Frame;

namespace dx12
{
    class CommandList;
}

namespace scene
{
    class Scene;
}

namespace Helpers
{
    void SetupSceneDataGPU(scene::Scene& scene, dx12::CommandList& commandList, Frame* frame);
} // namespace Helpers
