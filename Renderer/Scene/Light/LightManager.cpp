#include "stdafx.h"

#include "LightManager.h"

#include "DXObjects/GraphicsCommandList.h"

#include "Scene/Light/DirectionalLight.h"
#include "Scene/Light/PointLight.h"

namespace
{
    struct GPULightDesc
    {
        DirectX::XMVECTOR position;
        DirectX::XMVECTOR direction;
        DirectX::XMVECTOR color;

        float intensity;
        float range;

        uint32_t type;

        uint32_t pad;
    };

    enum class LightType : uint32_t
    {
        Directional = 0,
        Point = 1
    };
} // namespace unnamed

namespace SceneLayer
{
    LightManager::LightManager()
        : _directionalLights{}
        , _pointLights{}
        , _internalResources{}
    {
        Core::HeapDescription heapDesc;
        heapDesc.SetHeapType(D3D12_HEAP_TYPE_UPLOAD);
        heapDesc.SetHeapFlags(D3D12_HEAP_FLAG_NONE);
        heapDesc.SetSize(_32MB);
        heapDesc.SetMemoryPoolPreference(D3D12_MEMORY_POOL_UNKNOWN);
        heapDesc.SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY_UNKNOWN);
        heapDesc.SetVisibleNodeMask(1);
        heapDesc.SetCreationNodeMask(1);

        _lightsHeap.SetDescription(heapDesc);
        _lightsHeap.Create();


        Core::DescriptorHeapDescription descHeapDesc;
        descHeapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        descHeapDesc.SetNumDescriptors(1);
        descHeapDesc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

        _descHeap.SetDescription(descHeapDesc);
        _descHeap.Create();



        Core::ResourceDescription desc;
        desc.SetResourceType(Core::EResourceType::Dynamic | Core::EResourceType::Buffer);
        desc.SetSize({ sizeof(GPULightDesc) * 20, 1 });
        desc.SetFormat(DXGI_FORMAT_UNKNOWN);
        desc.SetDepthOrArraySize(1);

        _lightsView = std::make_shared<Core::Resource>(desc);
        _lightsView->CreateCommitedResource(D3D12_RESOURCE_STATE_GENERIC_READ);

        D3D12_SHADER_RESOURCE_VIEW_DESC d;
        d.Buffer.FirstElement = 0;
        d.Buffer.NumElements = 20;
        d.Buffer.StructureByteStride = sizeof(GPULightDesc);
        d.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        d.Format = DXGI_FORMAT_UNKNOWN;
        d.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        d.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        Core::Device::GetDXDevice()->CreateShaderResourceView(_lightsView->GetDXResource().Get(), &d, _descHeap.GetHeapStartCPUHandle());
    }

    void LightManager::SetupLights(Core::GraphicsCommandList& commandList)
    {
        GPULightDesc* data = (GPULightDesc*)_lightsView->Map();
        int index = 0;
        for (const auto& l : _directionalLights)
        {
            GPULightDesc desc;
            desc.direction = l->GetDirection();
            desc.color = l->GetColor();
            desc.type = 0;
            data[index++] = desc;
        }

        for (const auto& l : _pointLights)
        {
            GPULightDesc desc;
            desc.position = l->GetGlobalTransform().r[3];
            desc.range = l->GetRange();
            desc.intensity = l->GetIntensity();
            desc.color = l->GetColor();
            desc.type = 1;
            data[index++] = desc;
        }

        commandList.SetSRV(2, _lightsView->OffsetGPU(0));
    }

    UINT LightManager::GetLightsNum() const
    {
        return _directionalLights.size() + _pointLights.size();
    }

    void LightManager::AddDirectionalLight(std::shared_ptr<DirectionalLight> directionalLight)
    {
        _directionalLights.push_back(directionalLight);

        std::shared_ptr<Core::Resource> light = CreateGPUResource();

        GPULightDesc* data = (GPULightDesc*)light->Map();
        {
            data->direction = directionalLight->GetDirection();
            data->color = directionalLight->GetColor();

            data->type = static_cast<uint32_t>(LightType::Directional);
        }
    }

    void LightManager::AddPointLight(std::shared_ptr<PointLight> pointLight)
    {
        _pointLights.push_back(pointLight);

        std::shared_ptr<Core::Resource> light = CreateGPUResource();

        GPULightDesc* data = (GPULightDesc*)light->Map();
        {
            data->position = pointLight->GetGlobalTransform().r[3];
            data->color = pointLight->GetColor();
            data->intensity = pointLight->GetIntensity();
            data->range = pointLight->GetRange();

            data->type = static_cast<uint32_t>(LightType::Point);
        }
    }

    std::shared_ptr<Core::Resource> LightManager::CreateGPUResource()
    {
        Core::ResourceDescription desc;
        desc.SetResourceType(Core::EResourceType::Buffer | Core::EResourceType::Dynamic);
        desc.SetSize({ sizeof(GPULightDesc), 1 });
        desc.SetFlags(D3D12_RESOURCE_FLAG_NONE);
        _internalResources.push_back(std::make_shared<Core::Resource>(desc));

        std::shared_ptr<Core::Resource> result = _internalResources.back();
        result->SetCurrentState(D3D12_RESOURCE_STATE_GENERIC_READ);
        _lightsHeap.PlaceResource(*result, D3D12_RESOURCE_STATE_GENERIC_READ);

        return result;
    }
} // namespace SceneLayer
