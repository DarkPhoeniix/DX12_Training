#include "stdafx.h"

#include "DescriptorHeapDescription.h"

namespace Core
{
    DescriptorHeapDescription::DescriptorHeapDescription()
        : _description{}
    {   }

    const D3D12_DESCRIPTOR_HEAP_DESC& DescriptorHeapDescription::GetDXDescription() const
    {
        return _description;
    }

    void DescriptorHeapDescription::SetType(D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        _description.Type = type;
    }

    D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapDescription::GetType() const
    {
        return _description.Type;
    }

    void DescriptorHeapDescription::SetNumDescriptors(UINT num)
    {
        _description.NumDescriptors = num;
    }

    UINT DescriptorHeapDescription::GetNumDescriptors() const
    {
        return _description.NumDescriptors;
    }

    void DescriptorHeapDescription::SetFlags(D3D12_DESCRIPTOR_HEAP_FLAGS flags)
    {
        _description.Flags = flags;
    }

    D3D12_DESCRIPTOR_HEAP_FLAGS DescriptorHeapDescription::GetFlags() const
    {
        return _description.Flags;
    }

    void DescriptorHeapDescription::SetNodeMask(UINT mask)
    {
        _description.NodeMask = mask;
    }

    UINT DescriptorHeapDescription::GetNodeMask() const
    {
        return _description.NodeMask;
    }
} // namespace Core
