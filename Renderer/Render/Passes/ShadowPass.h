#pragma once

#include "IRenderPass.h"

#include "Scene/Entity/Components/Camera.h"

namespace render
{
    class ShadowPass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Inititalize() override;
        void Destroy() override;

        void Execute() override;

    private:
        dx12::PipelineState _shadowsPipeline;

        scene::Viewport _vp;
        dx12::Texture _shadowTexture;
    };
} // namespace render
