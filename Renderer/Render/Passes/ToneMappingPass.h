#pragma once

#include "IRenderPass.h"

namespace render
{
    class ToneMappingPass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Inititalize() override;
        void Destroy() override;

        void Execute() override;

    private:
        void Downscale1();
        void Downscale2();
        void Tonemapping();

        dx12::PipelineState _lumDownscale1Pipeline;
        dx12::PipelineState _lumDownscale2Pipeline;
        dx12::PipelineState _toneMappingPipeline;

        dx12::Resource _averageLuminance;
        float _adaptation;
    };
} // namespace render
