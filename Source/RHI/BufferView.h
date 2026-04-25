#pragma once

#include "Buffer.h"

namespace rhi
{
    class BufferView
    {
    public:
        BufferView(Buffer* buffer,
            ResourceViewType viewType,
            std::uint32_t size,
            std::uint64_t offset = 0,
            std::uint32_t firstElement = 0,
            std::uint32_t numElements = 0,
            std::uint32_t stride = 0,
            Buffer* counterResource = nullptr,
            std::uint64_t uavCounterOffset = 0)
            : _buffer(buffer)
            , _viewType(viewType)
            , _size(size)
            , _offset(offset)
            , _firstElement(firstElement)
            , _numElements(numElements)
            , _stride(stride)
            , _counterResource(counterResource)
            , _uavCounterOffset(uavCounterOffset)
        {
            ASSERT(_buffer, "BufferView cannot be created with a null buffer pointer.");
            if (_buffer)
            {
                _format = _buffer->GetDescription().Format;
                _virtualAddress = _buffer->GetVirtualAddress(offset);
            }
        }
        BufferView(const BufferView&) = delete;
        BufferView(BufferView&&) noexcept = default;
        virtual ~BufferView() = default;

        BufferView& operator=(const BufferView&) = delete;
        BufferView& operator=(BufferView&&) noexcept = default;

        [[nodiscard]] Buffer* GetBuffer() const { return _buffer; }

        void SetFormat(Format format) { _format = format; }
        [[nodiscard]] Format GetFormat() const { return _format; }

        void SetVirtualAddress(std::uint64_t virtualAddress) { _virtualAddress = virtualAddress; }
        [[nodiscard]] std::uint64_t GetVirtualAddress() const { return _virtualAddress; }

        void SetType(ResourceViewType viewType) { _viewType = viewType; }
        [[nodiscard]] ResourceViewType GetType() const { return _viewType; }

        void SetOffset(std::uint64_t offset) { _offset = offset; }
        [[nodiscard]] std::uint64_t GetOffset() const { return _offset; }

        void SetSize(std::uint32_t size) { _size = size; }
        [[nodiscard]] std::uint32_t GetSize() const { return _size; }

        void SetFirstElement(std::uint32_t firstElement) { _firstElement = firstElement; }
        [[nodiscard]] std::uint32_t GetFirstElement() const { return _firstElement; }

        void SetNumElements(std::uint32_t numElements) { _numElements = numElements; }
        [[nodiscard]] std::uint32_t GetNumElements() const { return _numElements; }

        void SetStride(std::uint32_t stride) { _stride = stride; }
        [[nodiscard]] std::uint32_t GetStride() const { return _stride; }

        [[nodiscard]] Buffer* GetCounterResource() const { return _counterResource; }

        void SetUAVCounterOffset(std::uint64_t uavCounterOffset) { _uavCounterOffset = uavCounterOffset; }
        [[nodiscard]] std::uint64_t GetUAVCounterOffset() const { return _uavCounterOffset; }

    private:
        Buffer* _buffer;

        ResourceViewType _viewType;

        Format _format;

        std::uint64_t _offset = 0;
        std::uint32_t _size = 0;

        std::uint64_t _firstElement = 0;
        std::uint32_t _numElements = 0;
        std::uint32_t _stride = 0;

        std::uint64_t _virtualAddress = 0;

        Buffer* _counterResource = nullptr;
        std::uint64_t _uavCounterOffset = 0;
    };
} // namespace rhi
