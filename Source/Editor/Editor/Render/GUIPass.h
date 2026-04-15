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
        GUIPass(rhi::Device* device, std::shared_ptr<gui::Editor> editor);

        // Inherited via RenderPass
        void Setup(rg::RenderPassBuilder& builder) override;
        void Execute(rg::RenderContext& context, rg::ITask* task) override;

    private:
        std::shared_ptr<gui::Editor> _editor;
    };
} // namespace render
