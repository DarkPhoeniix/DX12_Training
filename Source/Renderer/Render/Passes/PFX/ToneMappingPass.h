#pragma once

#include "RenderGraph/RenderPass.h"

#include "PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
    struct ToneMappingPassData
    {
        rg::ResourceId FrameBuffer;

        rg::ResourceId HDRTarget;
        rg::ResourceId AverageLuminance;

        rg::ResourceId Target;
    };

    class ToneMappingPass : public rg::RenderPass<ToneMappingPassData>
    {
    public:
        ToneMappingPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

        // Inherited via RenderPass
        void Setup(rg::RenderPassBuilder& builder) override;
        void Execute(rg::RenderContext& context, TaskGPU& task) override;

    private:
        dx12::PipelineState _toneMappingPipeline;

        std::shared_ptr<scene::Scene> _scene;
        scene::Camera* _camera;
    };
} // namespace render
