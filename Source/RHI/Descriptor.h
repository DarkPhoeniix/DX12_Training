#pragma once

namespace rhi
{
    // DescriptorType represents the type of descriptor, which can be either CPU or GPU
    enum class DescriptorType
    {
        CPU,
        GPU
    };

    // TypedDescriptor is a template struct that represents a descriptor of a specific type (CPU or GPU). It encapsulates a descriptor pointer
    template<DescriptorType Type>
    struct TypedDescriptor
    {
        std::uint64_t ptr;

        TypedDescriptor() : ptr(std::uint64_t(-1)) {}
        bool operator==(const TypedDescriptor& other) const { return ptr == other.ptr; }
        bool operator!=(const TypedDescriptor& other) const { return ptr != other.ptr; }

        // Checks if the descriptor is valid by comparing the pointer to an invalid value (-1)
        bool Valid() const { return ptr != std::uint64_t(-1); }
        // Offsets the descriptor pointer by a specified amount
        void Offset(std::uint64_t offset) { ptr += offset; }
    };

    // Type alias for CPU descriptor
    using CPUDescriptor = TypedDescriptor<DescriptorType::CPU>;
    // Type alias for GPU descriptor
    using GPUDescriptor = TypedDescriptor<DescriptorType::GPU>;
} // namespace rhi
