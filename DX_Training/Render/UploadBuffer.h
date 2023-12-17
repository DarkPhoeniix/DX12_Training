#pragma once

#include "Utility/Defines.h"

#include <deque>

class UploadBuffer
{
public:
    // Use to upload data to the GPU
    struct Allocation
    {
        void* CPU;
        D3D12_GPU_VIRTUAL_ADDRESS GPU;
    };

    explicit UploadBuffer(size_t pageSize = _2MB);
    virtual ~UploadBuffer();

    Allocation Allocate(size_t sizeInBytes, size_t alignment);
    void Reset();

    size_t GetPageSize() const;

private:
    // A single page for the allocator.
    struct Page
    {
        Page(size_t sizeInBytes);
        ~Page();

        Allocation Allocate(size_t sizeInBytes, size_t alignment);
        void Reset();

        bool HasSpace(size_t sizeInBytes, size_t alignment) const;

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> _d3d12Resource;

        void* _CPUPtr;
        D3D12_GPU_VIRTUAL_ADDRESS _GPUPtr;

        size_t _pageSize;
        size_t _offset;
    };

    using PagePool = std::deque<std::shared_ptr<Page>>;

    std::shared_ptr<Page> RequestPage();

    PagePool _pagePool;
    PagePool _availablePages;

    std::shared_ptr<Page> _currentPage;

    size_t _pageSize;
};
