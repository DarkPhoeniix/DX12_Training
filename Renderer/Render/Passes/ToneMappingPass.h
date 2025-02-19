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
        void BuildLuminanceHistogram();
        void CalculateAverageLuminance();
        void ApplyTonemapping();

        dx12::PipelineState _luminanceHistogramPipeline;
        dx12::PipelineState _averageluminanceHistogramPipeline;
        dx12::PipelineState _toneMappingPipeline;

        float _adaptationSpeed;
    };
} // namespace render