#pragma once

#include "IRenderPass.h"

namespace render
{
    class DebugArmaturePass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Initialize() override;
        void Destroy() override;

        void Execute() override;

    private:
        dx12::PipelineState _debugArmaturePipeline;
    };
} // namespace render
