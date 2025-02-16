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
        dx12::PipelineState _pointLightCullingPipeline;

        ComPtr<ID3D12CommandSignature> _cmdSignature;

        dx12::Resource _counterReset;
        dx12::Resource _counterReadBack;
        std::vector<dx12::Resource> _counters;
        std::vector<dx12::Resource> _commandsBuffers;
        dx12::DescriptorHeap _commandsDescHeap;
    };
} // namespace render
