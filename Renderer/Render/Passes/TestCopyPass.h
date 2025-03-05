#pragma once

#include "RenderGraph/RenderPass.h"

#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
    struct TestCopyPassData
    {
        rg::ResourceId HDR;
        rg::ResourceId AlbedoMetallic;
        rg::ResourceId Target;
    };

    class TestCopyPass : public rg::RenderPass<TestCopyPassData>
    {
    public:
        TestCopyPass(scene::Scene* scene, scene::Camera* camera);

        // Inherited via RenderPass
        void Setup(rg::RenderPassBuilder& builder) override;
        void Execute(rg::RenderContext& context, TaskGPU& task) override;

    private:
        scene::Scene* _scene;
        scene::Camera* _camera;
    };
} // namespace render
