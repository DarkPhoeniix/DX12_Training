
#include "RHI_PCH.h"

#include "D3D12CommandList.h"

#include "D3D12Descriptor.h"
#include "D3D12Helpers.h"

#include "Buffer.h"
#include "Texture.h"
#include "CommandSignature.h"
#include "DescriptorHeap.h"
#include "QueryHeap.h"
#include "PipelineState.h"
#include "ResourceBarrier.h"

#include <pix3.h>

namespace rhi::d3d12
{
    D3D12CommandList::D3D12CommandList(rhi::Device* device, rhi::CommandListType type, const std::string& name)
        : _commandList(nullptr)
        , _device(device)
        , _type(type)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES

    {
        ID3D12Device* nativeDevice = D3D12Cast<ID3D12Device>(device->GetNative());

        D3D12_COMMAND_LIST_TYPE d3d12Type = GetD3D12CommandListType(_type);

        HRESULT result = nativeDevice->CreateCommandAllocator(d3d12Type, IID_PPV_ARGS(&_commandAllocator));
        CHECK(result, "Failed to create D3D12CommandAllocator.");

        result = nativeDevice->CreateCommandList(0, d3d12Type, _commandAllocator.Get(), nullptr, IID_PPV_ARGS(&_commandList));
        CHECK(result, "Failed to create D3D12CommandList.");

#if ENABLE_DEBUG_NAMES
        SetD3D12Name(_commandAllocator.Get(), name);
        SetD3D12Name(_commandList.Get(), name);
#endif // ENABLE_DEBUG_NAMES
    }

    D3D12CommandList::D3D12CommandList(D3D12CommandList&& other) noexcept
        : _commandList(std::move(other._commandList))
        , _commandAllocator(std::move(other._commandAllocator))
        , _device(other._device)
        , _type(other._type)
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }
     
    D3D12CommandList::~D3D12CommandList()
    {
    }

    D3D12CommandList& D3D12CommandList::operator=(D3D12CommandList&& other) noexcept
    {
        if (this != &other)
        {
            _commandList = std::move(other._commandList);
            _commandAllocator = std::move(other._commandAllocator);
            _device = std::move(other._device);
            _type = other._type;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }

        return *this;
    }

    rhi::CommandListType D3D12CommandList::GetCommandListType() const
    {
        return _type;
    }

    void D3D12CommandList::SetPredication(std::shared_ptr<Buffer> buffer, std::uint64_t offset, PredicationOperation operation)
    {
        ID3D12Resource* nativeResource = D3D12Cast<ID3D12Resource>(buffer->GetNative());

        _commandList->SetPredication(nativeResource, offset, GetD3D12PredicationOp(operation));
    }

    void D3D12CommandList::BeginQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index)
    {
        ID3D12QueryHeap* nativeQueryHeap = D3D12Cast<ID3D12QueryHeap>(queryHeap->GetNative());

        _commandList->BeginQuery(nativeQueryHeap, GetD3D12QueryType(type), index);
    }

    void D3D12CommandList::ResolveQueryData(QueryHeap* queryHeap, QueryType type, std::uint32_t index, std::shared_ptr<Buffer> destination, std::uint64_t offset)
    {
        ID3D12QueryHeap* nativeQueryHeap = D3D12Cast<ID3D12QueryHeap>(queryHeap->GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destination->GetNative());

        _commandList->ResolveQueryData(nativeQueryHeap, GetD3D12QueryType(type), index, 1, nativeDestination, offset);
    }

    void D3D12CommandList::ResolveQueryData(QueryHeap* queryHeap, QueryType type, std::uint32_t index, std::uint32_t numQueries, std::shared_ptr<Buffer> destination, std::uint64_t offset)
    {
        ID3D12QueryHeap* nativeQueryHeap = D3D12Cast<ID3D12QueryHeap>(queryHeap->GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destination->GetNative());

        _commandList->ResolveQueryData(nativeQueryHeap, GetD3D12QueryType(type), index, numQueries, nativeDestination, offset);
    }

    void D3D12CommandList::EndQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index)
    {
        ID3D12QueryHeap* nativeQueryHeap = D3D12Cast<ID3D12QueryHeap>(queryHeap->GetNative());

        _commandList->EndQuery(nativeQueryHeap, GetD3D12QueryType(type), index);
    }

    void D3D12CommandList::TransitionBarriers(const std::vector<BufferBarrier>& barriers)
    {
        std::uint32_t numBarriers = static_cast<std::uint32_t>(barriers.size());
        if (numBarriers == 0)
        {
            return;
        }

        if (_device->IsEnhancedBarriersSupported())
        {
            std::vector<CD3DX12_BUFFER_BARRIER> bufferBarriers;

            for (size_t i = 0; i < numBarriers; ++i)
            {
                ASSERT(!barriers[i].TargetResource.expired(), "Resource in barrier is null.");
                if (std::shared_ptr<Buffer> resource = barriers[i].TargetResource.lock())
                {
                    const BufferBarrier& barrier = barriers[i];
                        bufferBarriers.emplace_back(
                            GetD3D12SyncFlags(barrier.BeforeState),
                            GetD3D12SyncFlags(barrier.AfterState),
                            GetD3D12AccessFlags(barrier.BeforeState),
                            GetD3D12AccessFlags(barrier.AfterState),
                            D3D12Cast<ID3D12Resource>(resource->GetNative()));

                    resource->SetCurrentState(barriers[i].AfterState);
                }
            }

            D3D12_BARRIER_GROUP barrierGroups[] =
            {
                CD3DX12_BARRIER_GROUP(static_cast<UINT32>(bufferBarriers.size()), bufferBarriers.data()),
            };

            _commandList->Barrier(1, barrierGroups);
        }
        else
        {
            std::vector<CD3DX12_RESOURCE_BARRIER> dxBarriers(numBarriers);

            for (size_t i = 0; i < numBarriers; ++i)
            {
                ASSERT(!barriers[i].TargetResource.expired(), "Resource in barrier is null.");
                if (std::shared_ptr<Buffer> resource = barriers[i].TargetResource.lock())
                {
                    dxBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
                        D3D12Cast<ID3D12Resource>(resource->GetNative()),
                        GetD3D12ResourceState(barriers[i].BeforeState),
                        GetD3D12ResourceState(barriers[i].AfterState),
                        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

                    resource->SetCurrentState(barriers[i].AfterState);
                }
            }

            _commandList->ResourceBarrier(numBarriers, dxBarriers.data());
        }
    }

    void D3D12CommandList::TransitionBarriers(const std::vector<TextureBarrier>& barriers)
    {
        std::uint32_t numBarriers = static_cast<std::uint32_t>(barriers.size());
        if (numBarriers == 0)
        {
            return;
        }

        if (_device->IsEnhancedBarriersSupported())
        {
            std::vector<CD3DX12_TEXTURE_BARRIER> textureBarriers;

            for (size_t i = 0; i < numBarriers; ++i)
            {
                ASSERT(!barriers[i].TargetResource.expired(), "Resource in barrier is null.");
                if (std::shared_ptr<Texture> resource = barriers[i].TargetResource.lock())
                {
                    const TextureBarrier& barrier = barriers[i];
                        textureBarriers.emplace_back(
                            GetD3D12SyncFlags(barrier.BeforeState),
                            GetD3D12SyncFlags(barrier.AfterState),
                            GetD3D12AccessFlags(barrier.BeforeState),
                            GetD3D12AccessFlags(barrier.AfterState),
                            GetD3D12Layout(barrier.BeforeState),
                            GetD3D12Layout(barrier.AfterState),
                            D3D12Cast<ID3D12Resource>(resource->GetNative()),
                            CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),       // All subresources
                            D3D12_TEXTURE_BARRIER_FLAG_NONE);

                    resource->SetCurrentState(barriers[i].AfterState);
                }
            }

            D3D12_BARRIER_GROUP barrierGroups[] =
            {
                CD3DX12_BARRIER_GROUP(static_cast<UINT32>(textureBarriers.size()), textureBarriers.data())
            };

            _commandList->Barrier(1, barrierGroups);
        }
        else
        {
            std::vector<CD3DX12_RESOURCE_BARRIER> dxBarriers(numBarriers);

            for (size_t i = 0; i < numBarriers; ++i)
            {
                ASSERT(!barriers[i].TargetResource.expired(), "Resource in barrier is null.");
                if (std::shared_ptr<Texture> resource = barriers[i].TargetResource.lock())
                {
                    dxBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
                        D3D12Cast<ID3D12Resource>(resource->GetNative()),
                        GetD3D12ResourceState(barriers[i].BeforeState),
                        GetD3D12ResourceState(barriers[i].AfterState),
                        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
                    resource->SetCurrentState(barriers[i].AfterState);
                }
            }

            _commandList->ResourceBarrier(numBarriers, dxBarriers.data());
        }
    }

    void D3D12CommandList::UAVBarrier(std::shared_ptr<Buffer> buffer)
    {
        FAIL(buffer != nullptr, "Buffer is null.");

        ID3D12Resource* nativeResource = D3D12Cast<ID3D12Resource>(buffer->GetNative());
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(nativeResource);

        _commandList->ResourceBarrier(1, &barrier);

        //CD3DX12_BUFFER_BARRIER barrier(
        //    D3D12_BARRIER_SYNC_ALL_SHADING,
        //    D3D12_BARRIER_SYNC_ALL_SHADING,
        //    D3D12_BARRIER_ACCESS_UNORDERED_ACCESS,
        //    D3D12_BARRIER_ACCESS_UNORDERED_ACCESS,
        //    D3D12Cast<ID3D12Resource>(buffer->GetNative())
        //);

        //D3D12_BARRIER_GROUP barrierGroups[] =
        //{
        //    CD3DX12_BARRIER_GROUP(1, &barrier)
        //};

        //_commandList->Barrier(1, barrierGroups);
    }

    void D3D12CommandList::UAVBarrier(std::shared_ptr<Texture> texture)
    {
        FAIL(texture != nullptr, "Texture is null.");

        ID3D12Resource* nativeResource = D3D12Cast<ID3D12Resource>(texture->GetNative());
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(nativeResource);

        _commandList->ResourceBarrier(1, &barrier);

        //CD3DX12_TEXTURE_BARRIER barrier(
        //    GetD3D12SyncFlags(texture->GetCurrentState()),
        //    D3D12_BARRIER_SYNC_ALL_SHADING,
        //    GetD3D12AccessFlags(texture->GetCurrentState()),
        //    D3D12_BARRIER_ACCESS_UNORDERED_ACCESS,
        //    GetD3D12Layout(texture->GetCurrentState()),
        //    D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS,
        //    D3D12Cast<ID3D12Resource>(texture->GetNative()),
        //    CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),       // All subresources
        //    D3D12_TEXTURE_BARRIER_FLAG_NONE
        //);

        //D3D12_BARRIER_GROUP barrierGroups[] =
        //{
        //    CD3DX12_BARRIER_GROUP(1, &barrier)
        //};

        //_commandList->Barrier(1, barrierGroups);
    }

    void D3D12CommandList::CopyBuffer(std::shared_ptr<Buffer> sourceResource, std::shared_ptr<Buffer> destinationResource)
    {
        ID3D12Resource* nativeSource = D3D12Cast<ID3D12Resource>(sourceResource->GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destinationResource->GetNative());

        _commandList->CopyResource(nativeDestination, nativeSource);
    }

    void D3D12CommandList::CopyBufferRegion(std::shared_ptr<Buffer> sourceResource, std::shared_ptr<Buffer> destinationResource, uint32_t numBytes, uint32_t sourceOffset, uint32_t destinationOffset)
    {
        ID3D12Resource* nativeSource = D3D12Cast<ID3D12Resource>(sourceResource->GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destinationResource->GetNative());

        _commandList->CopyBufferRegion(nativeDestination, destinationOffset, nativeSource, sourceOffset, numBytes);
    }

    void D3D12CommandList::CopyTexture(std::shared_ptr<Texture> sourceResource, std::shared_ptr<Texture> destinationResource)
    {
        ID3D12Resource* nativeSource = D3D12Cast<ID3D12Resource>(sourceResource->GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destinationResource->GetNative());

        _commandList->CopyResource(nativeDestination, nativeSource);
    }

    void D3D12CommandList::CopyTextureRegion(std::shared_ptr<Texture> sourceResource, std::shared_ptr<Texture> destinationResource, uint32_t numBytes, uint32_t sourceOffset, uint32_t destinationOffset)
    {
        ID3D12Resource* nativeSource = D3D12Cast<ID3D12Resource>(sourceResource->GetNative());
        ID3D12Resource* nativeDestination = D3D12Cast<ID3D12Resource>(destinationResource->GetNative());

        _commandList->CopyBufferRegion(nativeDestination, destinationOffset, nativeSource, sourceOffset, numBytes);
    }

    void D3D12CommandList::SetGraphicsPipelineState(rhi::PipelineState* pipelineState)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        ID3D12PipelineState* nativePipelineState = D3D12Cast<ID3D12PipelineState>(pipelineState->GetNative());
        ID3D12RootSignature* signature = D3D12Cast<ID3D12RootSignature>(pipelineState->GetNativeRootSignature());

        _commandList->SetPipelineState(nativePipelineState);
        _commandList->SetGraphicsRootSignature(signature);
    }

    void D3D12CommandList::SetComputePipelineState(rhi::PipelineState* pipelineState)
    {
        FAIL((_type == CommandListType::Graphics) || (_type == CommandListType::Compute), "Command list type is not Graphics or Compute.");

        ID3D12PipelineState* nativePipelineState = D3D12Cast<ID3D12PipelineState>(pipelineState->GetNative());
        ID3D12RootSignature* signature = D3D12Cast<ID3D12RootSignature>(pipelineState->GetNativeRootSignature());

        _commandList->SetPipelineState(nativePipelineState);
        _commandList->SetComputeRootSignature(signature);
    }

    void D3D12CommandList::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
    {
        _commandList->IASetPrimitiveTopology(GetD3D12PrimitiveTopology(primitiveTopology));
    }

    void D3D12CommandList::SetVertexBuffer(std::uint32_t slot, const VertexBufferView& vertexBufferView)
    {
        D3D12_VERTEX_BUFFER_VIEW vbv =
        {
            .BufferLocation = vertexBufferView.BufferLocation,
            .SizeInBytes = vertexBufferView.SizeInBytes,
            .StrideInBytes = vertexBufferView.StrideInBytes
        };
        _commandList->IASetVertexBuffers(slot, 1, &vbv);
    }

    void D3D12CommandList::SetIndexBuffer(const IndexBufferView& indexBufferView)
    {
        D3D12_INDEX_BUFFER_VIEW ibv =
        {
            .BufferLocation = indexBufferView.BufferLocation,
            .SizeInBytes = indexBufferView.SizeInBytes,
            .Format = GetDXGIFormat(indexBufferView.Format)
        };
        _commandList->IASetIndexBuffer(&ibv);
    }

    void D3D12CommandList::SetRenderTarget(rhi::CPUDescriptor* renderTargetDescriptor, rhi::CPUDescriptor* depthStencilDescriptor)
    {
        std::uint32_t numTargets = renderTargetDescriptor ? 1 : 0;

        D3D12_CPU_DESCRIPTOR_HANDLE* pRTHandle = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE RTHandle;
        if (renderTargetDescriptor)
        {
            RTHandle = ToD3D12Handle(*renderTargetDescriptor);
            pRTHandle = &RTHandle;
        }
        D3D12_CPU_DESCRIPTOR_HANDLE* pDSHandle = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE DSHandle;
        if (depthStencilDescriptor)
        {
            DSHandle = ToD3D12Handle(*depthStencilDescriptor);
            pDSHandle = &DSHandle;
        }

        _commandList->OMSetRenderTargets(numTargets, pRTHandle, FALSE, pDSHandle);
    }

    void D3D12CommandList::SetRenderTargets(const std::vector<rhi::CPUDescriptor>& renderTargetDescriptors, rhi::CPUDescriptor* depthStencilDescriptor)
    {
        std::uint32_t numTargets = static_cast<std::uint32_t>(renderTargetDescriptors.size());

        D3D12_CPU_DESCRIPTOR_HANDLE RTHandles[16]; // TODO: temp workaround
        for (size_t i = 0; i < numTargets; ++i)
        {
            RTHandles[i] = ToD3D12Handle(renderTargetDescriptors[i]);
        }
        D3D12_CPU_DESCRIPTOR_HANDLE* pDSHandle = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE DSHandle;
        if (depthStencilDescriptor)
        {
            DSHandle = ToD3D12Handle(*depthStencilDescriptor);
            pDSHandle = &DSHandle;
        }

        _commandList->OMSetRenderTargets(numTargets, RTHandles, FALSE, pDSHandle);
    }

    void D3D12CommandList::SetViewport(const Viewport& viewport, const ScissorRect& scissorRectangle)
    {
        D3D12_VIEWPORT nativeViewport =
        {
            .TopLeftX = viewport.TopLeftX,
            .TopLeftY = viewport.TopLeftY,
            .Width = viewport.Width,
            .Height = viewport.Height,
            .MinDepth = viewport.MinDepth,
            .MaxDepth = viewport.MaxDepth
        };
        _commandList->RSSetViewports(1, &nativeViewport);

        D3D12_RECT nativeRect =
        {
            .left = scissorRectangle.Left,
            .top = scissorRectangle.Top,
            .right = scissorRectangle.Right,
            .bottom = scissorRectangle.Bottom,
        };
        _commandList->RSSetScissorRects(1, &nativeRect);
    }

    void D3D12CommandList::ClearRTV(rhi::CPUDescriptor renderTargetView, const float color[4], ScissorRect* rectangle)
    {
        std::uint32_t numRects = rectangle ? 1 : 0;
        D3D12_RECT scissorRect = {};

        if (rectangle)
        {
            scissorRect =
            {
                .left = rectangle->Left,
                .top = rectangle->Top,
                .right = rectangle->Right,
                .bottom = rectangle->Bottom
            };
        }

        _commandList->ClearRenderTargetView(ToD3D12Handle(renderTargetView), color, numRects, rectangle ? &scissorRect : nullptr);
    }

    void D3D12CommandList::ClearDSV(rhi::CPUDescriptor depthStencilView, rhi::ClearFlags clearFlags, float depth, std::uint8_t stencil, ScissorRect* rectangle)
    {
        std::uint32_t numRects = rectangle ? 1 : 0;
        D3D12_RECT scissorRect = {};

        if (rectangle)
        {
            scissorRect =
            {
                .left = rectangle->Left,
                .top = rectangle->Top,
                .right = rectangle->Right,
                .bottom = rectangle->Bottom
            };
        }

        _commandList->ClearDepthStencilView(ToD3D12Handle(depthStencilView), GetD3D12ClearFlags(clearFlags), depth, stencil, numRects, rectangle ? &scissorRect : nullptr);
    }

    void D3D12CommandList::Draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t startVertex, std::uint32_t startInstance)
    {
        FAIL(_type == rhi::CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
    }

    void D3D12CommandList::DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t startIndex, std::uint32_t baseVertex, std::uint32_t startInstance)
    {
        FAIL(_type == rhi::CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
    }

    void D3D12CommandList::Dispatch(std::uint32_t xThreadGroupsCount, std::uint32_t yThreadGroupsCount, std::uint32_t zThreadGroupsCount)
    {
        FAIL(_type == rhi::CommandListType::Graphics || _type == CommandListType::Compute, "Command list type is not Compute.");

        _commandList->Dispatch(xThreadGroupsCount, yThreadGroupsCount, zThreadGroupsCount);
    }

    void D3D12CommandList::ExecuteIndirect(rhi::CommandSignature* commandSignature, std::uint32_t maxCommandCount, std::shared_ptr<Buffer> argumentBuffer, std::shared_ptr<Buffer> countBuffer, std::uint32_t argumentBufferOffset, std::uint32_t countBufferOffset)
    {
        ID3D12CommandSignature* d3d12CommandSignature = D3D12Cast<ID3D12CommandSignature>(commandSignature->GetNative());
        ID3D12Resource* d3d12ArgumentResource = D3D12Cast<ID3D12Resource>(argumentBuffer->GetNative());
        ID3D12Resource* d3d12CounterResource = nullptr;
        if (countBuffer)
        {
            d3d12CounterResource = D3D12Cast<ID3D12Resource>(countBuffer->GetNative());
        }
        _commandList->ExecuteIndirect(d3d12CommandSignature, maxCommandCount, d3d12ArgumentResource, argumentBufferOffset, d3d12CounterResource, countBufferOffset);
    }

    void D3D12CommandList::SetDescriptorHeaps(rhi::DescriptorHeap* descriptorHeap)
    {
        ID3D12DescriptorHeap* nativeHeap = D3D12Cast<ID3D12DescriptorHeap>(descriptorHeap->GetNative());
        _commandList->SetDescriptorHeaps(1, &nativeHeap);
    }

    void D3D12CommandList::SetDescriptorHeaps(const std::vector<rhi::DescriptorHeap*>& descriptorHeaps)
    {
        const std::uint32_t heapCount = static_cast<std::uint32_t>(descriptorHeaps.size());
        ASSERT(heapCount <= 2, "Too many decriptor heaps.");

        ID3D12DescriptorHeap* nativeHeaps[2];
        for (size_t i = 0; i < heapCount; ++i)
        {
            nativeHeaps[i] = D3D12Cast<ID3D12DescriptorHeap>(descriptorHeaps[i]->GetNative());
        }

        _commandList->SetDescriptorHeaps(heapCount, nativeHeaps);
    }

    void D3D12CommandList::SetGraphicsConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->SetGraphicsRoot32BitConstant(index, data, offset);
    }

    void D3D12CommandList::SetComputeConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset)
    {
        FAIL((_type == CommandListType::Graphics) || (_type == CommandListType::Compute), "Command list type is not Graphics or Compute.");

        _commandList->SetComputeRoot32BitConstant(index, data, offset);
    }

    void D3D12CommandList::SetGraphicsConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->SetGraphicsRoot32BitConstants(index, numValues, data, offset);
    }

    void D3D12CommandList::SetComputeConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset)
    {
        FAIL((_type == CommandListType::Graphics) || (_type == CommandListType::Compute), "Command list type is not Graphics or Compute.");

        _commandList->SetComputeRoot32BitConstants(index, numValues, data, offset);
    }

    void D3D12CommandList::SetGraphicsCBV(std::uint32_t index, std::uint64_t bufferLocation)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->SetGraphicsRootConstantBufferView(index, bufferLocation);
    }

    void D3D12CommandList::SetComputeCBV(std::uint32_t index, std::uint64_t bufferLocation)
    {
        FAIL((_type == CommandListType::Graphics) || (_type == CommandListType::Compute), "Command list type is not Graphics or Compute.");

        _commandList->SetComputeRootConstantBufferView(index, bufferLocation);
    }

    void D3D12CommandList::SetGraphicsSRV(std::uint32_t index, std::uint64_t bufferLocation)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->SetGraphicsRootShaderResourceView(index, bufferLocation);
    }

    void D3D12CommandList::SetComputeSRV(std::uint32_t index, std::uint64_t bufferLocation)
    {
        FAIL((_type == CommandListType::Graphics) || (_type == CommandListType::Compute), "Command list type is not Graphics or Compute.");

        _commandList->SetComputeRootShaderResourceView(index, bufferLocation);
    }

    void D3D12CommandList::SetGraphicsUAV(std::uint32_t index, std::uint64_t bufferLocation)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->SetGraphicsRootUnorderedAccessView(index, bufferLocation);
    }

    void D3D12CommandList::SetComputeUAV(std::uint32_t index, std::uint64_t bufferLocation)
    {
        FAIL((_type == CommandListType::Graphics) || (_type == CommandListType::Compute), "Command list type is not Graphics or Compute.");

        _commandList->SetComputeRootUnorderedAccessView(index, bufferLocation);
    }

    void D3D12CommandList::SetGraphicsDescriptorTable(std::uint32_t index, rhi::GPUDescriptor descriptor)
    {
        FAIL(_type == CommandListType::Graphics, "Command list type is not Graphics.");

        _commandList->SetGraphicsRootDescriptorTable(index, ToD3D12Handle(descriptor));
    }

    void D3D12CommandList::SetComputeDescriptorTable(std::uint32_t index, rhi::GPUDescriptor descriptor)
    {
        FAIL((_type == CommandListType::Graphics) || (_type == CommandListType::Compute), "Command list type is not Graphics or Compute.");

        _commandList->SetComputeRootDescriptorTable(index, ToD3D12Handle(descriptor));
    }

    void D3D12CommandList::Reset(rhi::PipelineState* pipelineState)
    {
        _commandList->Reset(_commandAllocator.Get(), pipelineState ? D3D12Cast<ID3D12PipelineState>(pipelineState->GetNative()) : nullptr);
    }

    void D3D12CommandList::Close()
    {
        HRESULT result = _commandList->Close();
        CHECK(result, "Failed to close command list.");
    }

    void D3D12CommandList::BeginEvent(const char* name, std::uint8_t color)
    {
        PIXBeginEvent(_commandList.Get(), color, name);
    }

    void D3D12CommandList::EndEvent()
    {
        PIXEndEvent(_commandList.Get());
    }

    void D3D12CommandList::SetMarker(const char* name, std::uint8_t color)
    {
        PIXSetMarker(color, name);
    }

    void D3D12CommandList::SetName(const std::string& name)
    {
        SetD3D12Name(_commandList.Get(), name);
    }

    void* D3D12CommandList::GetNative() const
    {
        return static_cast<void*>(_commandList.Get());
    }
} // namespace rhi::d3d12
