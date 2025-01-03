#include "RendererPCH.h"

#include "Skybox.h"

#include "DescriptorHeap.h"
#include "Heap.h"

Skybox::Skybox()
    : IComponent("Skybox")
{
    {
        dx12::DescriptorHeapDescription desc;
        desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        desc.SetNumDescriptors(10);
        desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

        DescHeap.Create(desc);
        DescHeap.SetName("Skybox descriptor heap");
    }

    {
        dx12::HeapDescription desc;
        desc.SetSize(_256MB * 2);
        desc.SetHeapType(D3D12_HEAP_TYPE_DEFAULT);
        desc.SetHeapFlags(D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES);

        TexHeap.Create(desc);
        TexHeap.SetName("Skybox heap");
    }
}
