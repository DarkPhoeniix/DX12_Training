#pragma once

#include "RenderGraph/RenderPass.h"

#include "Editor/Editor.h"

namespace render
{
    struct GUIPassData
    {
        rg::RGTextureRenderTargetId Target;
        rg::RGTextureDepthStencilWriteId Depth;
    };

    class GUIPass : public rg::RenderPass<GUIPassData>
    {
    public:
        GUIPass(rhi::Device* device, gui::Editor* editor, scene::Camera* camera);

        // Inherited via RenderPass
        void Setup(rg::RenderPassBuilder& builder) override;
        void Execute(rg::RenderContext& context, rg::ITask* task) override;

    private:
        gui::Editor* _editor;
        scene::Camera* _camera;
    };
} // namespace render
