#pragma once

#include "DescriptorHeap.h"

namespace dx12
{
    class CommandList;
} // namespace dx12

namespace scene
{
    class Scene;
} // namespace scene

LRESULT GUI_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace gui
{
    class DebugInfoWidget;
    class SceneTreeWidget;
    
    class Editor
    {
    public:
        Editor(const Editor& copy) = delete;
        Editor operator=(const Editor& copy) = delete;

        static void Init(HWND windowhandle);
        static void Destroy();

        static void NewFrame();
        static void Update();
        static void Render(dx12::CommandList& commandList);

        static void SetScene(scene::Scene* scene);
        static scene::Scene* GetScene();

    private:
        Editor();
        ~Editor() = default;

        static void CreateWidgets();

        static Editor& Instance();

        scene::Scene* _scene;

        std::shared_ptr<SceneTreeWidget> _sceneTreeWidget;
        std::shared_ptr<DebugInfoWidget> _debugInfoWidget;

        std::shared_ptr<dx12::DescriptorHeap> _srvDescriptorHeap;

        static Editor* _instance;
    };
} // namespace gui
