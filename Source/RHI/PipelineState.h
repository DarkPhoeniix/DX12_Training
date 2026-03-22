#pragma once

#include "PipelineStates.h"

namespace rhi
{
    class PipelineState
    {
    public:
        PipelineState() = default;
        PipelineState(const PipelineState&) = delete;
        PipelineState(PipelineState&&) noexcept = default;
        virtual ~PipelineState() = default;

        PipelineState& operator=(const PipelineState&) = delete;
        PipelineState& operator=(PipelineState&&) noexcept = default;

        virtual PipelineStateType GetType() const = 0;

        virtual void Parse(const std::string& filepath) = 0;

        virtual BlendState ParseBlendDescription(const std::string& filepath) = 0;
        virtual RasterizerState ParseRasterizerDescription(const std::string& filepath) = 0;
        virtual DepthStencilState ParseDepthStencilDescription(const std::string& filepath) = 0;

        virtual void* GetNative() const = 0;
    };
} // namespace rhi
