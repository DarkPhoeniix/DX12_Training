#pragma once

#include "ISceneProcessor.h"

class UploadSceneProcessor : public ISceneProcessor
{
public:
    void Process(SceneLayer::Scene& scene, dx12::CommandList& commandList) override;

    void ProcessEntity(SceneLayer::Entity& entity, dx12::CommandList& commandList);

private:
    void UploadData(dx12::CommandList& commandList,
        ID3D12Resource** destinationResource,
        size_t numElements,
        size_t elementSize,
        const void* bufferData,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    std::vector<ComPtr<ID3D12Resource>> _intermediates;
};
