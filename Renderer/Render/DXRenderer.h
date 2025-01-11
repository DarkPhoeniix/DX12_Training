#pragma once

#include "Render/Frame/Frame.h"
#include "Render/GBuffer.h"
#include "Render/Passes/IRenderPass.h"

#include "Scene/Entity/Components/Camera.h"
#include "Scene/Scene.h"
#include "SceneProcessors/UploadSceneProcessor.h"

#include "Window/IWindowEventListener.h"

namespace render
{
    class DXRenderer : public core::events::IWindowEventListener
    {
    public:
        DXRenderer(HWND windowHandle);
        ~DXRenderer();

        virtual bool LoadContent(TaskGPU* loadTask);
        virtual void UnloadContent();

        virtual void SetFrame(Frame& frame);

        virtual void OnUpdate(core::events::UpdateEvent& e) override;
        virtual void OnRender(core::events::RenderEvent& e) override;
        virtual void OnKeyPressed(core::events::KeyEvent& e) override;
        virtual void OnKeyReleased(core::events::KeyEvent& e) override {}
        virtual void OnMouseMoved(core::events::MouseMoveEvent& e) override;
        virtual void OnMouseButtonPressed(core::events::MouseButtonEvent& e) override;
        virtual void OnMouseButtonReleased(core::events::MouseButtonEvent& e) override;
        virtual void OnMouseScroll(core::events::MouseScrollEvent& e) override {}
        virtual void OnResize(core::events::ResizeEvent& e) override;

    private:
        void SetupRenderPipeline();

        HWND _windowHandle;

        Frame* _currentFrame;

        render::GBuffer _gBuffer;
        scene::Scene _scene;
        std::shared_ptr<scene::Camera> _cameraComponent;

        UploadSceneProcessor _uploadProcessor;

        std::vector<std::unique_ptr<IRenderPass>> _renderPasses;

        bool _isCameraMoving;
        float _deltaTime;

        bool _renderArmature;
        bool _renderAABB;
        bool _renderSkybox;
        bool _applyFXAA;
        float _timeMiltiplier;

        bool _contentLoaded;
    };
} // namespace render
