#pragma once

#include "Core/DescriptorHeapManager.h"
#include "Core/ResourceTable.h"
#include "Core/TextureManager.h"

#include "Render/Frame/Frame.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Scene.h"
#include "Scene/SceneLoader.h"
#include "Helpers/Profiler.h"
#include "Window/IWindowEventListener.h"

#include "RenderGraph/RenderGraph.h"

namespace render
{
    class DXRenderer : public core::events::IWindowEventListener
    {
    public:
        DXRenderer(rhi::Device* device, HWND windowHandle);
        ~DXRenderer();

        virtual bool LoadContent(TaskGPU* loadTask, const std::string& filepath);
        virtual void UnloadContent();

        virtual void SetFrame(Frame& frame);

        rg::RenderGraph* GetRenderGraph();

        std::shared_ptr<scene::Scene> GetCurrentScene();

        // Inherited via IWindowEventListener
        void OnUpdate(core::events::UpdateEvent& e) override;
        void OnRender(core::events::RenderEvent& e) override;
        void OnKeyDown(core::events::KeyEvent& e) override;
        void OnKeyPressed(core::events::KeyEvent& e) override;
        void OnKeyReleased(core::events::KeyEvent& e) override;
        void OnMouseMoved(core::events::MouseMoveEvent& e) override;
        void OnMouseButtonPressed(core::events::MouseButtonEvent& e) override;
        void OnMouseButtonReleased(core::events::MouseButtonEvent& e) override;
        void OnMouseScroll(core::events::MouseScrollEvent& e) override {}
        void OnResize(core::events::ResizeEvent& e) override;
        void OnPipelineChanged() override;
        void OnLoadScene(const std::string& filepath) override;

    private:
        void UpdateSceneBuffers();
        void CreateShadowMap(std::shared_ptr<scene::Entity> light);
        void CreateShadowMaps();

        void UpdateEntity(std::shared_ptr<scene::Entity> entity);
        void UpdateBoundingVolumes(std::shared_ptr<scene::Entity> entity);

        void WaitAllFrames();
        void SetupRenderPipeline();

        HWND _windowHandle;

        Frame* _currentFrame;

        Profiler _gpuProfiler;
        Profiler::TimerID _frameTimeTimerID;

        std::unique_ptr<rg::RenderGraph> _renderGraph;

        scene::helpers::SceneLoader _sceneLoader;

        std::shared_ptr<scene::Scene> _scene;
        std::shared_ptr<scene::Camera> _cameraComponent;

        std::shared_ptr<rhi::Texture> _diffuseIrradianceMap;
        std::shared_ptr<rhi::Texture> _preFilteredEnvironmentMap;
        std::shared_ptr<rhi::Texture> _brdfLUT;

        rhi::Device* _device;

        bool _isMinimized;
        bool _isCameraMoving;
        bool _enableAbsoluteMovement;
        bool _enableAbsoluteMovementPrevState;
        float _deltaTime;

        bool _contentLoaded;

        enum class SceneBufferType
        {
            Model,
            Light,
            Count
        };
        std::array<std::shared_ptr<rhi::Buffer>, static_cast<size_t>(SceneBufferType::Count)> _sceneBuffers;
    };
} // namespace render
