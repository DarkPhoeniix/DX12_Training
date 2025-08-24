#pragma once

#include "RenderGraph/RenderPass.h"

#include "PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
    struct LightingPassData
    {
        rg::ResourceId FrameBuffer;

        rg::ResourceId AlbedoMetallic;
        rg::ResourceId NormalRoughness;
        rg::ResourceId Depth;

        rg::ResourceId ShadowMaps;

        rg::ResourceId HDRTarget;
    };

    class LightingPass : public rg::RenderPass<LightingPassData>
    {
    public:
        LightingPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

        // Inherited via RenderPass
        void Setup(rg::RenderPassBuilder& builder) override;
        void Execute(rg::RenderContext& context, TaskGPU& task) override;

    private:
        dx12::PipelineState _deferredPipeline;

        std::shared_ptr<scene::Scene> _scene;
        scene::Camera* _camera;
    };
} // namespace render
