#include "RendererPCH.h"

#include "DrawHelpers.h"

#include "Scene/Entity/Components/Camera.h"

namespace render
{
    std::unique_ptr<DrawHelper> DrawHelper::_instance = nullptr;

    void DrawHelper::Init()
    {
        ASSERT(!_instance, "DrawHelper has alreade been initialized");

        _instance = std::unique_ptr<DrawHelper>(new DrawHelper);
    }

    void DrawHelper::Destroy()
    {
        if (_instance)
        {
            _instance.reset();
        }
    }

    void DrawHelper::DrawSphere(dx12::CommandList& commandList, const scene::Camera& camera, float radius, const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& color)
    {
        ASSERT(_instance != nullptr, "DrawHelper has not been initialized");

        commandList.SetPipelineState(_instance->_sphereDebug);

        commandList.SetConstants(0, 16, &camera.ViewProjection());
        commandList.SetConstants(1, 3, &position);
        commandList.SetConstants(1, 1, &radius, 3);
        commandList.SetConstants(2, 4, &color);

        commandList.Draw(1);
    }

    void DrawHelper::DrawCone(dx12::CommandList& commandList, const scene::Camera& camera, float angle, float height, const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& direction, const DirectX::XMVECTOR& color)
    {
        ASSERT(_instance != nullptr, "DrawHelper has not been initialized");

        commandList.SetPipelineState(_instance->_coneDebug);

        commandList.SetConstants(0, 16, &camera.ViewProjection());
        commandList.SetConstants(1, 3, &position);
        commandList.SetConstants(1, 1, &angle, 3);
        commandList.SetConstants(1, 3, &direction, 4);
        commandList.SetConstants(1, 1, &height, 7);
        commandList.SetConstants(2, 4, &color);

        commandList.Draw(1);
    }

    DrawHelper::DrawHelper()
    {
        _sphereDebug.Parse("PipelineDescriptions\\DebugSpherePipeline.tech");
        _coneDebug.Parse("PipelineDescriptions\\DebugConePipeline.tech");
    }
} // namespace render
