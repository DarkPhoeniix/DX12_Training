#include "EditorPCH.h"

#include "DebugInfoWidget.h"

#include "Utility/DebugInfo.h"

namespace gui
{
    namespace
    {
        std::string ConvertWCharToString(const WCHAR* wideStr)
        {
            if (!wideStr) return "";

            size_t size_needed = 128;
            std::string result(size_needed, 0);
            size_t i;
            wcstombs_s(&i, &result[0], size_needed, wideStr, size_needed - 1);

            return result;
        }
    }

    void DebugInfoWidget::Init()
    {
        IWidget::Init();
    }

    void DebugInfoWidget::Destroy()
    {
        IWidget::Destroy();
    }

    void DebugInfoWidget::Update()
    {
        IWidget::Update();

        if (ImGui::BeginChild("Debug Info", {0,0}, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY))
        {
            ImGui::Text("FPS: %i (%.03f ms)", DebugInfo::GetFPS(), DebugInfo::GetMsPerFrame());
        
            if (ImGui::CollapsingHeader("Pipeline statistics"))
            {
                D3D12_QUERY_DATA_PIPELINE_STATISTICS stats = DebugInfo::GetPipelineStatisctics();
                ImGui::Text("Primitives: %i", stats.IAPrimitives);
                ImGui::Text("VS invocs: %i", stats.VSInvocations);
                ImGui::Text("GS invocs: %i", stats.GSInvocations);
                ImGui::Text("PS invocs: %i", stats.PSInvocations);
            }
        
            if (ImGui::CollapsingHeader("Inputs"))
            {
                ImGuiIO& io = ImGui::GetIO();
                if (ImGui::IsMousePosValid())
                {
                    ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
                }
                else
                {
                    ImGui::Text("Mouse pos: <INVALID>");
                }
                ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
            }

            if (ImGui::CollapsingHeader("Adapter"))
            {
                DXGI_ADAPTER_DESC desc;
                dx12::Device::GetDXAdapter()->GetDesc(&desc);
                std::string a = ConvertWCharToString(desc.Description);
                ImGui::Text("Adapter: %s", a.c_str());

                DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo = {};
                dx12::Device::GetDXAdapter()->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo);

                ImGui::Text("Memory usage:  %i MB", memoryInfo.CurrentUsage / (1024 * 1024));
                ImGui::Text("Memory budget: %i MB", memoryInfo.Budget / (1024 * 1024));
            }
        }
        ImGui::EndChild();
    }
} // namespace gui
