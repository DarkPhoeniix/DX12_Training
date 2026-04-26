#pragma once

#include "PipelineStates.h"

namespace rhi
{
    // PipelineState is an abstract interface representing a pipeline state object, which encapsulates the configuration and state of the graphics or compute pipeline
    class PipelineState
    {
    public:
        PipelineState() = default;
        PipelineState(const PipelineState&) = delete;
        PipelineState(PipelineState&&) noexcept = default;
        virtual ~PipelineState() = default;

        PipelineState& operator=(const PipelineState&) = delete;
        PipelineState& operator=(PipelineState&&) noexcept = default;

        // Retrieves the type of the pipeline state, which determines how it will be used in the rendering or compute process
        virtual PipelineStateType GetType() const = 0;

        // Retrieves the native pipeline state object, allowing the application to access the underlying API-specific pipeline state
        virtual void* GetNative() const = 0;
        // Retrieves the native root signature object associated with the pipeline state, allowing the application to access the underlying API-specific root signature
        virtual void* GetNativeRootSignature() const = 0;
    };
} // namespace rhi
