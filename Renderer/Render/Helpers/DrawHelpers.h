#pragma once

#include "CommandList.h"

namespace scene
{
    class Camera;
} // namespace scene

namespace render
{
    class DrawHelper
    {
    public:
        ~DrawHelper() = default;

        DrawHelper(const DrawHelper&) = delete;
        DrawHelper& operator=(const DrawHelper&) = delete;

        static void Init();
        static void Destroy();

        static void DrawBox(dx12::CommandList& commandList,
            const scene::Camera& camera,
            const DirectX::XMVECTOR& min,
            const DirectX::XMVECTOR& max,
            const DirectX::XMVECTOR& color = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));

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

        dx12::PipelineState _boxDebug;
        dx12::PipelineState _sphereDebug;
        dx12::PipelineState _coneDebug;

        static std::unique_ptr<DrawHelper> _instance;
    };
} // namespace render
