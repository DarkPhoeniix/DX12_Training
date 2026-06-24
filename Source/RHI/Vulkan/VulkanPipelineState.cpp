
#include "RHI_PCH.h"

#include "VulkanPipelineState.h"

#include <json/json.h>

namespace rhi::vulkan
{
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
        , _type(other._type)
        , _device(other._device)
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    VulkanPipelineState::~VulkanPipelineState()
    {
        if (_pipeline || _pipelineLayout)
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
        }
    }

    VulkanPipelineState& VulkanPipelineState::operator=(VulkanPipelineState&& other) noexcept
    {
        if (this != &other)
        {
            _pipelineLayout = std::exchange(other._pipelineLayout, nullptr);
            _pipeline       = std::exchange(other._pipeline, nullptr);
            _type           = other._type;
            _device         = other._device;
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

    void VulkanPipelineState::Parse(const std::string&)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanPipelineState::ParseGraphicsPipeline(const Json::Value&)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanPipelineState::ParseComputePipeline(const Json::Value&)
    {
        NOT_IMPLEMENTED();
    }

    rhi::BlendState VulkanPipelineState::ParseBlendDescription(const std::string&)
    {
        NOT_IMPLEMENTED();
        return {};
    }

    rhi::RasterizerState VulkanPipelineState::ParseRasterizerDescription(const std::string&)
    {
        NOT_IMPLEMENTED();
        return {};
    }

    rhi::DepthStencilState VulkanPipelineState::ParseDepthStencilDescription(const std::string&)
    {
        NOT_IMPLEMENTED();
        return {};
    }
} // namespace rhi::vulkan
