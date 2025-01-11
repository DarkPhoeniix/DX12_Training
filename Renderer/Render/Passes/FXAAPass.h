#pragma once

#include "IRenderPass.h"

namespace render
{
    class FXAAPass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Inititalize() override;
        void Destroy() override;

        void Execute() override;

    private:
        dx12::PipelineState _FXAAPipeline;

        dx12::Resource _fxaaRTT;
    };
} // namespace render
