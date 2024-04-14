#include "stdafx.h"

#include "HeapDescription.h"

HeapDescription::HeapDescription()
    : _heapDescription()
{
}

void HeapDescription::SetAlignment(UINT64 alignment)
{
    _heapDescription.Alignment = alignment;
}

UINT64 HeapDescription::GetAlignment() const
{
    return _heapDescription.Alignment;
}

void HeapDescription::SetHeapFlags(D3D12_HEAP_FLAGS heapFlags)
{
    _heapDescription.Flags = heapFlags;
}

D3D12_HEAP_FLAGS HeapDescription::GetHeapFlags() const
{
    return _heapDescription.Flags;
}

void HeapDescription::SetSize(UINT64 size)
{
    _heapDescription.SizeInBytes = size;
}

UINT64 HeapDescription::GetSize() const
{
    return _heapDescription.SizeInBytes;
}

void HeapDescription::SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY property)
{
    _heapDescription.Properties.CPUPageProperty = property;
}

D3D12_CPU_PAGE_PROPERTY HeapDescription::GetCPUPageProperty() const
{
    return _heapDescription.Properties.CPUPageProperty;
}

void HeapDescription::SetMemoryPoolPreference(D3D12_MEMORY_POOL memPoolPreference)
{
    _heapDescription.Properties.MemoryPoolPreference = memPoolPreference;
}

D3D12_MEMORY_POOL HeapDescription::GetMemoryPoolPreference() const
{
    return _heapDescription.Properties.MemoryPoolPreference;
}

void HeapDescription::SetCreationNodeMask(UINT nodeMask)
{
    _heapDescription.Properties.CreationNodeMask = nodeMask;
}

UINT HeapDescription::GetCreationNodeMask() const
{
    return _heapDescription.Properties.CreationNodeMask;
}

void HeapDescription::SetVisibleNodeMask(UINT nodeMask)
{
    _heapDescription.Properties.VisibleNodeMask = nodeMask;
}

UINT HeapDescription::GetVisibleNodeMask() const
{
    return _heapDescription.Properties.VisibleNodeMask;
}

void HeapDescription::SetHeapType(D3D12_HEAP_TYPE heapType)
{
    _heapDescription.Properties.Type = heapType;
}

D3D12_HEAP_TYPE HeapDescription::GetHeapType() const
{
    return _heapDescription.Properties.Type;
}

const D3D12_HEAP_DESC& HeapDescription::GetDXHeapDescription()
{
    return _heapDescription;
}
