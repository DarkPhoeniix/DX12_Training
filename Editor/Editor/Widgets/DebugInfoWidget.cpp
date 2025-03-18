#include "EditorPCH.h"

#include "DebugInfoWidget.h"

#include "Render/RenderSettings.h"
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

    DebugInfoWidget::DebugInfoWidget(std::shared_ptr<Editor> editor)
        : IWidget(editor)
    {
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
            ImGui::Text("FPS: %i", DebugInfo::GetFPS());
            ImGui::Text("CPU Time: %.03f ms", DebugInfo::GetMsPerFrame());
        
            if (ImGui::CollapsingHeader("Pipeline statistics"))
            {
                D3D12_QUERY_DATA_PIPELINE_STATISTICS stats = DebugInfo::GetPipelineStatisctics();
                ImGui::Text("* Geometry pass only");
                ImGui::Text("Primitives: %i", stats.IAPrimitives);
                ImGui::Text("VS invocations: %i", stats.VSInvocations);
                ImGui::Text("GS invocations: %i", stats.GSInvocations);
                ImGui::Text("PS invocations: %i", stats.PSInvocations);
            }
        
            if (ImGui::CollapsingHeader("Custom render passes"))
            {
                bool pendingUpdate = false;

                if (ImGui::Checkbox("Use FXAA", &RenderSettings::UseFXAA()))
                {
                    pendingUpdate = true;
                }
                if (ImGui::Checkbox("Render debug volumes", &RenderSettings::RenderDebugVolumes()))
                {
                    pendingUpdate = true;
                }
                if (ImGui::Checkbox("Render debug armature", &RenderSettings::RenderDebugArmature()))
                {
                    pendingUpdate = true;
                }

                if (pendingUpdate)
                {
                    // TODO: It's the kinda lousy solution... but it works for now
                    PostMessage(_editor->GetWindowHandle(), WM_PIPELINE_CHANGED, NULL, NULL);
                }
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
