
#include "RHI_PCH.h"

#include "D3D12CommandQueue.h"

#include "CommandList.h"
#include "Fence.h"

namespace rhi::d3d12
{
    D3D12CommandQueue::D3D12CommandQueue(rhi::Device* device, rhi::CommandListType type, const std::string& name)
        : _type(type)
        , _timestampFrequency(0)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

        switch (type)
        {
        case rhi::CommandListType::Graphics:
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            break;
        case rhi::CommandListType::Compute:
            desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            break;
        case rhi::CommandListType::Copy:
            desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            break;
        default:
            UNREACHABLE("Unsupported command list type!");
            break;
        }

        ID3D12Device2* d3d12Device = D3D12Cast<ID3D12Device2>(device->GetNative());

        HRESULT result = d3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_commandQueue));
        CHECK(result, "Failed to create D3D12 command queue.");

        SetD3D12Name(_commandQueue.Get(), name);
    }

    D3D12CommandQueue::D3D12CommandQueue(D3D12CommandQueue&& other) noexcept
        : rhi::CommandQueue(std::move(other))
        , _type(other._type)
        , _timestampFrequency(other._timestampFrequency)
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif
    {
    }

    D3D12CommandQueue& D3D12CommandQueue::operator=(D3D12CommandQueue&& other) noexcept
    {
        if (this != &other)
        {
            rhi::CommandQueue::operator=(std::move(other));
            _type = other._type;
            _timestampFrequency = other._timestampFrequency;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif
        }

        return *this;
    }

    void D3D12CommandQueue::ExecuteCommandLists(std::vector<std::shared_ptr<rhi::CommandList>> commandLists)
    {
        std::vector<ID3D12CommandList*> d3d12CommandLists;
        //std::transform(commandLists.begin(), commandLists.end(), d3d12CommandLists.begin(), [](const std::shared_ptr<rhi::CommandList>& elem) { return elem.get(); });
        for (const auto& commandList : commandLists)
        {
            d3d12CommandLists.push_back(D3D12Cast<ID3D12CommandList>(commandList->GetNative()));
        }
        _commandQueue->ExecuteCommandLists(d3d12CommandLists.size(), d3d12CommandLists.data());
    }

    void D3D12CommandQueue::Signal(rhi::Fence& fence, std::uint64_t value)
    {
        ID3D12Fence* d3d12Fence = D3D12Cast<ID3D12Fence>(fence.GetNative());
        _commandQueue->Signal(d3d12Fence, value);
    }

    void D3D12CommandQueue::Wait(rhi::Fence& fence, std::uint64_t value)
    {
        ID3D12Fence* d3d12Fence = D3D12Cast<ID3D12Fence>(fence.GetNative());
        _commandQueue->Wait(d3d12Fence, value);
    }

    rhi::CommandListType D3D12CommandQueue::GetType() const
    {
        return _type;
    }

    std::uint64_t D3D12CommandQueue::GetTimestampFrequency() const
    {
        return _timestampFrequency;
    }

    void* D3D12CommandQueue::GetNative() const
    {
        return static_cast<void*>(_commandQueue.Get());
    }
} // namespace rhi::d3d12
