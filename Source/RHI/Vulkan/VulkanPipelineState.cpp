
#include "RHI_PCH.h"

#include "VulkanPipelineState.h"

#include "VulkanHelpers.h"

#include "PipelineHelpers.h"

#include <json/json.h>

#include <filesystem>
#include <fstream>

namespace rhi::vulkan
{
    namespace
    {
        static const std::string kShaderNameExtension = ".spv";

        static std::vector<char> ReadFile(const std::string& filename)
        {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);
            if (!file.is_open())
            {
                throw std::runtime_error("failed to open file!");
            }
            std::vector<char> buffer(file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            file.close();
            return buffer;
        }

        // TODO: add FORMAT types
        const std::map<std::string, vk::Format> TEX_FORMAT =
        {
            { "R8G8B8A8_UNORM",        vk::Format::eR8G8B8A8Unorm         },
            { "R8G8B8A8_SNORM",        vk::Format::eR8G8B8A8Snorm         },
            { "R32G32B32A32_FLOAT",    vk::Format::eR32G32B32A32Sfloat    },
            { "R11G11B10_FLOAT",       vk::Format::eB10G11R11UfloatPack32 },
            { "D32_FLOAT",             vk::Format::eD32Sfloat             },
            { "R32_FLOAT",             vk::Format::eR32Sfloat             },
        };

        vk::Format ParseTexFormat(const std::string& str)
        {
            auto it = TEX_FORMAT.find(str);
            if (it != TEX_FORMAT.end())
            {
                return it->second;
            }

            LOG_WARNING("Failed to parse {} from the texture format description", str);
            return TEX_FORMAT.begin()->second;
        }

        vk::PrimitiveTopology ParseTopologyType(const std::string& typeName)
        {
            if (typeName == "point")    return vk::PrimitiveTopology::ePointList;
            if (typeName == "line")     return vk::PrimitiveTopology::eLineList;
            if (typeName == "triangle") return vk::PrimitiveTopology::eTriangleList;

            LOG_WARNING("Failed to parse {} from topology type description", typeName);
            return vk::PrimitiveTopology::eTriangleList;
        }

        struct VertexInputData
        {
            std::vector<vk::VertexInputBindingDescription>   bindings;
            std::vector<vk::VertexInputAttributeDescription> attributes;
        };

        VertexInputData ParseVertexInputData(const Json::Value& layout)
        {
            VertexInputData result;
            if (layout.isNull())
            {
                return result;
            }

            std::map<std::uint32_t, std::uint32_t> slotStride;

            for (std::uint32_t i = 0; i < layout.size(); ++i)
            {
                const Json::Value&  elem      = layout[i];
                const std::uint32_t slot      = elem["Slot"].asUInt();
                const std::uint32_t offset    = elem["Offset"].asUInt();
                const rhi::Format   rhiFormat = internal::ParseFormat(elem["Format"].asCString());

                result.attributes.push_back(
                {
                    .location = i,
                    .binding  = slot,
                    .format   = GetVkVertexFormat(rhiFormat),
                    .offset   = offset
                });

                slotStride[slot] = std::max(slotStride[slot], offset + GetVkFormatSize(rhiFormat));
            }

            for (const auto& [slot, stride] : slotStride)
            {
                result.bindings.push_back(
                {
                    .binding   = slot,
                    .stride    = stride,
                    .inputRate = vk::VertexInputRate::eVertex
                });
            }

            return result;
        }

        std::vector<vk::Sampler> CreateImmutableSamplers(vk::Device device)
        {
            struct SamplerDesc
            {
                vk::Filter             filter;
                vk::SamplerMipmapMode  mipmapMode;
                vk::SamplerAddressMode addressMode;
                bool                   anisotropy;
                bool                   comparison;
            };

            // Matches UnifiedRootSignature static sampler declarations (s0..s12)
            const std::array<SamplerDesc, 13> descs =
            {{
                { vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest, vk::SamplerAddressMode::eClampToEdge,    false, false }, // s0  PointClamp
                { vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest, vk::SamplerAddressMode::eRepeat,         false, false }, // s1  PointWrap
                { vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest, vk::SamplerAddressMode::eMirroredRepeat, false, false }, // s2  PointMirror
                { vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest, vk::SamplerAddressMode::eClampToBorder,  false, false }, // s3  PointBorder
                { vk::Filter::eLinear,  vk::SamplerMipmapMode::eLinear,  vk::SamplerAddressMode::eClampToEdge,    false, false }, // s4  LinearClamp
                { vk::Filter::eLinear,  vk::SamplerMipmapMode::eLinear,  vk::SamplerAddressMode::eRepeat,         false, false }, // s5  LinearWrap
                { vk::Filter::eLinear,  vk::SamplerMipmapMode::eLinear,  vk::SamplerAddressMode::eMirroredRepeat, false, false }, // s6  LinearMirror
                { vk::Filter::eLinear,  vk::SamplerMipmapMode::eLinear,  vk::SamplerAddressMode::eClampToBorder,  false, false }, // s7  LinearBorder
                { vk::Filter::eLinear,  vk::SamplerMipmapMode::eLinear,  vk::SamplerAddressMode::eClampToEdge,    true,  false }, // s8  AnisotropicClamp
                { vk::Filter::eLinear,  vk::SamplerMipmapMode::eLinear,  vk::SamplerAddressMode::eRepeat,         true,  false }, // s9  AnisotropicWrap
                { vk::Filter::eLinear,  vk::SamplerMipmapMode::eLinear,  vk::SamplerAddressMode::eMirroredRepeat, true,  false }, // s10 AnisotropicMirror
                { vk::Filter::eLinear,  vk::SamplerMipmapMode::eLinear,  vk::SamplerAddressMode::eClampToBorder,  true,  false }, // s11 AnisotropicBorder
                { vk::Filter::eLinear,  vk::SamplerMipmapMode::eLinear,  vk::SamplerAddressMode::eClampToEdge,    false, true  }, // s12 ShadowComparisonClamp
            }};

            std::vector<vk::Sampler> samplers;
            samplers.reserve(descs.size());

            for (const auto& desc : descs)
            {
                const vk::SamplerCreateInfo createInfo =
                {
                    .magFilter               = desc.filter,
                    .minFilter               = desc.filter,
                    .mipmapMode              = desc.mipmapMode,
                    .addressModeU            = desc.addressMode,
                    .addressModeV            = desc.addressMode,
                    .addressModeW            = desc.addressMode,
                    .mipLodBias              = 0.0f,
                    .anisotropyEnable        = desc.anisotropy ? vk::True : vk::False,
                    .maxAnisotropy           = desc.anisotropy ? 16.0f : 1.0f,
                    .compareEnable           = desc.comparison ? vk::True : vk::False,
                    .compareOp               = desc.comparison ? vk::CompareOp::eLess : vk::CompareOp::eAlways,
                    .minLod                  = 0.0f,
                    .maxLod                  = vk::LodClampNone,
                    .borderColor             = vk::BorderColor::eFloatOpaqueBlack,
                    .unnormalizedCoordinates = vk::False
                };

                auto [result, sampler] = device.createSampler(createInfo);
                VK_CHECK(result, "Failed to create immutable sampler");
                samplers.push_back(sampler);
            }

            return samplers;
        }

        std::vector<vk::DescriptorSetLayout> CreateDescriptorSetLayouts(vk::Device device, const std::vector<vk::Sampler>& immutableSamplers)
        {
            // Set 0: frame constants (b0) + static samplers (s0..s12)
            std::vector<vk::DescriptorSetLayoutBinding> set0Bindings;
            set0Bindings.push_back(
            {
                .binding         = 0,
                .descriptorType  = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = 1,
                .stageFlags      = vk::ShaderStageFlagBits::eAll
            });

            for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(immutableSamplers.size()); ++i)
            {
                set0Bindings.push_back(
                {
                    .binding            = 1 + i,
                    .descriptorType     = vk::DescriptorType::eSampler,
                    .descriptorCount    = 1,
                    .stageFlags         = vk::ShaderStageFlagBits::eAll,
                    .pImmutableSamplers = &immutableSamplers[i]
                });
            }

            const vk::DescriptorSetLayoutCreateInfo set0CreateInfo =
            {
                .bindingCount = static_cast<std::uint32_t>(set0Bindings.size()),
                .pBindings    = set0Bindings.data()
            };

            auto [set0Result, set0Layout] = device.createDescriptorSetLayout(set0CreateInfo);
            VK_CHECK(set0Result, "Failed to create descriptor set layout (set 0)");

            // Set 1: bindless resource heap — ResourceDescriptorHeap equivalent
            // TODO: requires VK_EXT_descriptor_indexing (Vulkan 1.2 core) enabled on VulkanDevice
            // Binding 0: storage buffers (StructuredBuffer SRVs)
            // Binding 1: storage images (UAVs)
            // Binding 2: sampled images (texture SRVs) — must be last for VARIABLE_DESCRIPTOR_COUNT
            const std::array<vk::DescriptorBindingFlags, 3> set1BindingFlags =
            {{
                vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind,
                vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind,
                vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::eVariableDescriptorCount,
            }};

            const vk::DescriptorSetLayoutBindingFlagsCreateInfo set1BindingFlagsInfo =
            {
                .bindingCount  = static_cast<std::uint32_t>(set1BindingFlags.size()),
                .pBindingFlags = set1BindingFlags.data()
            };

            const std::array<vk::DescriptorSetLayoutBinding, 3> set1Bindings =
            {{
                { .binding = 0, .descriptorType = vk::DescriptorType::eStorageBuffer, .descriptorCount = 65536, .stageFlags = vk::ShaderStageFlagBits::eAll },
                { .binding = 1, .descriptorType = vk::DescriptorType::eStorageImage,  .descriptorCount = 65536, .stageFlags = vk::ShaderStageFlagBits::eAll },
                { .binding = 2, .descriptorType = vk::DescriptorType::eSampledImage,  .descriptorCount = 65536, .stageFlags = vk::ShaderStageFlagBits::eAll },
            }};

            const vk::DescriptorSetLayoutCreateInfo set1CreateInfo =
            {
                .pNext        = &set1BindingFlagsInfo,
                .flags        = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
                .bindingCount = static_cast<std::uint32_t>(set1Bindings.size()),
                .pBindings    = set1Bindings.data()
            };

            auto [set1Result, set1Layout] = device.createDescriptorSetLayout(set1CreateInfo);
            VK_CHECK(set1Result, "Failed to create descriptor set layout (set 1)");

            return { set0Layout, set1Layout };
        }
    } // namespace unnamed

    VulkanPipelineState::VulkanPipelineState(rhi::Device* device, const std::string& filepath)
        : _pipelineLayout(nullptr)
        , _pipeline(nullptr)
        , _type(rhi::PipelineStateType::Graphics)
        , _device(device)
    {
        Parse(filepath);
    }

    VulkanPipelineState::VulkanPipelineState(VulkanPipelineState&& other) noexcept
        : _pipelineLayout(std::exchange(other._pipelineLayout, nullptr))
        , _pipeline(std::exchange(other._pipeline, nullptr))
        , _descriptorSetLayouts(std::move(other._descriptorSetLayouts))
        , _immutableSamplers(std::move(other._immutableSamplers))
        , _type(other._type)
        , _device(other._device)
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    VulkanPipelineState::~VulkanPipelineState()
    {
        if (_pipeline || _pipelineLayout || !_descriptorSetLayouts.empty() || !_immutableSamplers.empty())
        {
            vk::Device vkDevice = VulkanCast<vk::Device>(_device->GetNative());
            if (_pipeline)
            {
                vkDevice.destroyPipeline(_pipeline);
            }
            if (_pipelineLayout)
            {
                vkDevice.destroyPipelineLayout(_pipelineLayout);
            }
            for (vk::DescriptorSetLayout layout : _descriptorSetLayouts)
            {
                vkDevice.destroyDescriptorSetLayout(layout);
            }
            for (vk::Sampler sampler : _immutableSamplers)
            {
                vkDevice.destroySampler(sampler);
            }
        }
    }

    VulkanPipelineState& VulkanPipelineState::operator=(VulkanPipelineState&& other) noexcept
    {
        if (this != &other)
        {
            _pipelineLayout      = std::exchange(other._pipelineLayout, nullptr);
            _pipeline            = std::exchange(other._pipeline, nullptr);
            _descriptorSetLayouts = std::move(other._descriptorSetLayouts);
            _immutableSamplers   = std::move(other._immutableSamplers);
            _type                = other._type;
            _device              = other._device;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }
        return *this;
    }

    vk::Pipeline VulkanPipelineState::GetPipeline() const
    {
        return _pipeline;
    }

    vk::PipelineLayout VulkanPipelineState::GetPipelineLayout() const
    {
        return _pipelineLayout;
    }

    rhi::PipelineStateType VulkanPipelineState::GetType() const
    {
        return _type;
    }

    void* VulkanPipelineState::GetNative() const
    {
        return VulkanNative(_pipeline);
    }

    void* VulkanPipelineState::GetNativeRootSignature() const
    {
        return VulkanNative(_pipelineLayout);
    }

    void VulkanPipelineState::Parse(const std::string& filepath)
    {
        // TODO: move to rhi::PipelineState?

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

    void VulkanPipelineState::ParseGraphicsPipeline(const Json::Value& fileRoot)
    {
        vk::Device nativeDevice = VulkanCast<vk::Device>(_device->GetNative());

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = LoadShaderStages(fileRoot);

        const RasterizerState   rasterizerState   = ParseRasterizerDescription(fileRoot["Raster"].asCString());
        const DepthStencilState depthStencilState = ParseDepthStencilDescription(fileRoot["Depth"].asCString());
        const BlendState        blendState        = ParseBlendDescription(fileRoot["Blend"].asCString());

        const vk::PipelineRasterizationStateCreateInfo rasterizationInfo = GetVkRasterizationDesc(rasterizerState);
        const vk::PipelineMultisampleStateCreateInfo   multisampleInfo   = GetVkMultisampleDesc(rasterizerState);
        const vk::PipelineDepthStencilStateCreateInfo  depthStencilInfo  = GetVkDepthStencilDescription(depthStencilState);

        const std::uint32_t rtCount = fileRoot["RenderTargets"].size();
        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
        for (std::uint32_t i = 0; i < rtCount; ++i)
        {
            colorBlendAttachments.push_back(GetVkColorBlendAttachmentDesc(blendState.RenderTargets[i]));
        }

        vk::PipelineColorBlendStateCreateInfo colorBlendInfo = GetVkColorBlendDesc(blendState);
        colorBlendInfo.attachmentCount = rtCount;
        colorBlendInfo.pAttachments    = colorBlendAttachments.data();

        auto [vertexBindings, vertexAttributes] = ParseVertexInputData(fileRoot["Layout"]);
        const vk::PipelineVertexInputStateCreateInfo vertexInputInfo =
        {
            .vertexBindingDescriptionCount   = static_cast<std::uint32_t>(vertexBindings.size()),
            .pVertexBindingDescriptions      = vertexBindings.data(),
            .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertexAttributes.size()),
            .pVertexAttributeDescriptions    = vertexAttributes.data()
        };

        const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = GetVkInputAssemblyDesc(ParseTopologyType(fileRoot["TopologyType"].asCString()));
        const vk::PipelineViewportStateCreateInfo      viewportInfo      = GetVkViewportDesc();

        const bool hasTessellation = !fileRoot["HS"].isNull() || !fileRoot["DS"].isNull();
        const vk::PipelineTessellationStateCreateInfo tessellationInfo = {};

        const std::array dynamicStates =
        {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor,
            vk::DynamicState::ePrimitiveTopology
        };
        const vk::PipelineDynamicStateCreateInfo dynamicStateInfo =
        {
            .dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
            .pDynamicStates    = dynamicStates.data()
        };

        // TODO: root signature is unified and identical across all pipelines — descriptor set layouts
        // and samplers should be owned by VulkanDevice and shared, same fix needed for D3D12 root signature duplication
        _immutableSamplers    = CreateImmutableSamplers(nativeDevice);
        _descriptorSetLayouts = CreateDescriptorSetLayouts(nativeDevice, _immutableSamplers);

        const vk::PushConstantRange pushConstantRange =
        {
            .stageFlags = vk::ShaderStageFlagBits::eAll,
            .offset     = 0,
            .size       = ROOT_CONSTANT_COUNT * sizeof(std::uint32_t)
        };

        const vk::PipelineLayoutCreateInfo layoutCreateInfo =
        {
            .setLayoutCount         = static_cast<std::uint32_t>(_descriptorSetLayouts.size()),
            .pSetLayouts            = _descriptorSetLayouts.data(),
            .pushConstantRangeCount = 1,
            .pPushConstantRanges    = &pushConstantRange
        };

        auto [layoutCreateResult, pipelineLayout] = nativeDevice.createPipelineLayout(layoutCreateInfo);
        VK_CHECK(layoutCreateResult, "Failed to create pipeline layout");

        std::vector<vk::Format> colorAttachmentFormats;
        for (const auto& rt : fileRoot["RenderTargets"])
        {
            colorAttachmentFormats.push_back(ParseTexFormat(rt.asCString()));
        }

        const vk::Format depthFormat = fileRoot["DepthBuffer"].isNull()
            ? vk::Format::eUndefined
            : vk::Format::eD32Sfloat;

        const vk::PipelineRenderingCreateInfo renderingInfo =
        {
            .colorAttachmentCount    = static_cast<std::uint32_t>(colorAttachmentFormats.size()),
            .pColorAttachmentFormats = colorAttachmentFormats.data(),
            .depthAttachmentFormat   = depthFormat,
            .stencilAttachmentFormat = vk::Format::eUndefined
        };

        // Create the graphics pipeline
        vk::GraphicsPipelineCreateInfo pipelineCreateInfo =
        {
            .pNext               = &renderingInfo,
            .flags               = {},
            .stageCount          = static_cast<std::uint32_t>(shaderStages.size()),
            .pStages             = shaderStages.data(),
            .pVertexInputState   = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pTessellationState  = hasTessellation ? &tessellationInfo : nullptr,
            .pViewportState      = &viewportInfo,
            .pRasterizationState = &rasterizationInfo,
            .pMultisampleState   = &multisampleInfo,
            .pDepthStencilState  = &depthStencilInfo,
            .pColorBlendState    = &colorBlendInfo,
            .pDynamicState       = &dynamicStateInfo,
            .layout              = pipelineLayout,
            .renderPass          = nullptr,
            .subpass             = 0,
            .basePipelineHandle  = {},
            .basePipelineIndex   = -1
        };

        // Create the pipeline cache
        vk::PipelineCacheCreateInfo cacheCreateInfo =
        {
            .flags = {},
            .initialDataSize = {},
            .pInitialData = {}
        };

        auto [cacheCreateResult, pipelineCache] = nativeDevice.createPipelineCache(cacheCreateInfo);
        VK_CHECK(cacheCreateResult, "Failed to create pipeline cache");

        auto [pipelineCreateResult, pipeline] = nativeDevice.createGraphicsPipeline(pipelineCache, pipelineCreateInfo);
        VK_CHECK(pipelineCreateResult, "Failed to create graphics pipeline");

        nativeDevice.destroyPipelineCache(pipelineCache);
        for (auto& stage : shaderStages)
        {
            nativeDevice.destroyShaderModule(stage.module);
        }

        _pipelineLayout = pipelineLayout;
        _pipeline       = pipeline;
    }

    void VulkanPipelineState::ParseComputePipeline(const Json::Value& fileRoot)
    {
        vk::Device nativeDevice = VulkanCast<vk::Device>(_device->GetNative());

        vk::PipelineShaderStageCreateInfo stageInfo = LoadShaderStage(fileRoot["CS"].asString(), vk::ShaderStageFlagBits::eCompute);

        _immutableSamplers    = CreateImmutableSamplers(nativeDevice);
        _descriptorSetLayouts = CreateDescriptorSetLayouts(nativeDevice, _immutableSamplers);

        const vk::PushConstantRange pushConstantRange =
        {
            .stageFlags = vk::ShaderStageFlagBits::eAll,
            .offset     = 0,
            .size       = ROOT_CONSTANT_COUNT * sizeof(std::uint32_t)
        };

        const vk::PipelineLayoutCreateInfo layoutCreateInfo =
        {
            .setLayoutCount         = static_cast<std::uint32_t>(_descriptorSetLayouts.size()),
            .pSetLayouts            = _descriptorSetLayouts.data(),
            .pushConstantRangeCount = 1,
            .pPushConstantRanges    = &pushConstantRange
        };

        auto [layoutCreateResult, pipelineLayout] = nativeDevice.createPipelineLayout(layoutCreateInfo);
        VK_CHECK(layoutCreateResult, "Failed to create pipeline layout");

        const vk::ComputePipelineCreateInfo pipelineCreateInfo =
        {
            .stage  = stageInfo,
            .layout = pipelineLayout
        };

        auto [pipelineCreateResult, pipeline] = nativeDevice.createComputePipeline(nullptr, pipelineCreateInfo);
        VK_CHECK(pipelineCreateResult, "Failed to create compute pipeline");

        nativeDevice.destroyShaderModule(stageInfo.module);

        _pipelineLayout = pipelineLayout;
        _pipeline       = pipeline;
    }

    rhi::BlendState VulkanPipelineState::ParseBlendDescription(const std::string& filepath)
    {
        // TODO: duplicates D3D12PipelineState::ParseBlendDescription
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

    rhi::RasterizerState VulkanPipelineState::ParseRasterizerDescription(const std::string& filepath)
    {
        // TODO: duplicates D3D12PipelineState::ParseRasterizerDescription
        Json::Value root = internal::ParseJson(filepath);
        ASSERT(!root.isNull() || root.empty(), "Failed to parse JSON from file: " + filepath);

        RasterizerState description = {};
        description.FillMode = internal::ParseFillMode(root["FillMode"].asCString());
        description.CullMode = internal::ParseCullMode(root["CullMode"].asCString());
        description.DepthClipEnable = root["DepthClipEnable"].asBool();

        return description;
    }

    rhi::DepthStencilState VulkanPipelineState::ParseDepthStencilDescription(const std::string& filepath)
    {
        // TODO: duplicates D3D12PipelineState::ParseDepthStencilDescription
        Json::Value root = internal::ParseJson(filepath);
        ASSERT(!root.isNull() || root.empty(), "Failed to parse JSON from file: " + filepath);

        DepthStencilState description = {};
        description.DepthEnable = root["DepthEnable"].asBool();
        description.DepthFunc = internal::ParseComparisonFunc(root["DepthFunc"].asCString());
        description.DepthWriteMask = internal::ParseDepthWriteMask(root["DepthWriteMask"].asCString());
        description.StencilEnable = root["StencilEnable"].asBool();

        return description;
    }

    std::vector<vk::PipelineShaderStageCreateInfo> VulkanPipelineState::LoadShaderStages(const Json::Value& fileRoot)
    {
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

        if (!fileRoot["CS"].isNull())
        {
            shaderStages.push_back(LoadShaderStage(fileRoot["CS"].asString(), vk::ShaderStageFlagBits::eCompute));
        }

        if (!fileRoot["VS"].isNull())
        {
            shaderStages.push_back(LoadShaderStage(fileRoot["VS"].asString(), vk::ShaderStageFlagBits::eVertex));
        }

        if (!fileRoot["GS"].isNull())
        {
            shaderStages.push_back(LoadShaderStage(fileRoot["GS"].asString(), vk::ShaderStageFlagBits::eGeometry));
        }

        if (!fileRoot["HS"].isNull())
        {
            shaderStages.push_back(LoadShaderStage(fileRoot["HS"].asString(), vk::ShaderStageFlagBits::eTessellationControl));
        }

        if (!fileRoot["DS"].isNull())
        {
            shaderStages.push_back(LoadShaderStage(fileRoot["DS"].asString(), vk::ShaderStageFlagBits::eTessellationEvaluation));
        }

        if (!fileRoot["PS"].isNull())
        {
            shaderStages.push_back(LoadShaderStage(fileRoot["PS"].asString(), vk::ShaderStageFlagBits::eFragment));
        }

        return shaderStages;
    }

    vk::PipelineShaderStageCreateInfo VulkanPipelineState::LoadShaderStage(const std::string& fileName, vk::ShaderStageFlagBits stage)
    {
        std::string psPath = fileName + kShaderNameExtension;
        vk::ShaderModule psModule = LoadShaderModule(psPath);
        vk::PipelineShaderStageCreateInfo psStageInfo =
        {
            .stage = stage,
            .module = psModule,
            .pName = "main"
        };

        return psStageInfo;
    }

    vk::ShaderModule VulkanPipelineState::LoadShaderModule(const std::string& filepath)
    {
        vk::Device nativeDevice = VulkanCast<vk::Device>(_device->GetNative());

        const std::vector<char> shaderCode = ReadFile(filepath);
        vk::ShaderModuleCreateInfo shaderModuleCreateInfo = 
        {
            .flags = {},
            .codeSize = shaderCode.size() * sizeof(char),
            .pCode = reinterpret_cast<const uint32_t*>(shaderCode.data())
        };

        auto [createResult, shaderModule] = nativeDevice.createShaderModule(shaderModuleCreateInfo);
        VK_CHECK(createResult, "Failed to create shader module");

        return shaderModule;
    }

} // namespace rhi::vulkan
