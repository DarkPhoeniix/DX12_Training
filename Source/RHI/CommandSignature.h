#pragma once

#include "PipelineState.h"

namespace rhi
{
    enum class IndirectArgumentType : std::uint8_t
    {
        Draw,
        DrawIndexed,
        Dispatch,
        VertexBufferView,
        IndexBufferView,
        Constant,
        ConstantBufferView,
        ShaderResourceView,
        UnorderedResourceView
    };

    struct IndirectArgumentDescription
    {
        IndirectArgumentType Type;
        union
        {
            struct
            {
                std::uint32_t Slot;
            } 	VertexBuffer;
            struct
            {
                std::uint32_t RootParameterIndex;
                std::uint32_t DestOffsetIn32BitValues;
                std::uint32_t Num32BitValuesToSet;
            } 	Constant;
            struct
            {
                std::uint32_t RootParameterIndex;
            } 	ConstantBufferView;
            struct
            {
                std::uint32_t RootParameterIndex;
            } 	ShaderResourceView;
            struct
            {
                std::uint32_t RootParameterIndex;
            } 	UnorderedAccessView;
            struct
            {
                std::uint32_t RootParameterIndex;
                std::uint32_t DestOffsetIn32BitValues;
            } 	IncrementingConstant;
        };
    };

    class CommandSignature
    {
    public:
        virtual ~CommandSignature() = default;

        virtual void* GetNative() const = 0;
    };
} // namespace rhi
