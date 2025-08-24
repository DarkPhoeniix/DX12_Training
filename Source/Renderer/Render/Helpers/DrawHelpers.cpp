#include "RendererPCH.h"

#include "DrawHelpers.h"

#include "Render/Frame/Frame.h"

namespace render
{
    std::unique_ptr<DrawHelper> DrawHelper::_instance = nullptr;

    void DrawHelper::Init()
    {
        if (!_instance)
        {
            _instance = std::unique_ptr<DrawHelper>(new DrawHelper);
        }
    }

    void DrawHelper::Destroy()
    {
        if (_instance)
        {
            _instance.reset();
        }
    }

    void DrawHelper::DrawBox(dx12::CommandList& commandList, const Frame& frame, const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max, const DirectX::XMVECTOR& color)
    {
        ASSERT(_instance, "DrawHelper has not been initialized");

        commandList.SetPipelineState(_instance->_boxDebug);

        commandList.SetCBV(0, frame._frameBuffer->OffsetGPU());
        commandList.SetConstants(1, 3, &min);
        commandList.SetConstants(1, 3, &max, 4);
        commandList.SetConstants(1, 4, &color, 8);

        commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

        commandList.Draw(1);
    }

    void DrawHelper::DrawSphere(dx12::CommandList& commandList, const Frame& frame, float radius, const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& color)
    {
        ASSERT(_instance, "DrawHelper has not been initialized");

        commandList.SetPipelineState(_instance->_sphereDebug);

        commandList.SetCBV(0, frame._frameBuffer->OffsetGPU());
        commandList.SetConstants(1, 3, &position);
        commandList.SetConstants(1, 1, &radius, 3);
        commandList.SetConstants(1, 4, &color, 4);

        commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

        commandList.Draw(1);
    }

    void DrawHelper::DrawCone(dx12::CommandList& commandList, const Frame& frame, float angle, float height, const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& direction, const DirectX::XMVECTOR& color)
    {
        ASSERT(_instance, "DrawHelper has not been initialized");

        commandList.SetPipelineState(_instance->_coneDebug);

		commandList.SetCBV(0, frame._frameBuffer->OffsetGPU());
        commandList.SetConstants(1, 3, &position);
        commandList.SetConstants(1, 1, &angle, 3);
        commandList.SetConstants(1, 3, &direction, 4);
        commandList.SetConstants(1, 1, &height, 7);
        commandList.SetConstants(1, 4, &color, 8);

        commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

        commandList.Draw(1);
    }

    DrawHelper::DrawHelper()
    {
        _boxDebug.Parse("PipelineDescriptions\\AABBRenderPipeline.tech");
        _sphereDebug.Parse("PipelineDescriptions\\DebugSpherePipeline.tech");
        _coneDebug.Parse("PipelineDescriptions\\DebugConePipeline.tech");
    }
} // namespace render
