#pragma once

#include "IRenderPass.h"

namespace render
{
    class ToneMappingPass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Initialize() override;
        void Destroy() override;

        void Execute() override;

    private:
        void BuildLuminanceHistogram();
        void CalculateAverageLuminance();
        void ApplyToneMapping();

        dx12::PipelineState _luminanceHistogramPipeline;
        dx12::PipelineState _averageLuminanceHistogramPipeline;
        dx12::PipelineState _toneMappingPipeline;

        dx12::Resource _averageFrameLum[3];

        float _adaptationSpeed;
    };
} // namespace render