#pragma once

#include "PipelineState.h"

namespace Json
{
    class Value;
} // namespace Json

namespace rhi::d3d12
{
    // Wrapper for DirectX 12 RootSignature and D3D12PipelineState objects.
    // Constructs a graphics or compute pipeline from a JSON description file.
    class D3D12PipelineState final : public rhi::PipelineState
    {
    public:
        // Copy constructor.
        D3D12PipelineState(const D3D12PipelineState& other) = delete;
        // Move constructor.
        D3D12PipelineState(D3D12PipelineState&& other) noexcept;
        // Destructor.
        ~D3D12PipelineState() override;

        // Copy assignment operator.
        D3D12PipelineState& operator=(const D3D12PipelineState& other) = delete;
        // Move assignment operator.
        D3D12PipelineState& operator=(D3D12PipelineState&& other) noexcept;

        // Get a pointer to the raw D3D12 root signature object.
        ComPtr<ID3D12RootSignature> GetRootSignature() const;
        // Get a pointer to the raw D3D12 pipeline state object.
        ComPtr<ID3D12PipelineState> GetPipelineState() const;

        rhi::PipelineStateType GetType() const override;

        // Parse and create a graphics or compute pipeline from the given JSON file.
        void Parse(const std::string& filepath);

        rhi::BlendState ParseBlendDescription(const std::string& filepath) override;
        rhi::RasterizerState ParseRasterizerDescription(const std::string& filepath) override;
        rhi::DepthStencilState ParseDepthStencilDescription(const std::string& filepath) override;

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        // Default null initialization.
        D3D12PipelineState(rhi::Device* device);

        // Parse the graphics pipeline settings from the JSON file.
        void ParseGraphicsPipeline(const Json::Value& fileRoot);
        // Parse the compute pipeline settings from the JSON file.
        void ParseComputePipeline(const Json::Value& fileRoot);

        // Pointer to the raw D3D12 root signature object.
        ComPtr<ID3D12RootSignature> _rootSignature;
        // Pointer to the raw D3D12 pipeline state object.
        ComPtr<ID3D12PipelineState> _pipelineState;

        // Indicates whether this is a graphics pipeline (true) or compute pipeline (false).
        rhi::PipelineStateType _type;

        rhi::Device* _device;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
