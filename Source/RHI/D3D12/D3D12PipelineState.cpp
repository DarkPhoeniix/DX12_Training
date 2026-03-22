
#include "RHI_PCH.h"

#include "D3D12PipelineState.h"

#include <json/json.h>

#include <filesystem>
#include <fstream>

namespace rhi::d3d12
{
    namespace
    {
        Json::Value ParseJson(const std::string& filepath)
        {
            ASSERT(std::filesystem::exists(filepath), "Failed to load {}" + filepath);

            std::ifstream file(filepath, std::ios_base::binary);
            file.open(filepath, std::ios_base::binary);
            Json::Value root;

            file >> root;

            return root;
        }

        // TODO: add types !!!!!!!!!!!!!!!!!!!!!!!!!!!!
        const std::map<std::string, rhi::Format> FORMAT =
        {
            { "float4", rhi::Format::R32G32B32A32_FLOAT },
            { "float3", rhi::Format::R32G32B32_FLOAT },
            { "float2", rhi::Format::R32G32_FLOAT },
            { "float", rhi::Format::R32_FLOAT },
            { "uint4", rhi::Format::R32G32B32A32_UINT },
            { "uint3", rhi::Format::R32G32B32_UINT },
            { "uint2", rhi::Format::R32G32_UINT },
            { "uint", rhi::Format::R32_UINT },
        };

        rhi::Format ParseFormat(const std::string& str)
        {
            auto it = FORMAT.find(str);
            if (it != FORMAT.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the tech description", str);
            return FORMAT.begin()->second;
        }

        // Blend Description

        const std::map<std::string, rhi::Blend> BLEND =
        {
            { "D3D12_BLEND_ZERO", rhi::Blend::Zero },
            { "D3D12_BLEND_ONE", rhi::Blend::One },
            { "D3D12_BLEND_SRC_COLOR", rhi::Blend::SrcColor },
            { "D3D12_BLEND_INV_SRC_COLOR", rhi::Blend::InvSrcColor },
            { "D3D12_BLEND_SRC_ALPHA", rhi::Blend::SrcAlpha },
            { "D3D12_BLEND_INV_SRC_ALPHA", rhi::Blend::InvSrcAlpha },
            { "D3D12_BLEND_DEST_ALPHA", rhi::Blend::DestAlpha },
            { "D3D12_BLEND_INV_DEST_ALPHA", rhi::Blend::InvDestAlpha },
            { "D3D12_BLEND_DEST_COLOR", rhi::Blend::DestColor },
            { "D3D12_BLEND_INV_DEST_COLOR", rhi::Blend::InvDestColor },
            { "D3D12_BLEND_SRC_ALPHA_SAT", rhi::Blend::SrcAlphaSat },
            { "D3D12_BLEND_BLEND_FACTOR", rhi::Blend::BlendFactor },
            { "D3D12_BLEND_INV_BLEND_FACTOR", rhi::Blend::InvBlendFactor },
            { "D3D12_BLEND_SRC1_COLOR", rhi::Blend::Src1Color },
            { "D3D12_BLEND_INV_SRC1_COLOR", rhi::Blend::InvSrc1Color },
            { "D3D12_BLEND_SRC1_ALPHA", rhi::Blend::Src1Alpha },
            { "D3D12_BLEND_INV_SRC1_ALPHA", rhi::Blend::InvSrc1Alpha },
            { "D3D12_BLEND_ALPHA_FACTOR", rhi::Blend::AlphaFactor },
            { "D3D12_BLEND_INV_ALPHA_FACTOR", rhi::Blend::InvAlphaFactor }
        };

        const std::map<std::string, rhi::BlendOpType> BLEND_OP =
        {
            { "D3D12_BLEND_OP_ADD", rhi::BlendOpType::Add },
            { "D3D12_BLEND_OP_ADD", rhi::BlendOpType::Subtract },
            { "D3D12_BLEND_OP_ADD", rhi::BlendOpType::RevSubtract },
            { "D3D12_BLEND_OP_ADD", rhi::BlendOpType::Min },
            { "D3D12_BLEND_OP_ADD", rhi::BlendOpType::Max }
        };

        const std::map<std::string, rhi::ColorWriteEnable> COLOR_WRITE =
        {
            { "D3D12_COLOR_WRITE_DISABLE", rhi::ColorWriteEnable::DisableAll },
            { "D3D12_COLOR_WRITE_ENABLE_RED", rhi::ColorWriteEnable::Red },
            { "D3D12_COLOR_WRITE_ENABLE_GREEN", rhi::ColorWriteEnable::Green },
            { "D3D12_COLOR_WRITE_ENABLE_BLUE", rhi::ColorWriteEnable::Blue },
            { "D3D12_COLOR_WRITE_ENABLE_ALPHA", rhi::ColorWriteEnable::Alpha },
            { "D3D12_COLOR_WRITE_ENABLE_ALL", rhi::ColorWriteEnable::All }
        };

        const std::map<std::string, rhi::LogicOp> LOGIC_OP =
        {
            { "D3D12_LOGIC_OP_CLEAR", rhi::LogicOp::Clear },
            { "D3D12_LOGIC_OP_SET", rhi::LogicOp::Set },
            { "D3D12_LOGIC_OP_COPY", rhi::LogicOp::Copy },
            { "D3D12_LOGIC_OP_COPY_INVERTED", rhi::LogicOp::CopyInverted },
            { "D3D12_LOGIC_OP_NOOP", rhi::LogicOp::NoOp },
            { "D3D12_LOGIC_OP_INVERT", rhi::LogicOp::Invert },
            { "D3D12_LOGIC_OP_AND", rhi::LogicOp::And },
            { "D3D12_LOGIC_OP_NAND", rhi::LogicOp::Nand },
            { "D3D12_LOGIC_OP_OR", rhi::LogicOp::Or },
            { "D3D12_LOGIC_OP_NOR", rhi::LogicOp::Nor },
            { "D3D12_LOGIC_OP_XOR", rhi::LogicOp::Xor },
            { "D3D12_LOGIC_OP_EQUIV", rhi::LogicOp::Equiv },
            { "D3D12_LOGIC_OP_AND_REVERSE", rhi::LogicOp::AndReverse },
            { "D3D12_LOGIC_OP_AND_INVERTED", rhi::LogicOp::AndInverted },
            { "D3D12_LOGIC_OP_OR_REVERSE", rhi::LogicOp::OrReverse },
            { "D3D12_LOGIC_OP_OR_INVERTED", rhi::LogicOp::OrInverted }
        };

        rhi::Blend ParseBlend(const std::string& str)
        {
            auto it = BLEND.find(str);
            if (it != BLEND.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the blend description", str);
            return BLEND.begin()->second;
        }

        rhi::BlendOpType ParseBlendOp(const std::string& str)
        {
            auto it = BLEND_OP.find(str);
            if (it != BLEND_OP.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the blend description", str);
            return BLEND_OP.begin()->second;
        }

        rhi::ColorWriteEnable ParseColorWriteEnable(const std::string& str)
        {
            auto it = COLOR_WRITE.find(str);
            if (it != COLOR_WRITE.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the blend description", str);
            return COLOR_WRITE.begin()->second;
        }

        rhi::LogicOp ParseLogicOp(const std::string& str)
        {
            auto it = LOGIC_OP.find(str);
            if (it != LOGIC_OP.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the blend description", str);
            return LOGIC_OP.begin()->second;
        }

        // Rasterizer Description

        const std::map<std::string, rhi::FillMode> FILL_MODE =
        {
            { "D3D12_FILL_MODE_WIREFRAME", rhi::FillMode::Wireframe },
            { "D3D12_FILL_MODE_SOLID", rhi::FillMode::Solid }
        };

        const std::map<std::string, rhi::CullMode> CULL_MODE =
        {
            { "D3D12_CULL_MODE_NONE", rhi::CullMode::None },
            { "D3D12_CULL_MODE_FRONT", rhi::CullMode::Front },
            { "D3D12_CULL_MODE_BACK", rhi::CullMode::Back }
        };

        rhi::FillMode ParseFillMode(const std::string& str)
        {
            auto it = FILL_MODE.find(str);
            if (it != FILL_MODE.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the raster description", str);
            return FILL_MODE.begin()->second;
        }

        rhi::CullMode ParseCullMode(const std::string& str)
        {
            auto it = CULL_MODE.find(str);
            if (it != CULL_MODE.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the raster description", str);
            return CULL_MODE.begin()->second;
        }

        // Depth Stencil Description

        const std::map<std::string, rhi::ComparisonFunc> COMPARISON_FUNC =
        {
            { "D3D12_COMPARISON_FUNC_NEVER", rhi::ComparisonFunc::Never },
            { "D3D12_COMPARISON_FUNC_LESS", rhi::ComparisonFunc::Less },
            { "D3D12_COMPARISON_FUNC_EQUAL", rhi::ComparisonFunc::Equal },
            { "D3D12_COMPARISON_FUNC_LESS_EQUAL", rhi::ComparisonFunc::LessEqual },
            { "D3D12_COMPARISON_FUNC_GREATER", rhi::ComparisonFunc::Greater },
            { "D3D12_COMPARISON_FUNC_NOT_EQUAL", rhi::ComparisonFunc::NotEqual },
            { "D3D12_COMPARISON_FUNC_GREATER_EQUAL", rhi::ComparisonFunc::GreaterEqual },
            { "D3D12_COMPARISON_FUNC_ALWAYS", rhi::ComparisonFunc::Always }
        };

        const std::map<std::string, rhi::DepthWriteMask> DEPTH_WRITE_MASK =
        {
            { "D3D12_DEPTH_WRITE_MASK_ZERO", rhi::DepthWriteMask::Zero },
            { "D3D12_DEPTH_WRITE_MASK_ALL", rhi::DepthWriteMask::All }
        };

        rhi::ComparisonFunc ParseComparisonFunc(const std::string& str)
        {
            auto it = COMPARISON_FUNC.find(str);
            if (it != COMPARISON_FUNC.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the depth stencil description", str);
            return COMPARISON_FUNC.begin()->second;
        }

        rhi::DepthWriteMask ParseDepthWriteMask(const std::string& str)
        {
            auto it = DEPTH_WRITE_MASK.find(str);
            if (it != DEPTH_WRITE_MASK.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the depth stencil description", str);
            return DEPTH_WRITE_MASK.begin()->second;
        }

        const std::map<std::string, D3D12_PRIMITIVE_TOPOLOGY_TYPE> TOPOLOGY_TYPE =
        {
            { "point", D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT},
            { "line", D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE },
            { "triangle", D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE }
        };

        D3D12_PRIMITIVE_TOPOLOGY_TYPE ParseTopologyType(const std::string& str)
        {
            auto it = TOPOLOGY_TYPE.find(str);
            if (it != TOPOLOGY_TYPE.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the topology type description", str);
            return TOPOLOGY_TYPE.begin()->second;
        }

        // TODO: add FORMAT types
        const std::map<std::string, DXGI_FORMAT> TEX_FORMAT =
        {
            { "R8G8B8A8_UNORM", DXGI_FORMAT_R8G8B8A8_UNORM },
            { "R8G8B8A8_SNORM", DXGI_FORMAT_R8G8B8A8_SNORM },
            { "R32G32B32A32_FLOAT", DXGI_FORMAT_R32G32B32A32_FLOAT },
            { "R11G11B10_FLOAT", DXGI_FORMAT_R11G11B10_FLOAT },
            { "D32_FLOAT", DXGI_FORMAT_D32_FLOAT },
            { "R32_FLOAT", DXGI_FORMAT_R32_FLOAT }
        };

        DXGI_FORMAT ParseTexFormat(const std::string& str)
        {
            auto it = TEX_FORMAT.find(str);
            if (it != TEX_FORMAT.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the texture format description", str);
            return TEX_FORMAT.begin()->second;
        }
    } // namespace unnamed

    D3D12PipelineState::D3D12PipelineState(rhi::Device* device)
        : _rootSignature(nullptr)
        , _pipelineState(nullptr)
        , _isGraphicsPipeline(false)
        , _device(device)
    {
    }

    D3D12PipelineState::D3D12PipelineState(D3D12PipelineState&& other) noexcept
        : _rootSignature(std::move(other._rootSignature))
        , _pipelineState(std::move(other._pipelineState))
        , _isGraphicsPipeline(other._isGraphicsPipeline)
        , _device(other._device)
    {
    }

    D3D12PipelineState::~D3D12PipelineState()
    {
        _rootSignature = nullptr;
        _pipelineState = nullptr;
    }

    D3D12PipelineState& D3D12PipelineState::operator=(D3D12PipelineState&& other) noexcept
    {
        if (this != &other)
        {
            _rootSignature = std::move(other._rootSignature);
            _pipelineState = std::move(other._pipelineState);
            _isGraphicsPipeline = other._isGraphicsPipeline;
            _device = other._device;
        }

        return *this;
    }

    ComPtr<ID3D12RootSignature> D3D12PipelineState::GetRootSignature() const
    {
        return _rootSignature;
    }

    ComPtr<ID3D12PipelineState> D3D12PipelineState::GetPipelineState() const
    {
        return _pipelineState;
    }

    void D3D12PipelineState::Parse(const std::string& filepath)
    {
        Json::Value jsonRoot = ParseJson(filepath);
        ASSERT(!jsonRoot.isNull() || jsonRoot.empty(), "Failed to parse JSON from file: " + filepath);

        _isGraphicsPipeline = jsonRoot["IsGraphicsPipeline"].asBool();

        if (_isGraphicsPipeline)
        {
            ParseGraphicsPipeline(jsonRoot);
        }
        else
        {
            ParseComputePipeline(jsonRoot);
        }
    }

    void D3D12PipelineState::ParseGraphicsPipeline(const Json::Value& fileRoot)
    {
        ID3D12Device* nativeDevice = D3D12Cast<ID3D12Device>(_device->GetNative());

        // Load the vertex shader
        ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
        if (!fileRoot["VS"].isNull())
        {
            std::string vertexShaderFilepath = fileRoot["VS"].asCString();
            HRESULT result = D3DReadFileToBlob(std::wstring(vertexShaderFilepath.begin(), vertexShaderFilepath.end()).c_str(), &vertexShaderBlob);
            CHECK(result, "Failed to load vertex shader from file: " + vertexShaderFilepath);
        }

        // Load the geometry shader
        ComPtr<ID3DBlob> geometryShaderBlob = nullptr;
        if (!fileRoot["GS"].isNull())
        {
            std::string geometryShaderFilepath = fileRoot["GS"].asCString();
            HRESULT result = D3DReadFileToBlob(std::wstring(geometryShaderFilepath.begin(), geometryShaderFilepath.end()).c_str(), &geometryShaderBlob);
            CHECK(result, "Failed to load geometry shader from file: " + geometryShaderFilepath);
        }

        // Load the pixel shader
        ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
        if (!fileRoot["PS"].isNull())
        {
            std::string pixelShaderFilepath = fileRoot["PS"].asCString();
            HRESULT result = D3DReadFileToBlob(std::wstring(pixelShaderFilepath.begin(), pixelShaderFilepath.end()).c_str(), &pixelShaderBlob);
            CHECK(result, "Failed to load pixel shader from file: " + pixelShaderFilepath);
        }

        // Create the vertex input layout
        unsigned int layoutElementsNum = fileRoot["Layout"].size();
        D3D12_INPUT_ELEMENT_DESC* inputLayout = nullptr;
        std::vector<std::string> names(layoutElementsNum);
        if (!fileRoot["Layout"].isNull())
        {
            inputLayout = new D3D12_INPUT_ELEMENT_DESC[layoutElementsNum];
            for (unsigned int i = 0; i < layoutElementsNum; ++i)
            {
                inputLayout[i] = {};
                Json::Value layout = fileRoot["Layout"][i];
                names[i] = layout["Name"].asCString();
                inputLayout[i].SemanticName = names[i].c_str();
                inputLayout[i].SemanticIndex = layout["SemanticIndex"].asUInt();
                inputLayout[i].Format = GetDXGIFormat(ParseFormat(layout["Format"].asCString()));
                inputLayout[i].AlignedByteOffset = layout["Offset"].asUInt();
                inputLayout[i].InputSlot = layout["Slot"].asUInt();
            }
        }

        // Create the root signature
        {
            HRESULT result = nativeDevice->CreateRootSignature(0, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
            CHECK(result, "Failed to create root signature from vertex shader blob.");
        }

        // Set the name of the root signature
        {
            std::string type = fileRoot["Type"].asCString();
            std::wstring name(type.begin(), type.end());
            _rootSignature->SetName(name.c_str());
        }

        // Create the graphics pipeline state description
        const std::string blendPipelineDescFilepath = fileRoot["Blend"].asCString();
        const std::string rasterPipelineDescFilepath = fileRoot["Raster"].asCString();
        const std::string depthPipelineDescFilepath = fileRoot["Depth"].asCString();

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDescription = {};

        // Set the pipeline state description properties
        {
            pipelineStateDescription.BlendState = GetD3D12BlendDesc(ParseBlendDescription(blendPipelineDescFilepath));
            pipelineStateDescription.RasterizerState = GetD3D12RasterizerDesc(ParseRasterizerDescription(rasterPipelineDescFilepath));
            pipelineStateDescription.DepthStencilState = GetD3D12DepthStencilDesc(ParseDepthStencilDescription(depthPipelineDescFilepath));
            pipelineStateDescription.pRootSignature = _rootSignature.Get();
            pipelineStateDescription.InputLayout = { inputLayout, layoutElementsNum };
            pipelineStateDescription.PrimitiveTopologyType = ParseTopologyType(fileRoot["TopologyType"].asCString());
            if (vertexShaderBlob)
            {
                pipelineStateDescription.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
            }
            if (geometryShaderBlob)
            {
                pipelineStateDescription.GS = CD3DX12_SHADER_BYTECODE(geometryShaderBlob.Get());
            }
            if (pixelShaderBlob)
            {
                pipelineStateDescription.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
            }
            pipelineStateDescription.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            Json::Value renderTargets = fileRoot["RenderTargets"];
            pipelineStateDescription.NumRenderTargets = renderTargets.size();
            for (unsigned int i = 0; i < renderTargets.size(); ++i)
            {
                pipelineStateDescription.RTVFormats[i] = ParseTexFormat(renderTargets[i].asCString());
            }
            pipelineStateDescription.SampleDesc.Count = 1; // must be the same sample description as the swapChain and depth/stencil buffer
            pipelineStateDescription.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
        }

        // Create the graphics pipeline state object
        {
            HRESULT result = nativeDevice->CreateGraphicsPipelineState(&pipelineStateDescription, IID_PPV_ARGS(&_pipelineState));
            CHECK(result, "Failed to create graphics pipeline state from the description.");
        }

        // Set the name of the pipeline state object
        {
            std::string type = fileRoot["Type"].asCString();
            std::wstring name(type.begin(), type.end());
            _pipelineState->SetName(name.c_str());
        }

        delete[] inputLayout;
    }

    void D3D12PipelineState::ParseComputePipeline(const Json::Value& fileRoot)
    {
        ID3D12Device* nativeDevice = D3D12Cast<ID3D12Device>(_device->GetNative());

        // Load the compute shader
        ComPtr<ID3DBlob> computeShaderBlob = nullptr;
        if (!fileRoot["CS"].isNull())
        {
            std::string computeShaderFilepath = fileRoot["CS"].asCString();
            HRESULT result = D3DReadFileToBlob(std::wstring(computeShaderFilepath.begin(), computeShaderFilepath.end()).c_str(), &computeShaderBlob);
            CHECK(result, "Failed to load compute shader from file: " + computeShaderFilepath);
        }

        // Create the root signature
        {
            HRESULT result = nativeDevice->CreateRootSignature(0, computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
            CHECK(result, "Failed to create root signature from compute shader blob.");
        }

        // Set the name of the root signature
        {
            std::string type = fileRoot["Type"].asCString();
            std::wstring name(type.begin(), type.end());
            _rootSignature->SetName(name.c_str());
        }

        // Create the compute pipeline state description
        D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDescription = {};
        pipelineStateDescription.pRootSignature = _rootSignature.Get();
        if (computeShaderBlob)
        {
            pipelineStateDescription.CS = CD3DX12_SHADER_BYTECODE(computeShaderBlob.Get());
        }

        // Create the compute pipeline state object
        {
            HRESULT result = nativeDevice->CreateComputePipelineState(&pipelineStateDescription, IID_PPV_ARGS(&_pipelineState));
            CHECK(result, "Failed to create compute pipeline state from the description.");
        }

        // Set the name of the pipeline state object
        {
            std::string type = fileRoot["Type"].asCString();
            std::wstring name(type.begin(), type.end());
            _pipelineState->SetName(name.c_str());
        }
    }

    PipelineStateType D3D12PipelineState::GetType() const
    {
        NOT_IMPLEMENTED();
        return PipelineStateType();
    }

    rhi::BlendState D3D12PipelineState::ParseBlendDescription(const std::string& filepath)
    {
        Json::Value root = ParseJson(filepath);
        ASSERT(!root.isNull() || root.empty(), "Failed to parse JSON from file: " + filepath);

        rhi::BlendState description = {};

        int renderTargetsSize = root["RenderTargets"].size();
        for (int i = 0; i < renderTargetsSize; ++i)
        {
            Json::Value target = root["RenderTargets"][i];
            description.RenderTargets[i].BlendEnable = target["BlendEnable"].asBool();
            description.RenderTargets[i].SrcBlend = ParseBlend(target["SrcBlend"].asCString());
            description.RenderTargets[i].DestBlend = ParseBlend(target["DestBlend"].asCString());
            description.RenderTargets[i].BlendOp = ParseBlendOp(target["BlendOp"].asCString());

            description.RenderTargets[i].SrcBlendAlpha = ParseBlend(target["SrcBlendAlpha"].asCString());
            description.RenderTargets[i].DestBlendAlpha = ParseBlend(target["DestBlendAlpha"].asCString());
            description.RenderTargets[i].BlendOpAlpha = ParseBlendOp(target["BlendOpAlpha"].asCString());

            description.RenderTargets[i].RenderTargetWriteMask = ParseColorWriteEnable(target["RenderTargetWriteMask"].asCString());

            description.RenderTargets[i].LogicOpEnable = target["LogicOpEnable"].asBool();
            description.RenderTargets[i].LogicOp = ParseLogicOp(target["LogicOp"].asCString());
        }

        return description;
    }

    rhi::RasterizerState D3D12PipelineState::ParseRasterizerDescription(const std::string& filepath)
    {
        Json::Value root = ParseJson(filepath);
        ASSERT(!root.isNull() || root.empty(), "Failed to parse JSON from file: " + filepath);

        rhi::RasterizerState description = {};
        description.FillMode = ParseFillMode(root["FillMode"].asCString());
        description.CullMode = ParseCullMode(root["CullMode"].asCString());
        description.DepthClipEnable = root["DepthClipEnable"].asBool();

        return description;
    }

    rhi::DepthStencilState D3D12PipelineState::ParseDepthStencilDescription(const std::string& filepath)
    {
        Json::Value root = ParseJson(filepath);
        ASSERT(!root.isNull() || root.empty(), "Failed to parse JSON from file: " + filepath);

        rhi::DepthStencilState description = {};
        description.DepthEnable = root["DepthEnable"].asBool();
        description.DepthFunc = ParseComparisonFunc(root["DepthFunc"].asCString());
        description.DepthWriteMask = ParseDepthWriteMask(root["DepthWriteMask"].asCString());
        description.StencilEnable = root["StencilEnable"].asBool();

        return description;
    }

    void* D3D12PipelineState::GetNative() const
    {
        return static_cast<void*>(_pipelineState.Get());
    }
} // namespace rhi::d3d12
