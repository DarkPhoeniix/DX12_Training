#pragma once

#include "ISceneProcessor.h"

class UploadSceneProcessor : public ISceneProcessor
{
public:
    void Process(SceneLayer::Scene& scene, Core::CommandList& commandList) override;

    void UploadEntity(SceneLayer::Entity& entity, Core::CommandList& commandList);

private:
    void UploadData(Core::CommandList& commandList,
        ID3D12Resource** destinationResource,
        size_t numElements,
        size_t elementSize,
        const void* bufferData,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    std::vector<ComPtr<ID3D12Resource>> _intermediates;
};
