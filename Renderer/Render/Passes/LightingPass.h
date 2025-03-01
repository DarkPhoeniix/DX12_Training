#pragma once

#include "IRenderPass.h"

namespace render
{
    class LightingPass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Initialize() override;
        void Destroy() override;

        void Execute() override;

    private:
        dx12::PipelineState _deferredPipeline;

        dx12::Resource _HDRTexture;
    };
} // namespace render
