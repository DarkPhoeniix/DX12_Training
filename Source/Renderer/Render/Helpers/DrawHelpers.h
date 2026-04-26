#pragma once


class Frame;
namespace rhi
{
    class CommandList;
    class PipelineState;
} // namespace rhi

namespace render
{
    class DrawHelper
    {
    public:
        ~DrawHelper() = default;

        DrawHelper(const DrawHelper&) = delete;
        DrawHelper& operator=(const DrawHelper&) = delete;

        static void Init(rhi::Device* device);
        static void Destroy();

        static void DrawFullscreenTriangle(rhi::CommandList* commandList);

        static void DrawBox(rhi::CommandList* commandList,
            std::uint64_t frameBufferAddress,
            const DirectX::XMVECTOR& min,
            const DirectX::XMVECTOR& max,
            const DirectX::XMVECTOR& color = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));

        static void DrawSphere(rhi::CommandList* commandList,
            std::uint64_t frameBufferAddress,
            float radius = 1.0f,
            const DirectX::XMVECTOR& position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
            const DirectX::XMVECTOR& color = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));

        static void DrawCone(rhi::CommandList* commandList,
            std::uint64_t frameBufferAddress,
            float radius = 1.0f,
            float height = 1.0f,
            const DirectX::XMVECTOR& position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
            const DirectX::XMVECTOR& direction = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f),
            const DirectX::XMVECTOR& color = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));

    private:
        DrawHelper(rhi::Device* device);

        std::unique_ptr<rhi::PipelineState> _boxDebug;
        std::unique_ptr<rhi::PipelineState> _sphereDebug;
        std::unique_ptr<rhi::PipelineState> _coneDebug;

        static std::unique_ptr<DrawHelper> _instance;
    };
} // namespace render
