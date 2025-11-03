#pragma once

namespace Json
{
    class Value;
} // namespace Json

namespace dx12
{
    // Wrapper for DirectX 12 RootSignature and PipelineState objects.
    // Constructs a graphics or compute pipeline from a JSON description file.
    class PipelineState
    {
    public:
        // Default null initialization.
        PipelineState();
        // Copy constructor.
        PipelineState(const PipelineState& other);
        // Move constructor.
        PipelineState(PipelineState&& other) noexcept;
        // Destructor.
        ~PipelineState();

        // Copy assignment operator.
        PipelineState& operator=(const PipelineState& other);
        // Move assignment operator.
        PipelineState& operator=(PipelineState&& other) noexcept;

        // Get a pointer to the raw D3D12 root signature object.
        ComPtr<ID3D12RootSignature> GetRootSignature() const;
        // Get a pointer to the raw D3D12 pipeline state object.
        ComPtr<ID3D12PipelineState> GetPipelineState() const;

        // Check if the pipeline is a graphics pipeline.
        bool IsGraphicsPipeline() const;

        // Parse and create a graphics or compute pipeline from the given JSON file.
        void Parse(const std::string& filepath);

        // Parse and return the blend state description for the graphics pipeline.
        D3D12_BLEND_DESC ParseBlendDescription(const std::string& filepath);
        // Parse and return the rasterizer state description for the graphics pipeline.
        D3D12_RASTERIZER_DESC ParseRasterizerDescription(const std::string& filepath);
        // Parse and return the depth/stencil state description for the graphics pipeline.
        D3D12_DEPTH_STENCIL_DESC ParseDepthStencilDescription(const std::string& filepath);

    private:
        // Parse the graphics pipeline settings from the JSON file.
        void ParseGraphicsPipeline(const Json::Value& fileRoot);
        // Parse the compute pipeline settings from the JSON file.
        void ParseComputePipeline(const Json::Value& fileRoot);

        // Pointer to the raw D3D12 root signature object.
        ComPtr<ID3D12RootSignature> _rootSignature;
        // Pointer to the raw D3D12 pipeline state object.
        ComPtr<ID3D12PipelineState> _pipelineState;

        // Indicates whether this is a graphics pipeline (true) or compute pipeline (false).
        bool _isGraphicsPipeline;
    };
} // namespace dx12
