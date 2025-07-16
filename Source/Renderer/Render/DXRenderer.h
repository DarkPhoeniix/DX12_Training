#pragma once

#include "Render/Frame/Frame.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Scene.h"
#include "Scene/SceneLoader.h"
#include "Window/IWindowEventListener.h"

#include "RenderGraph/RenderGraph.h"

namespace render
{
    class DXRenderer : public core::events::IWindowEventListener
    {
    public:
        DXRenderer(HWND windowHandle);
        ~DXRenderer();

        virtual bool LoadContent(TaskGPU* loadTask, const std::string& filepath);
        virtual void UnloadContent();

        virtual void SetFrame(Frame& frame);

        rg::RenderGraph& GetRenderGraph();

        std::shared_ptr<scene::Scene> GetCurrentScene();

        // Inherited via IWindowEventListener
        void OnUpdate(core::events::UpdateEvent& e) override;
        void OnRender(core::events::RenderEvent& e) override;
        void OnKeyPressed(core::events::KeyEvent& e) override;
        void OnKeyReleased(core::events::KeyEvent& e) override {}
        void OnMouseMoved(core::events::MouseMoveEvent& e) override;
        void OnMouseButtonPressed(core::events::MouseButtonEvent& e) override;
        void OnMouseButtonReleased(core::events::MouseButtonEvent& e) override;
        void OnMouseScroll(core::events::MouseScrollEvent& e) override {}
        void OnResize(core::events::ResizeEvent& e) override;
        void OnPipelineChanged() override;
        void OnLoadScene(const std::string& filepath) override;

    private:
        void UpdateEntity(std::shared_ptr<scene::Entity> entity);
        void UpdateBoundingVolumes(std::shared_ptr<scene::Entity> entity);

        void WaitAllFrames();
        void SetupRenderPipeline();
        void UploadSceneCache(CacheGPU& cache, dx12::ResourceTable& table);

        HWND _windowHandle;

        Frame* _currentFrame;

        rg::RenderGraph _renderGraph;

        scene::helpers::SceneLoader _sceneLoader;

        std::shared_ptr<scene::Scene> _scene;
        std::shared_ptr<scene::Camera> _cameraComponent;

        std::shared_ptr<dx12::Resource> _diffuseIrradianceMap;
        std::shared_ptr<dx12::Resource> _preFilteredEnvironmentMap;
        std::shared_ptr<dx12::Resource> _brdfLUT;

        bool _isMinimized;
        bool _isCameraMoving;
        float _deltaTime;

        bool _contentLoaded;
    };
} // namespace render
