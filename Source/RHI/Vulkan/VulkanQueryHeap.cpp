
#include "RHI_PCH.h"

#include "VulkanQueryHeap.h"

namespace rhi::vulkan
{
    namespace
    {
        constexpr vk::QueryPipelineStatisticFlags kPipelineStatistics =
            vk::QueryPipelineStatisticFlagBits::eInputAssemblyVertices |
            vk::QueryPipelineStatisticFlagBits::eInputAssemblyPrimitives |
            vk::QueryPipelineStatisticFlagBits::eVertexShaderInvocations |
            vk::QueryPipelineStatisticFlagBits::eGeometryShaderInvocations |
            vk::QueryPipelineStatisticFlagBits::eGeometryShaderPrimitives |
            vk::QueryPipelineStatisticFlagBits::eClippingInvocations |
            vk::QueryPipelineStatisticFlagBits::eClippingPrimitives |
            vk::QueryPipelineStatisticFlagBits::eFragmentShaderInvocations |
            vk::QueryPipelineStatisticFlagBits::eTessellationControlShaderPatches |
            vk::QueryPipelineStatisticFlagBits::eTessellationEvaluationShaderInvocations |
            vk::QueryPipelineStatisticFlagBits::eComputeShaderInvocations;

        vk::QueryType GetVkQueryType(QueryHeapType type)
        {
            switch (type)
            {
            case QueryHeapType::Occlusion:           return vk::QueryType::eOcclusion;
            case QueryHeapType::Timestamp:           return vk::QueryType::eTimestamp;
            case QueryHeapType::PipelineStatistics:
            case QueryHeapType::PipelineStatistics1: return vk::QueryType::ePipelineStatistics;
            default:
                UNREACHABLE("Unsupported query heap type!");
                return vk::QueryType::eOcclusion;
            }
        }
    } // namespace unnamed

    VulkanQueryHeap::VulkanQueryHeap(rhi::Device* device, const QueryHeapDescription& description, const std::string& name)
        : _queryPool(nullptr)
        , _type(description.Type)
        , _device(device)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        const vk::QueryType queryType = GetVkQueryType(description.Type);

        const vk::QueryPoolCreateInfo createInfo =
        {
            .queryType = queryType,
            .queryCount = description.Count,
            .pipelineStatistics = queryType == vk::QueryType::ePipelineStatistics
                ? kPipelineStatistics
                : vk::QueryPipelineStatisticFlags{}
        };

        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());

        auto [result, queryPool] = logicalDevice.createQueryPool(createInfo);
        VK_CHECK(result, "Failed to create query pool");
        _queryPool = queryPool;

        SetVulkanName(logicalDevice, _queryPool, name);
    }

    VulkanQueryHeap::VulkanQueryHeap(VulkanQueryHeap&& other) noexcept
        : _queryPool(std::exchange(other._queryPool, nullptr))
        , _type(other._type)
        , _device(other._device)
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    VulkanQueryHeap::~VulkanQueryHeap()
    {
        if (_queryPool)
        {
            VulkanCast<vk::Device>(_device->GetNative()).destroyQueryPool(_queryPool);
        }
    }

    VulkanQueryHeap& VulkanQueryHeap::operator=(VulkanQueryHeap&& other) noexcept
    {
        if (this != &other)
        {
            _queryPool = std::exchange(other._queryPool, nullptr);
            _type      = other._type;
            _device    = other._device;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }
        return *this;
    }

    QueryHeapType VulkanQueryHeap::GetType() const
    {
        return _type;
    }

    void* VulkanQueryHeap::GetNative() const
    {
        return VulkanNative(_queryPool);
    }
} // namespace rhi::vulkan
