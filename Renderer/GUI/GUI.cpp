#include "stdafx.h"

#include "GUI.h"

#include "DXObjects/GraphicsCommandList.h"
#include "DXObjects/SwapChain.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

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

void GUI::Init(HWND windowHandle, const Core::SwapChain& swapChain)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(windowHandle);
    ImGui_ImplDX12_Init(Core::Device::GetDXDevice().Get(), 
                        Core::BACK_BUFFER_COUNT, 
                        swapChain.GetDescription().BufferDesc.Format,
                        Instance()._srvDescriptorHeap->GetDXDescriptorHeap().Get(),
                        Instance()._srvDescriptorHeap->GetHeapStartCPUHandle(),
                        Instance()._srvDescriptorHeap->GetHeapStartGPUHandle());
}

void GUI::NewFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow(); // Show demo window! :)
}

void GUI::Render(Core::GraphicsCommandList& commandList)
{
    ImGui::Render();
    commandList.SetDescriptorHeaps({ Instance()._srvDescriptorHeap->GetDXDescriptorHeap().Get() });
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.GetDXCommandList().Get());
}

void GUI::Destroy()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    delete Instance()._srvDescriptorHeap;
}

GUI::GUI()
{
    Core::DescriptorHeapDescription desc;
    desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    desc.SetNumDescriptors(1);

    _srvDescriptorHeap = new Core::DescriptorHeap;
    _srvDescriptorHeap->SetDescription(desc);
    _srvDescriptorHeap->Create();
    _srvDescriptorHeap->SetName("GUI SRV descriptor heap");
}

GUI::~GUI()
{   }

GUI& GUI::Instance()
{
    static GUI instance;
    return instance;
}
