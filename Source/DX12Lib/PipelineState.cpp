#include "DX12LibPCH.h"

#include "PipelineState.h"

#include <json/json.h>

#include <filesystem>
#include <fstream>

namespace dx12
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
        const std::map<std::string, DXGI_FORMAT> FORMAT =
        {
            { "float4", DXGI_FORMAT_R32G32B32A32_FLOAT },
            { "float3", DXGI_FORMAT_R32G32B32_FLOAT },
            { "float2", DXGI_FORMAT_R32G32_FLOAT },
            { "float", DXGI_FORMAT_R32_FLOAT },
            { "uint4", DXGI_FORMAT_R32G32B32A32_UINT },
            { "uint3", DXGI_FORMAT_R32G32B32_UINT },
            { "uint2", DXGI_FORMAT_R32G32_UINT },
            { "uint", DXGI_FORMAT_R32_UINT },
        };

        DXGI_FORMAT ParseFormat(const std::string& str)
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

        const std::map<std::string, D3D12_BLEND> DEPTH =
        {
            { "D3D12_BLEND_ZERO", D3D12_BLEND_ZERO },
            { "D3D12_BLEND_ONE", D3D12_BLEND_ONE },
            { "D3D12_BLEND_SRC_COLOR", D3D12_BLEND_SRC_COLOR },
            { "D3D12_BLEND_INV_SRC_COLOR", D3D12_BLEND_INV_SRC_COLOR },
            { "D3D12_BLEND_SRC_ALPHA", D3D12_BLEND_SRC_ALPHA },
            { "D3D12_BLEND_INV_SRC_ALPHA", D3D12_BLEND_INV_SRC_ALPHA },
            { "D3D12_BLEND_DEST_ALPHA", D3D12_BLEND_DEST_ALPHA },
            { "D3D12_BLEND_INV_DEST_ALPHA", D3D12_BLEND_INV_DEST_ALPHA },
            { "D3D12_BLEND_DEST_COLOR", D3D12_BLEND_DEST_COLOR },
            { "D3D12_BLEND_INV_DEST_COLOR", D3D12_BLEND_INV_DEST_COLOR },
            { "D3D12_BLEND_SRC_ALPHA_SAT", D3D12_BLEND_SRC_ALPHA_SAT },
            { "D3D12_BLEND_BLEND_FACTOR", D3D12_BLEND_BLEND_FACTOR },
            { "D3D12_BLEND_INV_BLEND_FACTOR", D3D12_BLEND_INV_BLEND_FACTOR },
            { "D3D12_BLEND_SRC1_COLOR", D3D12_BLEND_SRC1_COLOR },
            { "D3D12_BLEND_INV_SRC1_COLOR", D3D12_BLEND_INV_SRC1_COLOR },
            { "D3D12_BLEND_SRC1_ALPHA", D3D12_BLEND_SRC1_ALPHA },
            { "D3D12_BLEND_INV_SRC1_ALPHA", D3D12_BLEND_INV_SRC1_ALPHA },
            { "D3D12_BLEND_ALPHA_FACTOR", D3D12_BLEND_ALPHA_FACTOR },
            { "D3D12_BLEND_INV_ALPHA_FACTOR", D3D12_BLEND_INV_ALPHA_FACTOR }
        };

        const std::map<std::string, D3D12_BLEND_OP> BLEND_OP =
        {
            { "D3D12_BLEND_OP_ADD", D3D12_BLEND_OP_ADD },
            { "D3D12_BLEND_OP_ADD", D3D12_BLEND_OP_SUBTRACT },
            { "D3D12_BLEND_OP_ADD", D3D12_BLEND_OP_REV_SUBTRACT },
            { "D3D12_BLEND_OP_ADD", D3D12_BLEND_OP_MIN },
            { "D3D12_BLEND_OP_ADD", D3D12_BLEND_OP_MAX }
        };

        const std::map<std::string, D3D12_COLOR_WRITE_ENABLE> COLOR_WRITE =
        {
            { "D3D12_COLOR_WRITE_DISABLE", (D3D12_COLOR_WRITE_ENABLE)0 },
            { "D3D12_COLOR_WRITE_ENABLE_RED", D3D12_COLOR_WRITE_ENABLE_RED },
            { "D3D12_COLOR_WRITE_ENABLE_GREEN", D3D12_COLOR_WRITE_ENABLE_GREEN },
            { "D3D12_COLOR_WRITE_ENABLE_BLUE", D3D12_COLOR_WRITE_ENABLE_BLUE },
            { "D3D12_COLOR_WRITE_ENABLE_ALPHA", D3D12_COLOR_WRITE_ENABLE_ALPHA },
            { "D3D12_COLOR_WRITE_ENABLE_ALL", D3D12_COLOR_WRITE_ENABLE_ALL }
        };

        const std::map<std::string, D3D12_LOGIC_OP> LOGIC_OP =
        {
            { "D3D12_LOGIC_OP_CLEAR", D3D12_LOGIC_OP_CLEAR },
            { "D3D12_LOGIC_OP_SET", D3D12_LOGIC_OP_SET },
            { "D3D12_LOGIC_OP_COPY", D3D12_LOGIC_OP_COPY },
            { "D3D12_LOGIC_OP_COPY_INVERTED", D3D12_LOGIC_OP_COPY_INVERTED },
            { "D3D12_LOGIC_OP_NOOP", D3D12_LOGIC_OP_NOOP },
            { "D3D12_LOGIC_OP_INVERT", D3D12_LOGIC_OP_INVERT },
            { "D3D12_LOGIC_OP_AND", D3D12_LOGIC_OP_AND },
            { "D3D12_LOGIC_OP_NAND", D3D12_LOGIC_OP_NAND },
            { "D3D12_LOGIC_OP_OR", D3D12_LOGIC_OP_OR },
            { "D3D12_LOGIC_OP_NOR", D3D12_LOGIC_OP_NOR },
            { "D3D12_LOGIC_OP_XOR", D3D12_LOGIC_OP_XOR },
            { "D3D12_LOGIC_OP_EQUIV", D3D12_LOGIC_OP_EQUIV },
            { "D3D12_LOGIC_OP_AND_REVERSE", D3D12_LOGIC_OP_AND_REVERSE },
            { "D3D12_LOGIC_OP_AND_INVERTED", D3D12_LOGIC_OP_AND_INVERTED },
            { "D3D12_LOGIC_OP_OR_REVERSE", D3D12_LOGIC_OP_OR_REVERSE },
            { "D3D12_LOGIC_OP_OR_INVERTED", D3D12_LOGIC_OP_OR_INVERTED }
        };

        D3D12_BLEND ParseBlend(const std::string& str)
        {
            auto it = DEPTH.find(str);
            if (it != DEPTH.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the blend description", str);
            return DEPTH.begin()->second;
        }

        D3D12_BLEND_OP ParseBlendOp(const std::string& str)
        {
            auto it = BLEND_OP.find(str);
            if (it != BLEND_OP.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the blend description", str);
            return BLEND_OP.begin()->second;
        }

        D3D12_COLOR_WRITE_ENABLE ParseColorWriteEnable(const std::string& str)
        {
            auto it = COLOR_WRITE.find(str);
            if (it != COLOR_WRITE.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the blend description", str);
            return COLOR_WRITE.begin()->second;
        }

        D3D12_LOGIC_OP ParseLogicOp(const std::string& str)
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

        const std::map<std::string, D3D12_FILL_MODE> FILL_MODE =
        {
            { "D3D12_FILL_MODE_WIREFRAME", D3D12_FILL_MODE_WIREFRAME },
            { "D3D12_FILL_MODE_SOLID", D3D12_FILL_MODE_SOLID }
        };

        const std::map<std::string, D3D12_CULL_MODE> CULL_MODE =
        {
            { "D3D12_CULL_MODE_NONE", D3D12_CULL_MODE_NONE },
            { "D3D12_CULL_MODE_FRONT", D3D12_CULL_MODE_FRONT },
            { "D3D12_CULL_MODE_BACK", D3D12_CULL_MODE_BACK }
        };

        D3D12_FILL_MODE ParseFillMode(const std::string& str)
        {
            auto it = FILL_MODE.find(str);
            if (it != FILL_MODE.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the raster description", str);
            return FILL_MODE.begin()->second;
        }

        D3D12_CULL_MODE ParseCullMode(const std::string& str)
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

        const std::map<std::string, D3D12_COMPARISON_FUNC> COMPARISON_FUNC =
        {
            { "D3D12_COMPARISON_FUNC_NEVER", D3D12_COMPARISON_FUNC_NEVER },
            { "D3D12_COMPARISON_FUNC_LESS", D3D12_COMPARISON_FUNC_LESS },
            { "D3D12_COMPARISON_FUNC_EQUAL", D3D12_COMPARISON_FUNC_EQUAL },
            { "D3D12_COMPARISON_FUNC_LESS_EQUAL", D3D12_COMPARISON_FUNC_LESS_EQUAL },
            { "D3D12_COMPARISON_FUNC_GREATER", D3D12_COMPARISON_FUNC_GREATER },
            { "D3D12_COMPARISON_FUNC_NOT_EQUAL", D3D12_COMPARISON_FUNC_NOT_EQUAL },
            { "D3D12_COMPARISON_FUNC_GREATER_EQUAL", D3D12_COMPARISON_FUNC_GREATER_EQUAL },
            { "D3D12_COMPARISON_FUNC_ALWAYS", D3D12_COMPARISON_FUNC_ALWAYS }
        };

        const std::map<std::string, D3D12_DEPTH_WRITE_MASK> DEPTH_WRITE_MASK =
        {
            { "D3D12_DEPTH_WRITE_MASK_ZERO", D3D12_DEPTH_WRITE_MASK_ZERO },
            { "D3D12_DEPTH_WRITE_MASK_ALL", D3D12_DEPTH_WRITE_MASK_ALL }
        };

        D3D12_COMPARISON_FUNC ParseComparisonFunc(const std::string& str)
        {
            auto it = COMPARISON_FUNC.find(str);
            if (it != COMPARISON_FUNC.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the depth stencil description", str);
            return COMPARISON_FUNC.begin()->second;
        }

        D3D12_DEPTH_WRITE_MASK ParseDepthWriteMask(const std::string& str)
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

    PipelineState::PipelineState()
        : _rootSignature(nullptr)
        , _pipelineState(nullptr)
        , _isGraphicsPipeline(false)
    {
    }

    PipelineState::PipelineState(const PipelineState& other)
        : _rootSignature(other._rootSignature)
        , _pipelineState(other._pipelineState)
        , _isGraphicsPipeline(other._isGraphicsPipeline)
    {
    }

    PipelineState::PipelineState(PipelineState&& other) noexcept
        : _rootSignature(std::move(other._rootSignature))
        , _pipelineState(std::move(other._pipelineState))
        , _isGraphicsPipeline(other._isGraphicsPipeline)
    {
    }

    PipelineState::~PipelineState()
    {
        _rootSignature = nullptr;
        _pipelineState = nullptr;
    }

    PipelineState& PipelineState::operator=(const PipelineState& other)
    {
        if (this != &other)
        {
            _rootSignature = other._rootSignature;
            _pipelineState = other._pipelineState;
            _isGraphicsPipeline = other._isGraphicsPipeline;
        }

        return *this;
    }

    PipelineState& PipelineState::operator=(PipelineState&& other) noexcept
    {
        if (this != &other)
        {
            _rootSignature = std::move(other._rootSignature);
            _pipelineState = std::move(other._pipelineState);
            _isGraphicsPipeline = other._isGraphicsPipeline;
        }

        return *this;
    }

    ComPtr<ID3D12RootSignature> PipelineState::GetRootSignature() const
    {
        return _rootSignature;
    }

    ComPtr<ID3D12PipelineState> PipelineState::GetPipelineState() const
    {
        return _pipelineState;
    }

    bool PipelineState::IsGraphicsPipeline() const
    {
        return _isGraphicsPipeline;
    }

    void PipelineState::Parse(const std::string& filepath)
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

    void PipelineState::ParseGraphicsPipeline(const Json::Value& fileRoot)
    {
        ComPtr<ID3D12Device> device = dx12::Device::GetDXDevice();

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
                inputLayout[i].Format = ParseFormat(layout["Format"].asCString());
                inputLayout[i].AlignedByteOffset = layout["Offset"].asUInt();
                inputLayout[i].InputSlot = layout["Slot"].asUInt();
            }
        }

        // Create the root signature
        {
            HRESULT result = device->CreateRootSignature(0, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
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
            pipelineStateDescription.BlendState = ParseBlendDescription(blendPipelineDescFilepath);
            pipelineStateDescription.RasterizerState = ParseRasterizerDescription(rasterPipelineDescFilepath);
            pipelineStateDescription.DepthStencilState = ParseDepthStencilDescription(depthPipelineDescFilepath);
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
            HRESULT result = device->CreateGraphicsPipelineState(&pipelineStateDescription, IID_PPV_ARGS(&_pipelineState));
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

    void PipelineState::ParseComputePipeline(const Json::Value& fileRoot)
    {
        ComPtr<ID3D12Device> device = dx12::Device::GetDXDevice();

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
            HRESULT result = device->CreateRootSignature(0, computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
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
            HRESULT result = device->CreateComputePipelineState(&pipelineStateDescription, IID_PPV_ARGS(&_pipelineState));
            CHECK(result, "Failed to create compute pipeline state from the description.");
        }

        // Set the name of the pipeline state object
        {
            std::string type = fileRoot["Type"].asCString();
            std::wstring name(type.begin(), type.end());
            _pipelineState->SetName(name.c_str());
        }
    }

    D3D12_BLEND_DESC PipelineState::ParseBlendDescription(const std::string& filepath)
    {
        Json::Value root = ParseJson(filepath);
        ASSERT(!root.isNull() || root.empty(), "Failed to parse JSON from file: " + filepath);

        D3D12_BLEND_DESC description = {};

        int renderTargetsSize = root["RenderTargets"].size();
        for (int i = 0; i < renderTargetsSize; ++i)
        {
            Json::Value target = root["RenderTargets"][i];
            description.RenderTarget[i].BlendEnable = target["BlendEnable"].asBool();
            description.RenderTarget[i].SrcBlend = ParseBlend(target["SrcBlend"].asCString());
            description.RenderTarget[i].DestBlend = ParseBlend(target["DestBlend"].asCString());
            description.RenderTarget[i].BlendOp = ParseBlendOp(target["BlendOp"].asCString());

            description.RenderTarget[i].SrcBlendAlpha = ParseBlend(target["SrcBlendAlpha"].asCString());
            description.RenderTarget[i].DestBlendAlpha = ParseBlend(target["DestBlendAlpha"].asCString());
            description.RenderTarget[i].BlendOpAlpha = ParseBlendOp(target["BlendOpAlpha"].asCString());

            description.RenderTarget[i].RenderTargetWriteMask = static_cast<UINT8>(ParseColorWriteEnable(target["RenderTargetWriteMask"].asCString()));

            description.RenderTarget[i].LogicOpEnable = target["LogicOpEnable"].asBool();
            description.RenderTarget[i].LogicOp = ParseLogicOp(target["LogicOp"].asCString());
        }

        return description;
    }

    D3D12_RASTERIZER_DESC PipelineState::ParseRasterizerDescription(const std::string& filepath)
    {
        Json::Value root = ParseJson(filepath);
        ASSERT(!root.isNull() || root.empty(), "Failed to parse JSON from file: " + filepath);

        D3D12_RASTERIZER_DESC description = {};
        description.FillMode = ParseFillMode(root["FillMode"].asCString());
        description.CullMode = ParseCullMode(root["CullMode"].asCString());
        description.DepthClipEnable = root["DepthClipEnable"].asBool();

        return description;
    }

    D3D12_DEPTH_STENCIL_DESC PipelineState::ParseDepthStencilDescription(const std::string& filepath)
    {
        Json::Value root = ParseJson(filepath);
        ASSERT(!root.isNull() || root.empty(), "Failed to parse JSON from file: " + filepath);

        D3D12_DEPTH_STENCIL_DESC description = {};
        description.DepthEnable = root["DepthEnable"].asBool();
        description.DepthFunc = ParseComparisonFunc(root["DepthFunc"].asCString());
        description.DepthWriteMask = ParseDepthWriteMask(root["DepthWriteMask"].asCString());
        description.StencilEnable = root["StencilEnable"].asBool();

        return description;
    }
} // namespace dx12
