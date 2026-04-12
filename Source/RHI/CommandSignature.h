#pragma once

#include "PipelineState.h"

namespace rhi
{
    // IndirectArgumentType represents the type of argument that can be used in an indirect command signature. 
    // It is used to specify the type of data that should be provided for each argument when executing indirect draw or dispatch commands
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

    // IndirectArgumentDescription describes a single argument in an indirect command signature. 
    // It specifies the type of the argument and any additional information needed to interpret the argument data when executing indirect draw or dispatch commands
    struct IndirectArgumentDescription
    {
        // Type of the argument, indicating how the GPU should interpret the data for this argument when executing indirect commands
        IndirectArgumentType Type;
        // Additional information for the argument, which can vary based on the type of the argument. 
        // It is used to provide specific details needed to interpret the argument data correctly when executing indirect commands
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

    // CommandSignature is an abstract interface representing a command signature, which defines the layout of indirect command buffers for GPU execution
    class CommandSignature
    {
    public:
        virtual ~CommandSignature() = default;

        // Retrieves the native command signature object, allowing the application to access the underlying API-specific command signature
        virtual void* GetNative() const = 0;
    };
} // namespace rhi
