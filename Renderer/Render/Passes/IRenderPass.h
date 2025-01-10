#pragma once

#include "Render/Frame/Frame.h"
#include "Render/GBuffer.h"
#include "Scene/Scene.h"

namespace render
{
    class IRenderPass
    {
    public:
        virtual ~IRenderPass() = default;

        void SetScene(scene::Scene& scene);
        void SetRenderFrame(Frame& currentFrame);
        void SetGeometryBuffer(GBuffer& gBuffer);

        const std::string& GetName() const;

        virtual void Inititalize();
        virtual void Destroy();

        virtual void Execute() = 0;

    protected:
        scene::Scene* _scene;
        Frame* _frame;
        GBuffer* _gBuffer;

        scene::Camera* _activeCamera;

        std::string _name;
    };
} // namespace render
