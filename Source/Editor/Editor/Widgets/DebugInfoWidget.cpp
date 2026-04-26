#include "EditorPCH.h"

#include "DebugInfoWidget.h"

#include "Renderer/Core/RenderSettings.h"
#include "Renderer/Helpers/Profiler.h"

#include "RenderGraph/RenderGraph.h"

#include "Helpers/DebugInfo.h"

namespace gui
{
    DebugInfoWidget::DebugInfoWidget(rhi::Device* device, Editor* editor)
        : IWidget(editor)
        , _openDetailedCPUTime(false)
        , _device(device)
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
            int id = 0;
#if ENABLE_CPU_PROFILING || ENABLE_GPU_PROFILING
            ImGui::Text("FPS: %i", DebugInfo::GetFPS());
            if (ImGui::TreeNode((void*)id++, "Frame Time: %.03f ms", DebugInfo::GetMsPerFrame()))
            {
                if (Profiler* profiler = _editor->GetRenderGraph()->GetProfiler())
                {
#if ENABLE_CPU_PROFILING 
                    const Profiler::Stats& cpuStats = profiler->GetCPUStats();
                    if (ImGui::TreeNode((void*)id++, "CPU Time: %.03f ms", cpuStats.FrameTimeMs))
                    {
                        float totalCPUTime = 0.0f;
                        for (const auto& timerResult : cpuStats.TimerResults)
                        {
                            const std::string& timerName = profiler->GetTimerName(timerResult.ID);
                            ImGui::Text("%s: %.03f ms", timerName.c_str(), timerResult.TimeMs);

                            totalCPUTime += static_cast<float>(timerResult.TimeMs);
                        }

                        ImGui::Text(" - Total: %.03f ms", totalCPUTime);

                        ImGui::TreePop();
                    }
#endif // ENABLE_CPU_PROFILING

#if ENABLE_GPU_PROFILING
                    const Profiler::Stats& gpuStats = profiler->GetGPUStats();
                    if (ImGui::TreeNode((void*)id++, "GPU Time: %.03f ms", gpuStats.FrameTimeMs))
                    {
                        float totalGPUTime = 0.0f;
                        for (const auto& timerResult : gpuStats.TimerResults)
                        {
                            const std::string& timerName = profiler->GetTimerName(timerResult.ID);
                            ImGui::Text("%s: %.03f ms", timerName.c_str(), timerResult.TimeMs);

                            totalGPUTime += static_cast<float>(timerResult.TimeMs);
                        }

                        ImGui::Text(" - Total: %.03f ms", totalGPUTime);

                        ImGui::TreePop();
                    }
#endif // ENABLE_GPU_PROFILING
                }

                ImGui::TreePop();
            }
#endif // ENABLE_CPU_PROFILING || ENABLE_GPU_PROFILING
        
            if (ImGui::CollapsingHeader("Pipeline statistics"))
            {
                const rhi::PipelineStatistics& stats = DebugInfo::GetPipelineStatisctics();
                ImGui::Text("* Geometry pass only");
                ImGui::Text("Primitives: %i", stats.IAPrimitives);
                ImGui::Text("VS invocations: %i", stats.VSInvocations);
                ImGui::Text("GS invocations: %i", stats.GSInvocations);
                ImGui::Text("PS invocations: %i", stats.PSInvocations);
            }
        
            if (ImGui::CollapsingHeader("Render settings"))
            {
                bool pendingUpdate = false;

                ImGui::Checkbox("Enable CPU frustum culling", &RenderSettings::EnableCPUFrustumCulling());

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
                    if (ImGui::Selectable("Show SSAO", selected == 6)) 
                    {
                        selected = 6;
                        RenderSettings::DebugView().DisableAll();
                        RenderSettings::DebugView().ShowSSAO = true;
                        pendingUpdate = true;
                    }
                    if (ImGui::Selectable("Show emission", selected == 7)) 
                    {
                        selected = 7;
                        RenderSettings::DebugView().DisableAll();
                        RenderSettings::DebugView().ShowEmission = true;
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
                const rhi::AdapterInfo& adapterInfo = _device->QueryAdapterInfo();
                ImGui::Text("Adapter: %s", adapterInfo.Name.c_str());

                ImGui::Text("Memory usage:  %i MB", adapterInfo.VideoMemory.CurrentUsage / (1024 * 1024));
                ImGui::Text("Memory budget: %i MB", adapterInfo.VideoMemory.Budget / (1024 * 1024));
            }
        }
        ImGui::EndChild();
    }
} // namespace gui
