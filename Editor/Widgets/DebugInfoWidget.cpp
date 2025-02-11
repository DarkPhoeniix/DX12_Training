#include "EditorPCH.h"

#include "DebugInfoWidget.h"

#include "Editor.h"
#include "Utility/DebugInfo.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace gui
{
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
                ImGui::Text(std::string("Primitives: " + std::to_string(stats.IAPrimitives)).c_str());
                ImGui::Text(std::string("VS invocs: " + std::to_string(stats.VSInvocations)).c_str());
                ImGui::Text(std::string("GS invocs: " + std::to_string(stats.GSInvocations)).c_str());
                ImGui::Text(std::string("PS invocs: " + std::to_string(stats.PSInvocations)).c_str());
            }
        
            if (ImGui::CollapsingHeader("inputs"))
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

            ImGui::EndChild();
        }
    }
} // namespace gui
