#pragma once

namespace rhi
{
    enum class DescriptorType
    {
        CPU,
        GPU
    };

    template<DescriptorType Type>
    struct TypedDescriptor
    {
        std::uint64_t ptr;

        TypedDescriptor() : ptr(std::uint64_t(-1)) {}
        bool operator==(const TypedDescriptor& other) const { return ptr == other.ptr; }
        bool operator!=(const TypedDescriptor& other) const { return ptr != other.ptr; }

        bool Valid() const { return ptr != std::uint64_t(-1); }
        void Offset(std::uint64_t offset) { ptr += offset; }
    };

    using CPUDescriptor = TypedDescriptor<DescriptorType::CPU>;
    using GPUDescriptor = TypedDescriptor<DescriptorType::GPU>;
} // namespace rhi
