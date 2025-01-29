#pragma once

#include "CommandList.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
    class DrawHelper
    {
    public:
        static void DrawSphere(dx12::CommandList& commandList,
            const scene::Camera& camera,
            float radius = 1.0f,
            const DirectX::XMVECTOR& position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
            const DirectX::XMVECTOR& color = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));

        static void DrawCone(dx12::CommandList& commandList,
            const scene::Camera& camera,
            float radius = 1.0f,
            float height = 1.0f,
            const DirectX::XMVECTOR& position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
            const DirectX::XMVECTOR& direction = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f),
            const DirectX::XMVECTOR& color = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));

    private:
        DrawHelper();
        ~DrawHelper() = default;

        static DrawHelper& Instance();

        dx12::PipelineState _sphereDebug;
        dx12::PipelineState _coneDebug;
    };
} // namespace render
