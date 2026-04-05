#include "RendererPCH.h"

#include "DrawHelpers.h"

#include "Render/Frame/Frame.h"

#include "RHI/CommandList.h"
#include "RHI/PipelineState.h"

namespace render
{
    std::unique_ptr<DrawHelper> DrawHelper::_instance = nullptr;

    void DrawHelper::Init(rhi::Device* device)
    {
        if (!_instance)
        {
            _instance = std::unique_ptr<DrawHelper>(new DrawHelper(device));
        }
    }

    void DrawHelper::Destroy()
    {
        if (_instance)
        {
            _instance.reset();
        }
    }

    void DrawHelper::DrawFullscreenTriangle(rhi::CommandList* commandList)
    {
        NOT_IMPLEMENTED();
    }

    void DrawHelper::DrawBox(rhi::CommandList* commandList, const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max, const DirectX::XMVECTOR& color)
    {
        ASSERT(_instance, "DrawHelper has not been initialized");

        commandList->SetGraphicsPipelineState(_instance->_boxDebug.get());

        commandList->SetGraphicsConstants(1, 3, &min);
        commandList->SetGraphicsConstants(1, 3, &max, 4);
        commandList->SetGraphicsConstants(1, 4, &color, 8);

        commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::PointList);

        commandList->Draw(1);
    }

    void DrawHelper::DrawSphere(rhi::CommandList* commandList, float radius, const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& color)
    {
        ASSERT(_instance, "DrawHelper has not been initialized");

        commandList->SetGraphicsPipelineState(_instance->_sphereDebug.get());

        commandList->SetGraphicsConstants(1, 3, &position);
        commandList->SetGraphicsConstants(1, 1, &radius, 3);
        commandList->SetGraphicsConstants(1, 4, &color, 4);

        commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::PointList);

        commandList->Draw(1);
    }

    void DrawHelper::DrawCone(rhi::CommandList* commandList, float angle, float height, const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& direction, const DirectX::XMVECTOR& color)
    {
        ASSERT(_instance, "DrawHelper has not been initialized");

        commandList->SetGraphicsPipelineState(_instance->_coneDebug.get());

        commandList->SetGraphicsConstants(1, 3, &position);
        commandList->SetGraphicsConstants(1, 1, &angle, 3);
        commandList->SetGraphicsConstants(1, 3, &direction, 4);
        commandList->SetGraphicsConstants(1, 1, &height, 7);
        commandList->SetGraphicsConstants(1, 4, &color, 8);

        commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::PointList);

        commandList->Draw(1);
    }

    DrawHelper::DrawHelper(rhi::Device* device)
    {
        _boxDebug = device->CreatePipelineState("PipelineDescriptions\\AABBRenderPipeline.tech");
        _sphereDebug = device->CreatePipelineState("PipelineDescriptions\\DebugSpherePipeline.tech");
        _coneDebug = device->CreatePipelineState("PipelineDescriptions\\DebugConePipeline.tech");
    }
} // namespace render
