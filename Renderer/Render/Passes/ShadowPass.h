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
        dx12::PipelineState _shadowsPipeline;
    };
} // namespace render
