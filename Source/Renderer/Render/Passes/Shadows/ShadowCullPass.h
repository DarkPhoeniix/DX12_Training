#pragma once

#include "RenderGraph/RenderPass.h"

#include "PipelineState.h"

#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
    struct ShadowCullPassData
    {
        rg::ResourceId ShadowMaps;  // TODO: remove

        rg::ResourceId CounterResetBuffer;
        rg::ResourceId AABBBuffer;
        std::vector<rg::ResourceId> CandidateInstancesBuffer;
        std::vector<rg::ResourceId> LightCommandBuffers[dx12::BACK_BUFFER_COUNT];
    };

    class ShadowCullPass : public rg::RenderPass<ShadowCullPassData>
    {
    public:
        ShadowCullPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

        // Inherited via RenderPass
        void Setup(rg::RenderPassBuilder& builder) override;
        void Execute(rg::RenderContext& context, TaskGPU& task) override;

    private:
        dx12::PipelineState _cullShadowsPipeline;
        dx12::PipelineState _lightShadowsPipeline;

        ComPtr<ID3D12CommandSignature> _cmdSignature;

        std::shared_ptr<scene::Scene> _scene;
        scene::Camera* _camera;
    };
} // namespace render
