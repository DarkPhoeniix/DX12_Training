#include "stdafx.h"

#include "UploadSceneProcessor.h"

#include "CommandList.h"
#include "ResourceTable.h"

#include "Scene/Scene.h"
#include "Scene/ECS/Components/Armature.h"
#include "Scene/ECS/Components/Animation.h"
#include "Scene/ECS/Components/Material.h"
#include "Scene/ECS/Components/Mesh.h"
#include "Scene/ECS/Components/Skybox.h"

#include "Render/Frame/CacheGPU.h"

void UploadSceneProcessor::Process(SceneLayer::Scene& scene, dx12::CommandList& commandList, CacheGPU* frameCache)
{
    SceneLayer::SceneCache& cache = scene.GetCache();

    for (std::shared_ptr<SceneLayer::Entity>& node : scene.GetRootNodes())
    {
        ProcessEntity(*node, commandList, frameCache);

        for (std::shared_ptr<SceneLayer::Entity>& child : node->GetChildrenNodes())
        {
            ProcessEntity(*child, commandList, frameCache);
        }
    }
}

void UploadSceneProcessor::ProcessEntity(SceneLayer::Entity& entity, dx12::CommandList& commandList, CacheGPU* frameCache)
{
    SceneLayer::SceneCache* cache = entity.GetSceneCache();
    if (ASSERT(cache, "Entity has no scene cache"))
    {
        return;
    }

    Mesh* mesh = entity.GetComponentAs<Mesh>("Mesh");
    // Create buffers for Mesh
    if (mesh)
    {
        ASSERT((!mesh->VertexData.empty() && !mesh->VertexData.empty()), "Mesh component has empty vertex ot index arrays");

        // Upload Vertex buffer
        {
            ComPtr<ID3D12Resource> vertexBuffer;
            UploadData(commandList, &vertexBuffer, mesh->VertexData.size(), sizeof(VertexData), mesh->VertexData.data());
            mesh->VertexBuffer = std::make_shared<dx12::Resource>();
            mesh->VertexBuffer->InitFromDXResource(vertexBuffer);
            mesh->VertexBuffer->SetName(entity.GetName() + "_VB");

            mesh->VertexBufferView.BufferLocation = mesh->VertexBuffer->OffsetGPU(0);
            mesh->VertexBufferView.SizeInBytes = static_cast<UINT>(mesh->VertexData.size() * sizeof(mesh->VertexData[0]));
            mesh->VertexBufferView.StrideInBytes = sizeof(VertexData);
        }

        // Upload Skinning Vertex buffer
        if (!mesh->SkinningVertexData.empty())
        {
            ComPtr<ID3D12Resource> skinBuffer;
            UploadData(commandList, &skinBuffer, mesh->SkinningVertexData.size(), sizeof(SkinningVertexData), mesh->SkinningVertexData.data());
            mesh->SkinningVertexBuffer = std::make_shared<dx12::Resource>();
            mesh->SkinningVertexBuffer->InitFromDXResource(skinBuffer);
            mesh->SkinningVertexBuffer->SetName(entity.GetName() + "_SVB");

            mesh->SkinningVertexBufferView.BufferLocation = mesh->SkinningVertexBuffer->OffsetGPU(0);
            mesh->SkinningVertexBufferView.SizeInBytes = static_cast<UINT>(mesh->SkinningVertexData.size() * sizeof(mesh->SkinningVertexData[0]));
            mesh->SkinningVertexBufferView.StrideInBytes = sizeof(SkinningVertexData);
        }

        // Upload Index buffer
        {
            ComPtr<ID3D12Resource> indexBuffer;
            UploadData(commandList, &indexBuffer, mesh->IndexData.size(), sizeof(UINT), mesh->IndexData.data());
            mesh->IndexBuffer = std::make_shared<dx12::Resource>();
            mesh->IndexBuffer->InitFromDXResource(indexBuffer);
            mesh->IndexBuffer->SetName(entity.GetName() + "_IB");

            mesh->IndexBufferView.BufferLocation = mesh->IndexBuffer->OffsetGPU(0);
            mesh->IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
            mesh->IndexBufferView.SizeInBytes = static_cast<UINT>(mesh->IndexData.size() * sizeof(mesh->IndexData[0]));
        }
    }

    Material* material = entity.GetComponentAs<Material>("Material");
    if (material)
    {
        std::shared_ptr<dx12::ResourceTable> textureTable = cache->GetTextureTable();
        ASSERT(textureTable.get(), "Resource table is nullptr");

        if (textureTable->AddResource(material->Albedo.get()))
        {
            material->Albedo->SetDescriptorHeap(&textureTable->GetDescriptorHeap());
            material->Albedo->UploadToGPU(commandList);
        }

        if (textureTable->AddResource(material->NormalMap.get()))
        {
            material->NormalMap->SetDescriptorHeap(&textureTable->GetDescriptorHeap());
            material->NormalMap->UploadToGPU(commandList);
        }

        if (textureTable->AddResource(material->Metalness.get()))
        {
            material->Metalness->SetDescriptorHeap(&textureTable->GetDescriptorHeap());
            material->Metalness->UploadToGPU(commandList);
        }

        if (textureTable->AddResource(material->Roughness.get()))
        {
            material->Roughness->SetDescriptorHeap(&textureTable->GetDescriptorHeap());
            material->Roughness->UploadToGPU(commandList);
        }
    }

    Armature* armature = entity.GetComponentAs<Armature>("Armature");
    if (armature)
    {
        dx12::ResourceDescription desc;
        {
            desc.SetResourceType(dx12::EResourceType::Buffer | dx12::EResourceType::Dynamic);
            desc.SetSize({ (uint32_t)armature->GetBones().size() * (uint32_t)sizeof(DirectX::XMMATRIX), 1});
            desc.SetFormat(DXGI_FORMAT_UNKNOWN);
            desc.SetFlags(D3D12_RESOURCE_FLAG_NONE);
        }

        armature->BoneTransforms.CreateCommitedResource(desc);
        armature->BoneTransforms.SetName(entity.GetName() + "_Bones");

        desc.SetSize({ ((uint32_t)armature->GetBones().size() - 1) * 2 * 16 , 1 });

        armature->BoneDebugTransforms.CreateCommitedResource(desc);
        armature->BoneDebugTransforms.SetName(entity.GetName() + "_DebugBones");
    }

    Skybox* skybox = entity.GetComponentAs<Skybox>("Skybox");
    if (skybox)
    {
        skybox->SkydomeTexture->SetDescriptorHeap(&skybox->DescHeap);

        skybox->DescHeap.PlaceResource(skybox->SkydomeTexture.get());
        skybox->TexHeap.PlaceResource(*skybox->SkydomeTexture);

        skybox->SkydomeTexture->UploadToGPU(commandList);

        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = skybox->SkydomeTexture->GetResourceDescription().GetFormat();
        SRVDesc.Texture2D.MipLevels = 1;
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        dx12::Device::GetDXDevice()->CreateShaderResourceView(skybox->SkydomeTexture->GetDXResource().Get(), &SRVDesc, skybox->DescHeap.GetResourceCPUHandle(skybox->SkydomeTexture.get()));
    }
}

void UploadSceneProcessor::UploadData(dx12::CommandList& commandList, ID3D12Resource** destinationResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
    size_t bufferSize = numElements * elementSize;

    CD3DX12_HEAP_PROPERTIES heapTypeDefault(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferWithFlags = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    Helper::throwIfFailed(dx12::Device::GetDXDevice()->CreateCommittedResource(
        &heapTypeDefault,
        D3D12_HEAP_FLAG_NONE,
        &bufferWithFlags,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(destinationResource)));

    CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC buffer = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    if (bufferData)
    {
        ComPtr<ID3D12Resource> intermediateResource = nullptr;

        Helper::throwIfFailed(dx12::Device::GetDXDevice()->CreateCommittedResource(
            &heapTypeUpload,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&intermediateResource)));
        intermediateResource->SetName(L"Intermediate");
        _intermediates.push_back(intermediateResource);

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.GetDXCommandList().Get(),
            *destinationResource, intermediateResource.Get(),
            0, 0, 1, &subresourceData);
    }
}
