#include "RendererPCH.h"

#include "GUIPass.h"

#include "GUI/GUI.h"
#include "Scene/Entity/Components/Camera.h"
#include "Utility/DebugInfo.h"

namespace render
{
    void GUIPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _name = "GUIPass";
    }

    void GUIPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void GUIPass::Execute()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("gui");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Render GUI command list");

        dx12::DescriptorHeap& RTVHeap = _frame->GetDescriptorHeap(dx12::DescriptorHeapType::RTV);

        D3D12_CPU_DESCRIPTOR_HANDLE rtv = RTVHeap.GetResourceCPUHandle(&_frame->GetTargetTexture(), dx12::ResourceViewType::RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = _gBuffer->GetDepthTextureCPUHandle();

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "GUI");
        {
            commandList.SetViewport(_activeCamera->GetViewport());
            commandList.SetRenderTarget(&rtv, &dsv);

            if (ImGui::Begin("Debug Info"), true, ImGuiWindowFlags_AlwaysAutoResize)
            {
                ImGui::SetWindowPos({ 0, 0 });
                ImGui::SetWindowSize({ 0, 0 });

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
                    ImGui::Text("Mouse down:");
                    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
                    {
                        if (ImGui::IsMouseDown(i))
                        {
                            ImGui::SameLine();
                            ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]);
                        }
                    }

                    struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { return key >= 0 && key < 512 && ImGui::GetIO().KeyMap[key] != -1; } }; // Hide Native<>ImGuiKey duplicates when both exists in the array
                    ImGuiKey start_key = (ImGuiKey)0;

                    ImGui::Text("Keys down:");         for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key)) continue; ImGui::SameLine(); ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key); }
                    ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
                    ImGui::Text("Chars queue:");       for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; ImGui::SameLine();  ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
                }
            }
            ImGui::End();

            GUI::Render(commandList);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
