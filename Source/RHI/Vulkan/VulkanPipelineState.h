#pragma once

#include "PipelineState.h"

namespace Json
{
    class Value;
} // namespace Json

namespace rhi::vulkan
{
    class VulkanPipelineState final : public rhi::PipelineState
    {
    public:
        VulkanPipelineState(rhi::Device* device, const std::string& filepath);
        VulkanPipelineState(const VulkanPipelineState&) = delete;
        VulkanPipelineState(VulkanPipelineState&& other) noexcept;
        ~VulkanPipelineState() override;

        VulkanPipelineState& operator=(const VulkanPipelineState&) = delete;
        VulkanPipelineState& operator=(VulkanPipelineState&& other) noexcept;

        vk::Pipeline GetPipeline() const;
        vk::PipelineLayout GetPipelineLayout() const;

        rhi::PipelineStateType GetType() const override;

        void* GetNative() const override;
        void* GetNativeRootSignature() const override;

    private:
        friend class VulkanDevice;

        VulkanPipelineState(rhi::Device* device);

        void Parse(const std::string& filepath);
        void ParseGraphicsPipeline(const Json::Value& fileRoot);
        void ParseComputePipeline(const Json::Value& fileRoot);

        rhi::BlendState ParseBlendDescription(const std::string& filepath);
        rhi::RasterizerState ParseRasterizerDescription(const std::string& filepath);
        rhi::DepthStencilState ParseDepthStencilDescription(const std::string& filepath);

        std::vector<vk::PipelineShaderStageCreateInfo> LoadShaderStages(const Json::Value& fileRoot);
        vk::PipelineShaderStageCreateInfo LoadShaderStage(const std::string& filepath, vk::ShaderStageFlagBits stage);
        vk::ShaderModule LoadShaderModule(const std::string& filepath);

        vk::PipelineLayout _pipelineLayout;
        vk::Pipeline       _pipeline;

        // TODO: unified root signature — descriptor set layouts and samplers are identical across all
        // pipelines and should be owned by VulkanDevice (same duplication problem exists in D3D12)
        std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts;
        std::vector<vk::Sampler>             _immutableSamplers;

        rhi::PipelineStateType _type;

        rhi::Device* _device;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::vulkan
