
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "D3D12UILayer.h"

#include "Logger/Logger.h"

#include "Renderer/Core/DescriptorHeapManager.h"

#include "RHI/CommandList.h"
#include "RHI/DescriptorHeap.h"
#include "RHI/Device.h"
#include "RHI/SwapChain.h"

#include "Utility/Helpers.h"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ui
{
    D3D12UILayer::D3D12UILayer(rhi::Device* device, void* windowHandle)
        : _device(device)
    {
        rhi::DescriptorHeapDescription desc =
        {
            .Type = rhi::DescriptorHeapType::CBV_SRV_UAV,
            .NumDescriptors = 1,
            .ShaderVisible = true
        };

        _descriptorHeap = _device->CreateDescriptorHeap(desc, "ui_layer_descriptor_heap");

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);

        ID3D12Device* d3d12Device = static_cast<ID3D12Device*>(_device->GetNative());
        ID3D12DescriptorHeap* d3d12DescriptorHeap = static_cast<ID3D12DescriptorHeap*>(_descriptorHeap->GetNative());
        D3D12_CPU_DESCRIPTOR_HANDLE heapStartCPUHandle = { _descriptorHeap->GetHeapStartCPUHandle().ptr };
        D3D12_GPU_DESCRIPTOR_HANDLE heapStartGPUHandle = { _descriptorHeap->GetHeapStartGPUHandle().ptr };

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(static_cast<HWND>(windowHandle));
        ImGui_ImplDX12_Init(d3d12Device,
            rhi::BACK_BUFFER_COUNT,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            d3d12DescriptorHeap,
            heapStartCPUHandle,
            heapStartGPUHandle);

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
    }

    D3D12UILayer::~D3D12UILayer()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void D3D12UILayer::Begin()
    {
        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void D3D12UILayer::End(rhi::CommandList* commandList)
    {
        ImGui::Render();
        commandList->SetDescriptorHeaps(_descriptorHeap.get());

        ID3D12GraphicsCommandList* d3d12CommandList = static_cast<ID3D12GraphicsCommandList*>(commandList->GetNative());
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), d3d12CommandList);
    }

    void D3D12UILayer::OnWindowEvent(const core::WindowEvent& windowEvent)
    {
        ImGui_ImplWin32_WndProcHandler(static_cast<HWND>(windowEvent.WindowHandle),
            static_cast<UINT>(windowEvent.Message),
            static_cast<WPARAM>(windowEvent.WParam),
            static_cast<LPARAM>(windowEvent.LParam));
    }
} // namespace ui
