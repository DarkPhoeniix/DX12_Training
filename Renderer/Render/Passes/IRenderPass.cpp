#include "RendererPCH.h"

#include "IRenderPass.h"

using namespace scene;

namespace render
{
    void IRenderPass::SetScene(Scene& scene)
    {
        _scene = &scene;
    }

    void IRenderPass::SetRenderFrame(Frame& currentFrame)
    {
        _tasks.clear();
        _frame = &currentFrame;
    }

    void IRenderPass::SetGeometryBuffer(GBuffer& gBuffer)
    {
        _gBuffer = &gBuffer;
    }

    const std::vector<TaskGPU*>& IRenderPass::GetTasks() const
    {
        return _tasks;
    }

    const std::string& IRenderPass::GetName() const
    {
        return _name;
    }

    void IRenderPass::Inititalize()
    {
        ASSERT(_scene, "Scene is not set in Render pass");
        ASSERT(_gBuffer, "Geometry buffer is not set in Render pass");

        _activeCamera = _scene->FindNodeByComponentName("Camera")->GetComponentAs<Camera>("Camera");
    }

    void IRenderPass::Destroy()
    {
        _scene = nullptr;
        _frame = nullptr;
        _gBuffer = nullptr;
    }
} // namespace render
