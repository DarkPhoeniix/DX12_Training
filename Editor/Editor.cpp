#include "EditorPCH.h"

#include "Editor.h"

#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"
#include "Widgets/DebugInfoWidget.h"
#include "Widgets/SceneTreeWidget.h"
#include "Widgets/EntityComponentsWidget.h"

#include "CommandList.h"
#include "SwapChain.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT GUI_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }
    // Doubtful, but okay
    return S_OK;
}

namespace gui
{
    Editor* Editor::_instance = nullptr;

    void Editor::Init(HWND windowHandle)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(windowHandle);
        ImGui_ImplDX12_Init(dx12::Device::GetDXDevice().Get(),
            dx12::BACK_BUFFER_COUNT,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            Instance()._srvDescriptorHeap->GetDXDescriptorHeap().Get(),
            Instance()._srvDescriptorHeap->GetHeapStartCPUHandle(),
            Instance()._srvDescriptorHeap->GetHeapStartGPUHandle());

        ImGuiStyle& style = ImGui::GetStyle();

        // light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
        style.Alpha = 1.0f;
        style.FrameRounding = 3.0f;
        style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
        //style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
        //style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.88f, 0.09f, 0.26f, 0.31f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.88f, 0.09f, 0.26f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.88f, 0.09f, 0.26f, 1.00f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.98f, 0.09f, 0.26f, 0.31f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.98f, 0.09f, 0.26f, 0.80f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.09f, 0.26f, 1.00f);
        //style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        //style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
        //style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        //style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
        //style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
        //style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        //style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

        float alpha_ = 0.7f;
        if (true)
        {
            for (int i = 0; i <= ImGuiCol_COUNT; i++)
            {
                ImVec4& col = style.Colors[i];
                float H, S, V;
                ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

                if (S < 0.1f)
                {
                    V = 1.0f - V;
                }
                ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
                if (col.w < 1.00f)
                {
                    col.w *= alpha_;
                }
            }
        }
        else
        {
            for (int i = 0; i <= ImGuiCol_COUNT; i++)
            {
                ImVec4& col = style.Colors[i];
                if (col.w < 1.00f)
                {
                    col.x *= alpha_;
                    col.y *= alpha_;
                    col.z *= alpha_;
                    col.w *= alpha_;
                }
            }
        }

        CreateWidgets();
    }

    void Editor::Destroy()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        Instance()._srvDescriptorHeap.reset();

        if (_instance)
        {
            delete _instance;
        }

        _instance = nullptr;
    }

    void Editor::NewFrame()
    {
        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void Editor::Update()
    {
        std::shared_ptr<scene::Entity> activeCamera = Instance()._scene->FindNodeByComponentName("Camera");
        scene::Camera* cameraComponent = activeCamera->GetComponentAs<scene::Camera>("Camera");
        scene::Viewport vp = cameraComponent->GetViewport();

        DirectX::XMUINT2 viewportSize = vp.GetSize();

        float positionX = (float)(viewportSize.x - (viewportSize.x * 0.2f));
        float positionY = 0.0f;
        float sizeX = (float)(viewportSize.x * 0.2f);
        float sizeY = (float)(viewportSize.y);

        ImGui::SetNextWindowPos({ 0.0f, 0.0f });
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });

        if (ImGui::Begin("Debug Information", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
        {
            Instance()._debugInfoWidget->Update();
        }
        ImGui::End();

        ImGui::SetNextWindowPos({ positionX, positionY });
        ImGui::SetNextWindowSize({ sizeX, sizeY });

        if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoMove))
        {
            Instance()._sceneTreeWidget->Update();
            Instance()._entityComponentsWidget->Update();
        }
        ImGui::End();
    }

    void Editor::Render(dx12::CommandList& commandList)
    {
        ImGui::Render();
        commandList.SetDescriptorHeaps({ Instance()._srvDescriptorHeap->GetDXDescriptorHeap().Get() });
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.GetDXCommandList().Get());
    }

    void Editor::SetScene(scene::Scene* scene)
    {
        Instance()._scene = scene;
    }

    scene::Scene* Editor::GetScene()
    {
        return Instance()._scene;
    }

    void Editor::SetSelectedEntity(std::shared_ptr<scene::Entity> entity)
    {
        Instance()._selectedEntity = entity;
    }

    std::shared_ptr<scene::Entity> Editor::GetSelectedEntity()
    {
        return Instance()._selectedEntity;
    }

    Editor::Editor()
        : _scene(nullptr)
        , _selectedEntity(nullptr)
    {
        dx12::DescriptorHeapDescription desc;
        desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        desc.SetNumDescriptors(1);

        _srvDescriptorHeap = std::make_shared<dx12::DescriptorHeap>();
        _srvDescriptorHeap->Create(desc);
        _srvDescriptorHeap->SetName("GUI SRV descriptor heap");
    }

    void Editor::CreateWidgets()
    {
        ASSERT(Instance()._scene, "Scene is not initialized");

        Instance()._sceneTreeWidget = std::make_shared<SceneTreeWidget>();
        Instance()._debugInfoWidget = std::make_shared<DebugInfoWidget>();
        Instance()._entityComponentsWidget = std::make_shared<EntityComponentsWidget>();
    }

    Editor& Editor::Instance()
    {
        if (!_instance)
        {
            _instance = new Editor;
        }

        return *_instance;
    }
} // namespace gui
