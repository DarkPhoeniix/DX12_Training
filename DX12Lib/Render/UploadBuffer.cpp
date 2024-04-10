#include "stdafx.h"

#include "UploadBuffer.h"

#include "Application.h"

UploadBuffer::UploadBuffer(size_t pageSize)
    : _pageSize(pageSize)
{   }

UploadBuffer::~UploadBuffer()
{   }

UploadBuffer::Allocation UploadBuffer::Allocate(size_t sizeInBytes, size_t alignment)
{
    if (sizeInBytes > _pageSize)
    {
        throw std::bad_alloc();
    }

    // If there is no current page, or the requested allocation exceeds the
    // remaining space in the current page, request a new page.
    if (!_currentPage || !_currentPage->HasSpace(sizeInBytes, alignment))
    {
        _currentPage = RequestPage();
    }

    return _currentPage->Allocate(sizeInBytes, alignment);
}

std::shared_ptr<UploadBuffer::Page> UploadBuffer::RequestPage()
{
    std::shared_ptr<Page> page;

    if (!_availablePages.empty())
    {
        page = _availablePages.front();
        _availablePages.pop_front();
    }
    else
    {
        page = std::make_shared<Page>(_pageSize);
        _pagePool.push_back(page);
    }

    return page;
}

void UploadBuffer::Reset()
{
    _currentPage = nullptr;
    // Reset all available pages.
    _availablePages = _pagePool;

    for (auto page : _availablePages)
    {
        // Reset the page for new allocations.
        page->Reset();
    }
}

size_t UploadBuffer::GetPageSize() const
{
    return _pageSize;
}

UploadBuffer::Page::Page(size_t sizeInBytes)
    : _pageSize(sizeInBytes)
    , _offset(0)
    , _CPUPtr(nullptr)
    , _GPUPtr(D3D12_GPU_VIRTUAL_ADDRESS(0))
{
    auto device = Application::Get().getDevice();

    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceBuffer = CD3DX12_RESOURCE_DESC::Buffer(_pageSize);
    Helper::throwIfFailed(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceBuffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_d3d12Resource)
    ));

    _GPUPtr = _d3d12Resource->GetGPUVirtualAddress();
    _d3d12Resource->Map(0, nullptr, &_CPUPtr);
}

UploadBuffer::Page::~Page()
{
    _d3d12Resource->Unmap(0, nullptr);
    _CPUPtr = nullptr;
    _GPUPtr = D3D12_GPU_VIRTUAL_ADDRESS(0);
}

bool UploadBuffer::Page::HasSpace(size_t sizeInBytes, size_t alignment) const
{
    size_t alignedSize = Math::AlignUp(sizeInBytes, alignment);
    size_t alignedOffset = Math::AlignUp(_offset, alignment);

    return alignedOffset + alignedSize <= _pageSize;
}

UploadBuffer::Allocation UploadBuffer::Page::Allocate(size_t sizeInBytes, size_t alignment)
{
    if (!HasSpace(sizeInBytes, alignment))
    {
        // Can't allocate space from page.
        throw std::bad_alloc();
    }

    size_t alignedSize = Math::AlignUp(sizeInBytes, alignment);
    _offset = Math::AlignUp(_offset, alignment);

    Allocation allocation;
    allocation.CPU = static_cast<uint8_t*>(_CPUPtr) + _offset;
    allocation.GPU = _GPUPtr + _offset;

    _offset += alignedSize;

    return allocation;
}

void UploadBuffer::Page::Reset()
{
    _offset = 0;
}
