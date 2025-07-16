#pragma once

#include "RenderGraph/RenderPass.h"

#include "Editor/Editor.h"

namespace render
{
    struct GUIPassData
    {
        rg::ResourceId Target;
        rg::ResourceId Depth;
    };

    class GUIPass : public rg::RenderPass<GUIPassData>
    {
    public:
        GUIPass(std::shared_ptr<gui::Editor> editor);

        // Inherited via RenderPass
        void Setup(rg::RenderPassBuilder& builder) override;
        void Execute(rg::RenderContext& context, TaskGPU& task) override;

    private:
        std::shared_ptr<gui::Editor> _editor;
    };
} // namespace render
