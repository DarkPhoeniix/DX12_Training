#include "EditorPCH.h"

#include "DebugInfoWidget.h"

#include "Core/RenderSettings.h"
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
        , _openDetailedCPUTime(false)
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
            int id = 0;
            if (ImGui::TreeNode((void*)id++, "Frame Time: %.03f ms", DebugInfo::GetMsPerFrame()))
            {
                ImGui::Text("Update Time: %.03f ms", DebugInfo::GetUpdateCPUTime());
                ImGui::Text("Render Time: %.03f ms", DebugInfo::GetRenderCPUTime());

                ImGui::TreePop();
            }
        
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

                if (ImGui::Checkbox("Use IBL", &RenderSettings::UseIBL()))
                {
                    pendingUpdate = true;
                }
                if (ImGui::Checkbox("Use FXAA", &RenderSettings::UseFXAA()))
                {
                    pendingUpdate = true;
                }
                if (ImGui::Checkbox("Debug FXAA", &RenderSettings::DebugFXAA()))
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
                if (ImGui::TreeNode((void*)id++, "Bloom", RenderSettings::UseBloom()))
                {
                    if (ImGui::Checkbox("Use bloom", &RenderSettings::UseBloom()))
                    {
                        pendingUpdate = true;
                    }

                    ImGui::DragFloat("Intensity", &RenderSettings::Bloom().Intensity, 0.001f, 0.001f, 1.0f);
                    ImGui::DragFloat("Intensity1", &RenderSettings::Bloom().Intensity1, 0.001f, 0.001f, 1.0f);
                    ImGui::DragFloat("Radius", &RenderSettings::Bloom().Radius, 0.01f, 0.01f, 5.0f, "%.2f");

                    ImGui::TreePop();
                }
                if (ImGui::TreeNode((void*)id++, "SSAO"))
                {
                    if (ImGui::Checkbox("Use SSAO", &RenderSettings::UseSSAO()))
                    {
                        pendingUpdate = true;
                    }

                    ImGui::DragFloat("Radius", &RenderSettings::SSAO().Radius, 0.01f, 0.01f, 5.0f, "%.2f");
                    ImGui::DragFloat("Bias", &RenderSettings::SSAO().Bias, 0.001f, 0.0f, 0.5f, "%.3f");
                    if (ImGui::DragInt("Blur radius", &RenderSettings::SSAO().BlurRadius, 1, 1, 10, "%d"))
                    {
                        pendingUpdate = true;
                    }
                    ImGui::DragFloat("Depth threshold", &RenderSettings::SSAO().DepthThreshold, 0.01f, 0.0f, 1.0f, "%.2f");
                    ImGui::DragFloat("Sharpness", &RenderSettings::SSAO().Sharpness, 1.0f, 1.0f, 200.0f, "%.0f");

                    ImGui::TreePop();
                }
                if (ImGui::TreeNode((void*)id++, "Debug view"))
                {
                    static int selected = -1;

                    if (ImGui::Selectable("None", selected == 0)) 
                    {
                        selected = 0;
                        RenderSettings::DebugView().DisableAll();
                        pendingUpdate = true;
                    }
                    if (ImGui::Selectable("Show albedo", selected == 1)) 
                    {
                        selected = 1;
                        RenderSettings::DebugView().DisableAll();
                        RenderSettings::DebugView().ShowAlbedo = true;
                        pendingUpdate = true;
                    }
                    if (ImGui::Selectable("Show normals", selected == 2)) 
                    {
                        selected = 2;
                        RenderSettings::DebugView().DisableAll();
                        RenderSettings::DebugView().ShowNormals = true;
                        pendingUpdate = true;
                    }
                    if (ImGui::Selectable("Show metalness", selected == 3)) 
                    {
                        selected = 3;
                        RenderSettings::DebugView().DisableAll();
                        RenderSettings::DebugView().ShowMetalness = true;
                        pendingUpdate = true;
                    }
                    if (ImGui::Selectable("Show roughness", selected == 4)) 
                    {
                        selected = 4;
                        RenderSettings::DebugView().DisableAll();
                        RenderSettings::DebugView().ShowRoughness = true;
                        pendingUpdate = true;
                    }
                    if (ImGui::Selectable("Show depth", selected == 5)) 
                    {
                        selected = 5;
                        RenderSettings::DebugView().DisableAll();
                        RenderSettings::DebugView().ShowDepth = true;
                        pendingUpdate = true;
                    }
                    if (ImGui::Selectable("Show SSAO", selected == 6)) 
                    {
                        selected = 6;
                        RenderSettings::DebugView().DisableAll();
                        RenderSettings::DebugView().ShowSSAO = true;
                        pendingUpdate = true;
                    }
                    if (ImGui::Selectable("Show bloom", selected == 7)) 
                    {
                        selected = 7;
                        RenderSettings::DebugView().DisableAll();
                        RenderSettings::DebugView().ShowBloom = true;
                        pendingUpdate = true;
                    }

                    ImGui::TreePop();
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
