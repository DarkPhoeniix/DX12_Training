#include "EditorPCH.h"

#include "Editor.h"

#include "Core/DescriptorHeapManager.h"
#include "Scene/Scene.h"
#include "Widgets/DebugInfoWidget.h"
#include "Widgets/SceneTreeWidget.h"
#include "Widgets/EntityComponentsWidget.h"

#include "Editor/Render/GUIPass.h"

#include "RenderGraph/RenderGraph.h"

#include "RHI/CommandList.h"
#include "RHI/SwapChain.h"

#include <commdlg.h>

namespace gui
{
    Editor::Editor(rhi::Device* device, HWND windowHandle)
        : _windowHandle(windowHandle)
        , _scene(nullptr)
        , _selectedEntity(nullptr)
        , _device(device)
    {
        _uiLayer = ui::CreateUILayer(device, windowHandle);
    }

    void Editor::Init(std::shared_ptr<scene::Scene> scene)
    {
        _scene = scene;
        std::shared_ptr<scene::Entity> activeCamera = scene->FindNodeByComponentName("Camera");
        _activeCamera = activeCamera->GetComponentAs<scene::Camera>("Camera").get();

        CreateWidgets();
    }

    void Editor::Destroy()
    {
        _selectedEntity.reset();
        _sceneTreeWidget.reset();
        _debugInfoWidget.reset();
        _entityComponentsWidget.reset();
        _scene.reset();
        _uiLayer.reset();
    }

    void Editor::SetRenderGraph(rg::RenderGraph* renderGraph)
    {
        _renderGraph = renderGraph;
    }

    rg::RenderGraph* Editor::GetRenderGraph() const
    { 
        return _renderGraph;
    }

    void Editor::AddGUIRenderPass()
    {
        _renderGraph->AddPass(std::make_shared<render::GUIPass>(_device, this, _activeCamera));
        _renderGraph->Compile();
    }

    void Editor::NewFrame()
    {
        _uiLayer->Begin();
    }

    void Editor::Update()
    {
        DirectX::XMUINT2 viewportSize = _activeCamera->GetSize();

        float positionX = (float)(viewportSize.x - (viewportSize.x * 0.2f));
        float positionY = 0.0f;
        float sizeX = (float)(viewportSize.x * 0.2f);
        float sizeY = (float)(viewportSize.y);

        ImGui::SetNextWindowPos({ 0.0f, 0.0f });
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });

        ImVec2 menuSize = { 0.0f, 0.0f };

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Load"))
                {
                    OPENFILENAME open;
                    ZeroMemory(&open, sizeof(open));

                    _filepath[0] = '\0';

                    open.lStructSize = sizeof(OPENFILENAME);
                    open.lpstrFilter = L".scene\0*.scene\0\0";
                    open.nFileOffset = 1;
                    open.nMaxFile = 2048;
                    open.lpstrTitle = L"Desc...";
                    open.lpstrFile = _filepath;
                    open.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

                    WCHAR* ecfas;
                    if (GetOpenFileName(&open))
                    {
                        PostMessage(_windowHandle, WM_LOAD_SCENE, NULL, (LPARAM)_filepath);
                    }
                }
                ImGui::EndMenu();
            }

            menuSize = ImGui::GetWindowSize();
        }
        ImGui::EndMainMenuBar();


        ImGui::SetNextWindowPos({ 0.0f, menuSize.y });
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });

        if (ImGui::Begin("Debug Information", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
        {
            _debugInfoWidget->Update();
        }
        ImGui::End();

        ImGui::SetNextWindowPos({ positionX, menuSize.y });
        ImGui::SetNextWindowSize({ sizeX, sizeY - menuSize.y });

        if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoMove))
        {
            _sceneTreeWidget->Update();
            _entityComponentsWidget->Update();
        }
        ImGui::End();
    }

    void Editor::Render(rhi::CommandList* commandList)
    {
        _uiLayer->End(commandList);
    }

    void Editor::SetScene(std::shared_ptr<scene::Scene> scene)
    {
        _scene = scene;
    }

    std::shared_ptr<scene::Scene> Editor::GetScene()
    {
        return _scene;
    }

    void Editor::SetSelectedEntity(std::shared_ptr<scene::Entity> entity)
    {
        _selectedEntity = entity;
    }

    std::shared_ptr<scene::Entity> Editor::GetSelectedEntity()
    {
        return _selectedEntity;
    }

    HWND Editor::GetWindowHandle() const
    {
        return _windowHandle;
    }

    void Editor::OnResize(core::events::ResizeEvent& e)
    {
        AddGUIRenderPass();
    }

    void Editor::OnPipelineChanged()
    {
        AddGUIRenderPass();
    }

    void Editor::OnLoadScene(const std::string& filepath)
    {
        _selectedEntity = nullptr;

        AddGUIRenderPass();
    }

    void Editor::OnWindowEvent(const core::WindowEvent& windowEvent)
    {
        if (_uiLayer)
        {
            _uiLayer->OnWindowEvent(windowEvent);
        }
    }

    std::shared_ptr<Editor> Editor::GetPtr()
    {
        return shared_from_this();
    }

    void Editor::CreateWidgets()
    {
        ASSERT((_scene != nullptr), "Scene is not initialized");

        _sceneTreeWidget = std::make_shared<SceneTreeWidget>(this);
        _debugInfoWidget = std::make_shared<DebugInfoWidget>(_device, this);
        _entityComponentsWidget = std::make_shared<EntityComponentsWidget>(this);
    }
} // namespace gui
