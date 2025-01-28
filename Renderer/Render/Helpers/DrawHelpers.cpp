#include "RendererPCH.h"

#include "DrawHelpers.h"

namespace render
{
    void DrawHelper::DrawSphere(dx12::CommandList& commandList, const scene::Camera& camera, float radius, const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& color)
    {
        commandList.SetPipelineState(Instance()._sphereDebug);

        DirectX::XMVECTOR pos = DirectX::XMVectorSet(0.0f, 30.0f, 0.0f, 25.0f);
        commandList.SetConstants(0, 16, &camera.ViewProjection());
        commandList.SetConstants(1, 4, &pos);
        DirectX::XMVECTOR col = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
        commandList.SetConstants(2, 4, &col);

        commandList.Draw(1);
    }

    DrawHelper::DrawHelper()
    {
        _sphereDebug.Parse("PipelineDescriptions\\DebugSpherePipeline.tech");
    }

    DrawHelper& DrawHelper::Instance()
    {
        static DrawHelper instance;
        return instance;
    }
} // namespace render
