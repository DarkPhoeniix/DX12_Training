#pragma once

#include "IRenderPass.h"

namespace render
{
    class DebugBoundingVolumePass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Initialize() override;
        void Destroy() override;

        void Execute() override;

    private:
        dx12::PipelineState _AABBpipeline;
        dx12::PipelineState _OBBpipeline;
    };
} // namespace render
