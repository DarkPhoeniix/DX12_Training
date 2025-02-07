#pragma once

#include "IRenderPass.h"

namespace render
{
    class ShadowPass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Inititalize() override;
        void Destroy() override;

        void Execute() override;

    private:
        void SpotLightsPass();
        void PointLightsPass();

        dx12::PipelineState _shadowSpotLightPipeline;
        dx12::PipelineState _shadowPointLightPipeline;
    };
} // namespace render
