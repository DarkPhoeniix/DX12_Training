#pragma once

#include "Render/Frame/Frame.h"
#include "Render/GBuffer.h"
#include "Scene/Scene.h"

class IRenderPass
{
public:
    virtual ~IRenderPass() = default;

    void SetScene(SceneLayer::Scene& scene);
    void SetRenderFrame(Frame& currentFrame);
    void SetGeometryBuffer(Core::GBuffer& gBuffer);

    const std::string& GetName() const;

    virtual void Inititalize();
    virtual void Destroy();

    virtual void Execute() = 0;

protected:
    SceneLayer::Scene* _scene;
    Frame* _frame;
    Core::GBuffer* _gBuffer;

    SceneLayer::Camera* _activeCamera;

    std::string _name;
};
