#pragma once

#include "IRenderPass.h"

class DebugBoundingVolumePass : public IRenderPass
{
public:
    // Inherited via IRenderPass
    void Inititalize() override;
    void Destroy() override;

    void Execute() override;

private:
    dx12::PipelineState _AABBpipeline;
    dx12::PipelineState _OBBpipeline;
};
