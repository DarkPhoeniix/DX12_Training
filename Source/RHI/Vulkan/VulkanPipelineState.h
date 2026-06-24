#pragma once

#include "PipelineState.h"
#include "VulkanHelpers.h"

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

        // vk::PipelineLayout is the Vulkan equivalent of ID3D12RootSignature
        vk::PipelineLayout _pipelineLayout;
        vk::Pipeline       _pipeline;

        rhi::PipelineStateType _type;

        rhi::Device* _device;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::vulkan
