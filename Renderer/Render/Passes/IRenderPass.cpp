#include "RendererPCH.h"

#include "IRenderPass.h"

using namespace SceneLayer;

void IRenderPass::SetScene(SceneLayer::Scene& scene)
{
    _scene = &scene;
}

void IRenderPass::SetRenderFrame(Frame& currentFrame)
{
    _frame = &currentFrame;
}

void IRenderPass::SetGeometryBuffer(Core::GBuffer& gBuffer)
{
    _gBuffer = &gBuffer;
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
