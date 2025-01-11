#pragma once

#include "IRenderPass.h"

namespace render
{
    class GeometryPass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Inititalize() override;
        void Destroy() override;

        void Execute() override;

    private:
        dx12::PipelineState _geometryPipeline;
    };
} // namespace render
