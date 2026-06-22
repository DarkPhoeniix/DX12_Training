
#include "RHI_PCH.h"

#include "VulkanCommandList.h"

#include "VulkanCommandQueue.h"
#include "VulkanDevice.h"
#include "VulkanHelpers.h"

namespace rhi::vulkan
{
    VulkanCommandList::VulkanCommandList(rhi::Device* device, rhi::CommandListType type, const std::string& name)
        : _commandPool(nullptr)
        , _commandBuffer(nullptr)
        , _type(type)
        , _device(device)
    {
        vk::Device vkDevice = VulkanCast<vk::Device>(device->GetNative());

        const std::uint32_t queueFamily = static_cast<VulkanCommandQueue*>(
            static_cast<VulkanDevice*>(device)->GetGraphicsQueue()
        )->GetQueueFamilyIndex();

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
        VK_CHECK(_commandBuffer.reset(), "Failed to reset command buffer");

        constexpr vk::CommandBufferBeginInfo beginInfo =
        {
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };
        VK_CHECK(_commandBuffer.begin(beginInfo), "Failed to begin command buffer");
    }

    void VulkanCommandList::Close()
    {
        VK_CHECK(_commandBuffer.end(), "Failed to end command buffer");
    }

    void* VulkanCommandList::GetNative() const
    {
        return VulkanNative(_commandBuffer);
    }

    // --- stubs below — to be filled in as the backend matures ---

    void VulkanCommandList::SetPredication(std::shared_ptr<Buffer>, std::uint64_t, PredicationOperation) { NOT_IMPLEMENTED(); }

    void VulkanCommandList::BeginQuery(QueryHeap*, QueryType, std::uint32_t)                                              { NOT_IMPLEMENTED(); }
    void VulkanCommandList::EndQuery(QueryHeap*, QueryType, std::uint32_t)                                                { NOT_IMPLEMENTED(); }
    void VulkanCommandList::ResolveQueryData(QueryHeap*, QueryType, std::uint32_t, std::shared_ptr<Buffer>, std::uint64_t)                           { NOT_IMPLEMENTED(); }
    void VulkanCommandList::ResolveQueryData(QueryHeap*, QueryType, std::uint32_t, std::uint32_t, std::shared_ptr<Buffer>, std::uint64_t)            { NOT_IMPLEMENTED(); }

    void VulkanCommandList::TransitionBarriers(const std::vector<BufferBarrier>&)  { NOT_IMPLEMENTED(); }
    void VulkanCommandList::TransitionBarriers(const std::vector<TextureBarrier>&) { NOT_IMPLEMENTED(); }
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

    void VulkanCommandList::ClearRTV(rhi::CPUDescriptor, const float[4], ScissorRect*)                                        { NOT_IMPLEMENTED(); }
    void VulkanCommandList::ClearDSV(rhi::CPUDescriptor, rhi::ClearFlags, float, std::uint8_t, ScissorRect*)                  { NOT_IMPLEMENTED(); }

    void VulkanCommandList::Draw(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)                                  { NOT_IMPLEMENTED(); }
    void VulkanCommandList::DrawIndexed(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)           { NOT_IMPLEMENTED(); }
    void VulkanCommandList::Dispatch(std::uint32_t, std::uint32_t, std::uint32_t)                                             { NOT_IMPLEMENTED(); }
    void VulkanCommandList::ExecuteIndirect(rhi::CommandSignature*, std::uint32_t, std::shared_ptr<Buffer>, std::shared_ptr<Buffer>, std::uint32_t, std::uint32_t) { NOT_IMPLEMENTED(); }

    void VulkanCommandList::SetDescriptorHeaps(rhi::DescriptorHeap*)                          { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetDescriptorHeaps(const std::vector<rhi::DescriptorHeap*>&)      { NOT_IMPLEMENTED(); }
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

    void VulkanCommandList::BeginEvent(const char*, std::uint8_t) { NOT_IMPLEMENTED(); }
    void VulkanCommandList::EndEvent()                            { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetMarker(const char*, std::uint8_t) { NOT_IMPLEMENTED(); }
    void VulkanCommandList::SetName(const std::string&)          { NOT_IMPLEMENTED(); }
} // namespace rhi::vulkan
