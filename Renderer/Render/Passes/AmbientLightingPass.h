
#pragma once

#include "RenderGraph/RenderPass.h"

#include "PipelineState.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
    struct AmbientLightingPassData
    {
        rg::ResourceId AlbedoMetallic;
        rg::ResourceId NormalRoughness;
        rg::ResourceId Depth;

        rg::ResourceId DiffuseIrradianceMap;
        rg::ResourceId PreFilteredMap;
        rg::ResourceId BRDF_LUT;

        rg::ResourceId HDRTarget;
    };

    class AmbientLightingPass : public rg::RenderPass<AmbientLightingPassData>
    {
    public:
        AmbientLightingPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera);

        // Inherited via RenderPass
        void Setup(rg::RenderPassBuilder& builder) override;
        void Execute(rg::RenderContext& context, TaskGPU& task) override;

    private:
        dx12::PipelineState _ambientLightingPipeline;

        std::shared_ptr<scene::Scene> _scene;
        scene::Camera* _camera;
    };
} // namespace render