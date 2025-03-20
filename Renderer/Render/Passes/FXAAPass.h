#pragma once

#include "RenderGraph/RenderPass.h"

#include "PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
    struct FXAAPassData
    {
        rg::ResourceId Target;
        rg::ResourceId WorkCounters;
        rg::ResourceId WorkQueue;
        rg::ResourceId ColorQueue;
        rg::ResourceId LumaBuffer;
        rg::ResourceId IndirectParams;
        rg::ResourceId FXAATarget;
    };

    class FXAAPass : public rg::RenderPass<FXAAPassData>
    {
    public:
        FXAAPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

        // Inherited via RenderPass
        void Setup(rg::RenderPassBuilder& builder) override;
        void Execute(rg::RenderContext& context, TaskGPU& task) override;

    private:
        dx12::PipelineState _FXAA_Pass1_Pipeline;
        dx12::PipelineState _FXAA_ResolveWork_Pipeline;
        dx12::PipelineState _FXAA_Pass2H_Pipeline;
        dx12::PipelineState _FXAA_Pass2V_Pipeline;

        ComPtr<ID3D12CommandSignature> _cmdSignature;

        dx12::Resource _paramsReset;

        std::shared_ptr<scene::Scene> _scene;
        scene::Camera* _camera;
    };
} // namespace render
