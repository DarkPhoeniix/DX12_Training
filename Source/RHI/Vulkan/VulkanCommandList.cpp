
#include "RHI_PCH.h"

#include "VulkanCommandList.h"

#include "VulkanCommandQueue.h"
#include "VulkanDescriptorHeap.h"
#include "VulkanDevice.h"
#include "VulkanHelpers.h"
#include "VulkanSwapChain.h"

#include "Buffer.h"
#include "QueryHeap.h"
#include "ResourceBarrier.h"
#include "StatisticsQuery.h"
#include "Texture.h"

namespace rhi::vulkan
{
    namespace
    {
        VulkanCommandQueue* SelectQueue(VulkanDevice* device, CommandListType type)
        {
            rhi::CommandQueue* queue = nullptr;
            switch (type)
            {
            case CommandListType::Graphics: queue = device->GetGraphicsQueue(); break;
            case CommandListType::Compute:  queue = device->GetComputeQueue();  break;
            case CommandListType::Copy:     queue = device->GetCopyQueue();     break;
            default:
                UNREACHABLE("Unsupported command list type!");
                queue = device->GetGraphicsQueue();
                break;
            }

            return static_cast<VulkanCommandQueue*>(queue);
        }

        vk::Rect2D GetRenderArea(const ScissorRect* rectangle, vk::Extent2D attachmentExtent)
        {
            vk::Rect2D renderArea = { .offset = { 0, 0 }, .extent = attachmentExtent };

            if (rectangle)
            {
                renderArea.offset = { rectangle->Left, rectangle->Top };
                renderArea.extent =
                {
                    static_cast<std::uint32_t>(rectangle->Right - rectangle->Left),
                    static_cast<std::uint32_t>(rectangle->Bottom - rectangle->Top)
                };
            }

            return renderArea;
        }
    } // namespace unnamed

    VulkanCommandList::VulkanCommandList(rhi::Device* device, rhi::CommandListType type, const std::string& name)
        : _commandPool(nullptr)
        , _commandBuffer(nullptr)
        , _type(type)
        , _boundDescriptorHeap(nullptr)
        , _device(device)
    {
        vk::Device vkDevice = VulkanCast<vk::Device>(device->GetNative());

        const std::uint32_t queueFamily = SelectQueue(static_cast<VulkanDevice*>(device), type)->GetQueueFamilyIndex();

        const vk::CommandPoolCreateInfo poolInfo =
        {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = queueFamily
        };
        auto [poolResult, pool] = vkDevice.createCommandPool(poolInfo);
        VK_CHECK(poolResult, "Failed to create command pool for command list");
        _commandPool = pool;

        const vk::CommandBufferAllocateInfo allocInfo =
        {
            .commandPool = _commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };
        auto [allocResult, buffers] = vkDevice.allocateCommandBuffers(allocInfo);
        VK_CHECK(allocResult, "Failed to allocate command buffer");
        _commandBuffer = buffers[0];

        SetVulkanName(vkDevice, _commandBuffer, name);
    }

    VulkanCommandList::VulkanCommandList(VulkanCommandList&& other) noexcept
        : _commandPool(std::exchange(other._commandPool, nullptr))
        , _commandBuffer(std::exchange(other._commandBuffer, nullptr))
        , _type(other._type)
        , _device(other._device)
    {
    }

    VulkanCommandList::~VulkanCommandList()
    {
        vk::Device vkDevice = VulkanCast<vk::Device>(_device->GetNative());

        if (_commandBuffer)
        {
            vkDevice.freeCommandBuffers(_commandPool, _commandBuffer);
        }
        if (_commandPool)
        {
            vkDevice.destroyCommandPool(_commandPool);
        }
    }

    VulkanCommandList& VulkanCommandList::operator=(VulkanCommandList&& other) noexcept
    {
        if (this != &other)
        {
            _commandPool   = std::exchange(other._commandPool, nullptr);
            _commandBuffer = std::exchange(other._commandBuffer, nullptr);
            _type          = other._type;
            _device        = other._device;
        }

        return *this;
    }

    rhi::CommandListType VulkanCommandList::GetCommandListType() const
    {
        return _type;
    }

    void VulkanCommandList::Reset(rhi::PipelineState*)
    {
        const vk::Result resetResult = _commandBuffer.reset();
        VK_CHECK(resetResult, "Failed to reset command buffer");

        constexpr vk::CommandBufferBeginInfo beginInfo =
        {
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };
        const vk::Result beginResult = _commandBuffer.begin(beginInfo);
        VK_CHECK(beginResult, "Failed to begin command buffer");
    }

    void VulkanCommandList::Close()
    {
        const vk::Result endResult = _commandBuffer.end();
        VK_CHECK(endResult, "Failed to end command buffer");
    }

    void* VulkanCommandList::GetNative() const
    {
        return VulkanNative(_commandBuffer);
    }

    void VulkanCommandList::SetPredication(std::shared_ptr<Buffer>, std::uint64_t, PredicationOperation) { NOT_IMPLEMENTED(); }

    void VulkanCommandList::BeginQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index)
    {
        vk::QueryPool queryPool = VulkanCast<vk::QueryPool>(queryHeap->GetNative());

        _commandBuffer.resetQueryPool(queryPool, index, 1);

        if (type != QueryType::Timestamp)
        {
            _commandBuffer.beginQuery(queryPool, index, {});
        }
    }

    void VulkanCommandList::EndQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index)
    {
        vk::QueryPool queryPool = VulkanCast<vk::QueryPool>(queryHeap->GetNative());

        if (type == QueryType::Timestamp)
        {
            _commandBuffer.resetQueryPool(queryPool, index, 1);
            _commandBuffer.writeTimestamp2(vk::PipelineStageFlagBits2::eBottomOfPipe, queryPool, index);
        }
        else
        {
            _commandBuffer.endQuery(queryPool, index);
        }
    }

    void VulkanCommandList::ResolveQueryData(QueryHeap* queryHeap, QueryType type, std::uint32_t index, std::shared_ptr<Buffer> destination, std::uint64_t offset)
    {
        ResolveQueryData(queryHeap, type, index, 1, destination, offset);
    }

    void VulkanCommandList::ResolveQueryData(QueryHeap* queryHeap, QueryType type, std::uint32_t index, std::uint32_t numQueries, std::shared_ptr<Buffer> destination, std::uint64_t offset)
    {
        constexpr std::uint64_t statisticsCount = sizeof(PipelineStatistics) / sizeof(std::uint64_t);
        const vk::DeviceSize stride = (type == QueryType::PipelineStatistics)
            ? (sizeof(std::uint64_t) * statisticsCount)
            : sizeof(std::uint64_t);

        _commandBuffer.copyQueryPoolResults(VulkanCast<vk::QueryPool>(queryHeap->GetNative()),
            index,
            numQueries,
            VulkanCast<vk::Buffer>(destination->GetNative()),
            offset,
            stride,
            vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait);
    }

    void VulkanCommandList::TransitionBarriers(const std::vector<BufferBarrier>&)  { NOT_IMPLEMENTED(); }

    void VulkanCommandList::TransitionBarriers(const std::vector<TextureBarrier>& barriers)
    {
        auto* swapChain = static_cast<VulkanSwapChain*>(static_cast<VulkanDevice*>(_device)->GetSwapChain());

        std::vector<vk::ImageMemoryBarrier2> imageBarriers;
        imageBarriers.reserve(barriers.size());

        for (const TextureBarrier& barrier : barriers)
        {
            std::shared_ptr<Texture> texture = barrier.TargetResource.lock();
            if (!texture)
            {
                LOG_WARNING("Skipping barrier for an expired texture.");
                continue;
            }

            const bool isSwapChainImage = swapChain && swapChain->OwnsTexture(texture.get());

            const ResourceStateInfo before = GetVkResourceStateInfo(barrier.BeforeState, isSwapChainImage);
            const ResourceStateInfo after = GetVkResourceStateInfo(barrier.AfterState, isSwapChainImage);

            const vk::ImageLayout oldLayout = (barrier.BeforeState == ResourceState::Common)
                ? vk::ImageLayout::eUndefined
                : before.Layout;

            imageBarriers.push_back(
            {
                .srcStageMask = before.Stage,
                .srcAccessMask = before.Access,
                .dstStageMask = after.Stage,
                .dstAccessMask = after.Access,
                .oldLayout = oldLayout,
                .newLayout = after.Layout,
                .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
                .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
                .image = VulkanCast<vk::Image>(texture->GetNative()),
                .subresourceRange =
                {
                    .aspectMask = GetVkImageAspect(texture->GetFormat()),
                    .baseMipLevel = 0,
                    .levelCount = vk::RemainingMipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = vk::RemainingArrayLayers
                }
            });

            texture->SetCurrentState(barrier.AfterState);
        }

        if (imageBarriers.empty())
        {
            return;
        }

        const vk::DependencyInfo dependencyInfo =
        {
            .imageMemoryBarrierCount = static_cast<std::uint32_t>(imageBarriers.size()),
            .pImageMemoryBarriers = imageBarriers.data()
        };

        _commandBuffer.pipelineBarrier2(dependencyInfo);
    }

    void VulkanCommandList::UAVBarrier(std::shared_ptr<Buffer>)                    { NOT_IMPLEMENTED(); }
    void VulkanCommandList::UAVBarrier(std::shared_ptr<Texture>)                   { NOT_IMPLEMENTED(); }

    void VulkanCommandList::CopyBuffer(std::shared_ptr<Buffer>, std::shared_ptr<Buffer>)                                                              { NOT_IMPLEMENTED(); }
    void VulkanCommandList::CopyBufferRegion(std::shared_ptr<Buffer>, std::shared_ptr<Buffer>, uint32_t, uint32_t, uint32_t)                          { NOT_IMPLEMENTED(); }
    void VulkanCommandList::CopyTexture(std::shared_ptr<Texture>, std::shared_ptr<Texture>)                                                           { NOT_IMPLEMENTED(); }
    void VulkanCommandList::CopyTextureRegion(std::shared_ptr<Texture>, std::shared_ptr<Texture>, uint32_t, uint32_t, uint32_t)                       { NOT_IMPLEMENTED(); }
    void VulkanCommandList::CopyBufferToTexture(std::shared_ptr<Buffer>, std::shared_ptr<Texture>, const std::vector<SubresourceData>&)               { NOT_IMPLEMENTED(); }

    void VulkanCommandList::SetGraphicsPipelineState(rhi::PipelineState*)  { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetComputePipelineState(rhi::PipelineState*)   { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetPrimitiveTopology(rhi::PrimitiveTopology)   { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetVertexBuffer(std::uint32_t, const rhi::VertexBufferView&) { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetIndexBuffer(const rhi::IndexBufferView&)    { NOT_IMPLEMENTED(); }

    void VulkanCommandList::SetRenderTarget(rhi::CPUDescriptor*, rhi::CPUDescriptor*)                          { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetRenderTargets(const std::vector<rhi::CPUDescriptor>&, rhi::CPUDescriptor*)      { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetViewport(const Viewport&, const ScissorRect&)                                   { NOT_IMPLEMENTED(); }

    void VulkanCommandList::ClearRTV(rhi::CPUDescriptor renderTargetView, const float color[4], ScissorRect* rectangle)
    {
        auto* heap = static_cast<VulkanDescriptorHeap*>(renderTargetView.Heap);
        ASSERT(heap, "Render target descriptor was not issued by a heap.");

        const std::uint32_t slot = static_cast<std::uint32_t>(renderTargetView.ptr);

        vk::ImageView imageView = heap->GetImageView(slot);
        ASSERT(imageView, "No render target view has been created for this descriptor.");

        const vk::RenderingAttachmentInfo attachmentInfo =
        {
            .imageView = imageView,
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = { .color = { std::array<float, 4>{ color[0], color[1], color[2], color[3] } } }
        };

        const vk::Rect2D renderArea = GetRenderArea(rectangle, heap->GetImageViewExtent(slot));

        const vk::RenderingInfo renderingInfo =
        {
            .renderArea = renderArea,
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentInfo
        };

        _commandBuffer.beginRendering(renderingInfo);
        _commandBuffer.endRendering();
    }
    void VulkanCommandList::ClearDSV(rhi::CPUDescriptor, rhi::ClearFlags, float, std::uint8_t, ScissorRect*)                  { NOT_IMPLEMENTED(); }

    void VulkanCommandList::Draw(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)                                  { NOT_IMPLEMENTED(); }
    void VulkanCommandList::DrawIndexed(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)           { NOT_IMPLEMENTED(); }
    void VulkanCommandList::Dispatch(std::uint32_t, std::uint32_t, std::uint32_t)                                             { NOT_IMPLEMENTED(); }
    void VulkanCommandList::ExecuteIndirect(rhi::CommandSignature*, std::uint32_t, std::shared_ptr<Buffer>, std::shared_ptr<Buffer>, std::uint32_t, std::uint32_t) { NOT_IMPLEMENTED(); }

    void VulkanCommandList::SetDescriptorHeaps(rhi::DescriptorHeap* descriptorHeap)
    {
        _boundDescriptorHeap = descriptorHeap;
    }

    void VulkanCommandList::SetDescriptorHeaps(const std::vector<rhi::DescriptorHeap*>& descriptorHeaps)
    {
        ASSERT(descriptorHeaps.size() <= 1, "Vulkan binds a single bindless heap per command list.");
        _boundDescriptorHeap = descriptorHeaps.empty() ? nullptr : descriptorHeaps.front();
    }

    void VulkanCommandList::SetGraphicsConstant(std::uint32_t, std::uint32_t, std::uint32_t) { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetComputeConstant(std::uint32_t, std::uint32_t, std::uint32_t)  { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetGraphicsConstants(std::uint32_t, std::uint32_t, const void*, std::uint32_t) { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetComputeConstants(std::uint32_t, std::uint32_t, const void*, std::uint32_t)  { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetGraphicsCBV(std::uint32_t, std::uint64_t) { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetComputeCBV(std::uint32_t, std::uint64_t)  { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetGraphicsSRV(std::uint32_t, std::uint64_t) { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetComputeSRV(std::uint32_t, std::uint64_t)  { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetGraphicsUAV(std::uint32_t, std::uint64_t) { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetComputeUAV(std::uint32_t, std::uint64_t)  { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetGraphicsDescriptorTable(std::uint32_t, rhi::GPUDescriptor) { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetComputeDescriptorTable(std::uint32_t, rhi::GPUDescriptor)  { NOT_IMPLEMENTED(); }

    void VulkanCommandList::BeginEvent(const char* name, std::uint8_t)
    {
        const vk::DebugUtilsLabelEXT label = { .pLabelName = name };
        _commandBuffer.beginDebugUtilsLabelEXT(label);
    }

    void VulkanCommandList::EndEvent()
    {
        _commandBuffer.endDebugUtilsLabelEXT();
    }

    void VulkanCommandList::SetMarker(const char* name, std::uint8_t)
    {
        const vk::DebugUtilsLabelEXT label = { .pLabelName = name };
        _commandBuffer.insertDebugUtilsLabelEXT(label);
    }

    void VulkanCommandList::SetName(const std::string& name)
    {
        SetVulkanName(VulkanCast<vk::Device>(_device->GetNative()), _commandBuffer, name);
    }
} // namespace rhi::vulkan
