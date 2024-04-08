#include "stdafx.h"
#include "DescriptorHeapDescription.h"

DescriptorHeapDescription::DescriptorHeapDescription()
    : _description{}
{
}

const D3D12_DESCRIPTOR_HEAP_DESC& DescriptorHeapDescription::GetDXDescription() const
{
    return _description;
}

D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapDescription::GetType() const
{
    return _description.Type;
}

void DescriptorHeapDescription::SetType(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    _description.Type = type;
}

UINT DescriptorHeapDescription::GetNumDescriptors() const
{
    return _description.NumDescriptors;
}

void DescriptorHeapDescription::SetNumDescriptors(UINT num)
{
    _description.NumDescriptors = num;
}

D3D12_DESCRIPTOR_HEAP_FLAGS DescriptorHeapDescription::GetFlags() const
{
    return _description.Flags;
}

void DescriptorHeapDescription::SetFlags(D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    _description.Flags = flags;
}

UINT DescriptorHeapDescription::GetNodeMask() const
{
    return _description.NodeMask;
}

void DescriptorHeapDescription::SetNodeMask(UINT mask)
{
    _description.NodeMask = mask;
}
