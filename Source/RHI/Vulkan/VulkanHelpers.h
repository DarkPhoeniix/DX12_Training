#pragma once

#include "Format.h"
#include "ResourceCommon.h"
#include "QueryHeap.h"
#include "CommandList.h"

struct VmaAllocator_T;
using VmaAllocator = VmaAllocator_T*;
struct VmaAllocation_T;
using VmaAllocation = VmaAllocation_T*;

#define VK_CHECK(result, message)                                                       \
    do {                                                                                \
        if ((result) != vk::Result::eSuccess)                                           \
        {                                                                               \
            logging::Logger::Instance().Log(logging::Level::Error,                      \
                "[{} ({})] {} (VkResult: {})",                                          \
                __func__, __LINE__, (message), vk::to_string(result));                  \
            __debugbreak();                                                             \
        }                                                                               \
    } while (0)

namespace rhi::vulkan
{
    template<typename T>
    concept VulkanHandle = requires { typename T::NativeType; };

    template<VulkanHandle T>
    inline T VulkanCast(void* native)
    {
        ASSERT(native != nullptr, "Trying to cast a null pointer.");
        return T(reinterpret_cast<typename T::NativeType>(native));
    }

    template<VulkanHandle T>
    inline void* VulkanNative(T handle)
    {
        return reinterpret_cast<void*>(static_cast<typename T::NativeType>(handle));
    }

    template<VulkanHandle T>
    inline void SetVulkanName([[maybe_unused]] vk::Device device, [[maybe_unused]] T handle, [[maybe_unused]] const std::string& name)
    {
#if ENABLE_DEBUG_NAMES
        if (handle && !name.empty())
        {
            const vk::DebugUtilsObjectNameInfoEXT nameInfo =
            {
                .objectType   = T::objectType,
                .objectHandle = reinterpret_cast<std::uint64_t>(static_cast<typename T::NativeType>(handle)),
                .pObjectName  = name.c_str()
            };
            vk::Result result = device.setDebugUtilsObjectNameEXT(nameInfo);
            VK_CHECK(result, "Failed to set debug name for Vulkan object");
        }
#endif // ENABLE_DEBUG_NAMES
    }

    vk::PrimitiveTopology GetVkPrimitiveTopology(PrimitiveTopology topology);
    vk::BlendFactor GetVkBlendFactor(Blend blend);
    vk::BlendOp GetVkBlendOp(BlendOpType blendOp);
    vk::LogicOp GetVkLogicOp(LogicOp logicOp);
    vk::PolygonMode GetVkPolygonMode(FillMode fillMode);
    vk::CullModeFlags GetVkCullMode(CullMode cullMode);
    vk::FrontFace GetVkFrontFace(bool isCCW);
    vk::ColorComponentFlags GetVkColorWriteMask(ColorWriteEnable colorWriteMask);
    vk::PipelineRasterizationStateCreateInfo GetVkRasterizationDesc(const RasterizerState& rasterizerState);
    vk::PipelineMultisampleStateCreateInfo GetVkMultisampleDesc(const RasterizerState& multrasterizerStateisampleState);
    vk::PipelineDepthStencilStateCreateInfo GetVkDepthStencilDescription(const DepthStencilState& depthStencilState);
    vk::PipelineColorBlendAttachmentState GetVkColorBlendAttachmentDesc(const RTBlendState& rtBlendState);
    vk::PipelineColorBlendStateCreateInfo GetVkColorBlendDesc(const BlendState& blendState);
    vk::CompareOp GetVkCompareOp(ComparisonFunc comparisonFunc);
    vk::Format GetVkFormat(rhi::Format format);
    vk::Format GetVkVertexFormat(rhi::Format format);
    std::uint32_t GetVkFormatSize(rhi::Format format);
    vk::PipelineInputAssemblyStateCreateInfo GetVkInputAssemblyDesc(vk::PrimitiveTopology topology);
    vk::PipelineViewportStateCreateInfo GetVkViewportDesc();

    rhi::Format GetRHIFormat(vk::Format format);

    struct ResourceStateInfo
    {
        vk::ImageLayout Layout;
        vk::PipelineStageFlags2 Stage;
        vk::AccessFlags2 Access;
    };

    ResourceStateInfo GetVkResourceStateInfo(ResourceState state, bool isSwapChainImage);

    vk::ImageAspectFlags GetVkImageAspect(rhi::Format format);
    vk::ImageType GetVkImageType(TextureDimension dimension);
    vk::ImageViewType GetVkImageViewType(TextureDimension dimension, std::uint32_t arraySize);
    vk::ImageUsageFlags GetVkImageUsage(ResourceFlags flags);
    vk::DescriptorType GetVkDescriptorType(ResourceViewType viewType);
    vk::ImageLayout GetVkDescriptorImageLayout(ResourceViewType viewType, rhi::Format format);
} // namespace rhi::vulkan
