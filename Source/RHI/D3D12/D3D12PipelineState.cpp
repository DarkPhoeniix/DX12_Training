
#include "RHI_PCH.h"

#include "D3D12PipelineState.h"

#include "D3D12Device.h"
#include "D3D12Helpers.h"

#include "PipelineHelpers.h"

#include <json/json.h>

#include <filesystem>
#include <fstream>

namespace rhi::d3d12
{
    namespace
    {
        static const std::string kShaderNameExtension = ".spv";

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

    D3D12PipelineState::D3D12PipelineState(Device* device, const std::string& filepath)
        : _rootSignature(nullptr)
        , _pipelineState(nullptr)
        , _type(PipelineStateType::Graphics)
        , _device(device)
    {
        Parse(filepath);
    }

    D3D12PipelineState::D3D12PipelineState(D3D12PipelineState&& other) noexcept
        : _rootSignature(std::move(other._rootSignature))
        , _pipelineState(std::move(other._pipelineState))
        , _type(other._type)
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
            _type = other._type;
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
        Json::Value jsonRoot = internal::ParseJson(filepath);
        ASSERT(!jsonRoot.isNull() || jsonRoot.empty(), "Failed to parse JSON from file: " + filepath);

        std::string pipelineType = jsonRoot["PipelineType"].asCString();
        _type = internal::ParsePipelineType(pipelineType);

        switch (_type)
        {
        case PipelineStateType::Graphics:
            ParseGraphicsPipeline(jsonRoot);
            break;
        case PipelineStateType::Compute:
            ParseComputePipeline(jsonRoot);
            break;
        default:
            UNREACHABLE("Unsupported pipeline state type!");
            return;
        }
    }

    void D3D12PipelineState::ParseGraphicsPipeline(const Json::Value& fileRoot)
    {
        NativeDevice* nativeDevice = D3D12Cast<NativeDevice>(_device->GetNative());

        // Load the vertex shader
        ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
        if (!fileRoot["VS"].isNull())
        {
            std::string vertexShaderFilepath = fileRoot["VS"].asString() + kShaderNameExtension;
            HRESULT result = D3DReadFileToBlob(std::wstring(vertexShaderFilepath.begin(), vertexShaderFilepath.end()).c_str(), &vertexShaderBlob);
            CHECK(result, "Failed to load vertex shader from file: " + vertexShaderFilepath);
        }

        // Load the geometry shader
        ComPtr<ID3DBlob> geometryShaderBlob = nullptr;
        if (!fileRoot["GS"].isNull())
        {
            std::string geometryShaderFilepath = fileRoot["GS"].asString() + kShaderNameExtension;
            HRESULT result = D3DReadFileToBlob(std::wstring(geometryShaderFilepath.begin(), geometryShaderFilepath.end()).c_str(), &geometryShaderBlob);
            CHECK(result, "Failed to load geometry shader from file: " + geometryShaderFilepath);
        }

        // Load the pixel shader
        ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
        if (!fileRoot["PS"].isNull())
        {
            std::string pixelShaderFilepath = fileRoot["PS"].asString() + kShaderNameExtension;
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
                inputLayout[i].Format = GetDXGIFormat(internal::ParseFormat(layout["Format"].asCString()));
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
        NativeDevice* nativeDevice = D3D12Cast<NativeDevice>(_device->GetNative());

        // Load the compute shader
        ComPtr<ID3DBlob> computeShaderBlob = nullptr;
        if (!fileRoot["CS"].isNull())
        {
            std::string computeShaderFilepath = fileRoot["CS"].asString() + kShaderNameExtension;
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
        return _type;
    }

    BlendState D3D12PipelineState::ParseBlendDescription(const std::string& filepath)
    {
        Json::Value root = internal::ParseJson(filepath);
        ASSERT(!root.isNull() || root.empty(), "Failed to parse JSON from file: " + filepath);

        BlendState description = {};

        int renderTargetsSize = root["RenderTargets"].size();
        for (int i = 0; i < renderTargetsSize; ++i)
        {
            Json::Value target = root["RenderTargets"][i];
            description.RenderTargets[i].BlendEnable = target["BlendEnable"].asBool();
            description.RenderTargets[i].SrcBlend = internal::ParseBlend(target["SrcBlend"].asCString());
            description.RenderTargets[i].DestBlend = internal::ParseBlend(target["DestBlend"].asCString());
            description.RenderTargets[i].BlendOp = internal::ParseBlendOp(target["BlendOp"].asCString());

            description.RenderTargets[i].SrcBlendAlpha = internal::ParseBlend(target["SrcBlendAlpha"].asCString());
            description.RenderTargets[i].DestBlendAlpha = internal::ParseBlend(target["DestBlendAlpha"].asCString());
            description.RenderTargets[i].BlendOpAlpha = internal::ParseBlendOp(target["BlendOpAlpha"].asCString());

            description.RenderTargets[i].RenderTargetWriteMask = internal::ParseColorWriteEnable(target["RenderTargetWriteMask"].asCString());

            description.RenderTargets[i].LogicOpEnable = target["LogicOpEnable"].asBool();
            description.RenderTargets[i].LogicOp = internal::ParseLogicOp(target["LogicOp"].asCString());
        }

        return description;
    }

    RasterizerState D3D12PipelineState::ParseRasterizerDescription(const std::string& filepath)
    {
        Json::Value root = internal::ParseJson(filepath);
        ASSERT(!root.isNull() || root.empty(), "Failed to parse JSON from file: " + filepath);

        RasterizerState description = {};
        description.FillMode = internal::ParseFillMode(root["FillMode"].asCString());
        description.CullMode = internal::ParseCullMode(root["CullMode"].asCString());
        description.DepthClipEnable = root["DepthClipEnable"].asBool();

        return description;
    }

    DepthStencilState D3D12PipelineState::ParseDepthStencilDescription(const std::string& filepath)
    {
        Json::Value root = internal::ParseJson(filepath);
        ASSERT(!root.isNull() || root.empty(), "Failed to parse JSON from file: " + filepath);

        DepthStencilState description = {};
        description.DepthEnable = root["DepthEnable"].asBool();
        description.DepthFunc = internal::ParseComparisonFunc(root["DepthFunc"].asCString());
        description.DepthWriteMask = internal::ParseDepthWriteMask(root["DepthWriteMask"].asCString());
        description.StencilEnable = root["StencilEnable"].asBool();

        return description;
    }

    void* D3D12PipelineState::GetNative() const
    {
        return static_cast<void*>(_pipelineState.Get());
    }

    void* D3D12PipelineState::GetNativeRootSignature() const
    {
        return static_cast<void*>(_rootSignature.Get());
    }
} // namespace rhi::d3d12
