#include "stdafx.h"

#include "DescriptorHeap.h"

DescriptorHeap::DescriptorHeap()
    : _descriptorHeap(nullptr)
    , _descriptorHeapDescription{}
    , _heapIncrementSize(0)
    , _DXDevice(nullptr)
{   }

DescriptorHeap::DescriptorHeap(const DescriptorHeapDescription& description)
    : _descriptorHeap(nullptr)
    , _descriptorHeapDescription(description)
    , _heapIncrementSize(0)
    , _DXDevice(nullptr)
{   }

DescriptorHeap::~DescriptorHeap()
{
    _descriptorHeap = nullptr;
    _DXDevice = nullptr;
}

void DescriptorHeap::SetDescription(const DescriptorHeapDescription& description)
{
    _descriptorHeapDescription = description;
}

const DescriptorHeapDescription& DescriptorHeap::GetDescription() const
{
    return _descriptorHeapDescription;
}

void DescriptorHeap::Create()
{
    if (!_DXDevice)
        Logger::Log(LogType::Error, "Device is nullptr when trying to create descriptor heap");

    _DXDevice->CreateDescriptorHeap(&_descriptorHeapDescription.GetDXDescription(), IID_PPV_ARGS(&_descriptorHeap));

    for (UINT i = 0; i < _descriptorHeapDescription.GetNumDescriptors(); ++i)
    {
        _resourceIndex[i] = nullptr;
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHeapStartCPUHandle()
{
    return _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHeapStartGPUHandle()
{
    return _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceCPUHandle(UINT index)
{
    if (!_DXDevice)
        Logger::Log(LogType::Error, "Device is nullptr when trying to Get CPU descriptor handle increment size");

    D3D12_CPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += _heapIncrementSize * index;

    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceCPUHandle(Resource* resource)
{
    if (!_DXDevice)
        Logger::Log(LogType::Error, "Device is nullptr when trying to Get CPU descriptor handle increment size");

    if (!resource)
        Logger::Log(LogType::Error, "Trying to Get CPU handle for nullptr resource");

    auto result = std::find_if(_resourceIndex.begin(), _resourceIndex.end(), [resource](const auto& pair) { return pair.second == resource; });
    UINT index = (result != _resourceIndex.end()) ? result->first : (UINT)-1;

    if (index == (UINT)-1)
        Logger::Log(LogType::Error, "Trying to Get invalid resource CPU handle from descriptor heap");

    D3D12_CPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += _heapIncrementSize * index;

    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceGPUHandle(UINT index)
{
    if (!_DXDevice)
        Logger::Log(LogType::Error, "Device is nullptr when trying to Get GPU descriptor handle increment size");

    D3D12_GPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += _heapIncrementSize * index;

    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceGPUHandle(Resource* resource)
{
    if (!_DXDevice)
        Logger::Log(LogType::Error, "Device is nullptr when trying to Get GPU descriptor handle increment size");

    if (!resource)
        Logger::Log(LogType::Error, "Trying to Get GPU handle for nullptr resource");

    auto result = std::find_if(_resourceIndex.begin(), _resourceIndex.end(), [resource](const auto& pair) { return pair.second == resource; });
    UINT index = (result != _resourceIndex.end()) ? result->first : (UINT)-1;

    if (index == (UINT)-1)
        Logger::Log(LogType::Error, "Trying to Get invalid resource GPU handle from descriptor heap");

    D3D12_GPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += _heapIncrementSize * index;

    return handle;
}

UINT DescriptorHeap::GetFreeHandleIndex() const
{
    UINT result = (UINT)-1;

    for (const auto& [index, resource] : _resourceIndex)
    {
        if (!resource)
        {
            result = index;
            break;
        }
    }

    return result;
}

ComPtr<ID3D12DescriptorHeap> DescriptorHeap::GetDXDescriptorHeap() const
{
    return _descriptorHeap;
}

void DescriptorHeap::SetDevice(ComPtr<ID3D12Device2> device)
{
    _DXDevice = device;
}

ComPtr<ID3D12Device2> DescriptorHeap::GetDevice() const
{
    return _DXDevice;
}
