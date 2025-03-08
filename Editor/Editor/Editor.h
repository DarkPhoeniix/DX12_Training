#pragma once

#include "Window/IWindowEventListener.h"
#include "DescriptorHeap.h"

namespace dx12
{
    class CommandList;
} // namespace dx12

namespace scene
{
    class Entity;
    class Scene;
    class Viewport;
} // namespace scene

namespace rg
{
    class RenderGraph;
}

LRESULT GUI_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace gui
{
    class DebugInfoWidget;
    class SceneTreeWidget;
    class EntityComponentsWidget;
    
    class Editor : public core::events::IWindowEventListener, public std::enable_shared_from_this<Editor>
    {
    public:
        Editor(HWND windowHandle);
        Editor(const Editor& copy) = delete;
        Editor operator=(const Editor& copy) = delete;
        ~Editor() = default;

        void Init(std::shared_ptr<scene::Scene> scene);
        void Destroy();

        void SetRenderGraph(rg::RenderGraph* renderGraph);
        void AddGUIRenderPass();

        void NewFrame();
        void Update();
        void Render(dx12::CommandList& commandList);

        void SetScene(std::shared_ptr<scene::Scene> scene);
        std::shared_ptr<scene::Scene> GetScene();

        void SetViewport(scene::Viewport* viewport);
        scene::Viewport* GetViewport();

        void SetSelectedEntity(std::shared_ptr<scene::Entity> entity);
        std::shared_ptr<scene::Entity> GetSelectedEntity();

        // Inherited via IWindowEventListener
        void OnResize(core::events::ResizeEvent& e) override;

    protected:
        std::shared_ptr<Editor> GetPtr();

    private:
        void CreateWidgets();

        std::shared_ptr<scene::Scene> _scene;
        scene::Viewport* _activeViewport;
        std::shared_ptr<scene::Entity> _selectedEntity;

        rg::RenderGraph* _renderGraph;

        std::shared_ptr<SceneTreeWidget> _sceneTreeWidget;
        std::shared_ptr<DebugInfoWidget> _debugInfoWidget;
        std::shared_ptr<EntityComponentsWidget> _entityComponentsWidget;

        std::shared_ptr<dx12::DescriptorHeap> _srvDescriptorHeap;
    };
} // namespace gui
