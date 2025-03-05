#include "RendererPCH.h"

#include "ToneMappingPass.h"

#include "ResourceTable.h"

#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Skybox.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Transformation.h"

#include "Render/Helpers/GPUStructs.h"
#include "Render/Helpers/RenderHelpers.h"
#include "Utility/DebugInfo.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

#include "Render/Passes/PassResources.h"

namespace render
{
    namespace
    {
        constexpr std::uint32_t LUM_HISTOGRAM_BINS_NUM = 256;
        constexpr std::uint32_t LUM_HISTOGRAM_THREADS_NUM = 16;
        constexpr std::uint32_t TONE_MAPPING_THREADS_NUM = 8;

        constexpr float MIN_LOG_LUM = -10.0f;
        constexpr float MAX_LOG_LUM = 4.0f;
        constexpr float LOG_LUM_RANGE = (MAX_LOG_LUM - MIN_LOG_LUM);
        constexpr float RCP_LOG_LUM_RANGE = 1.0f / LOG_LUM_RANGE;

        constexpr float MIDDLE_GREY = 0.775f;
        constexpr float WHITE = 2.5f;
    } // namespace unnamed

    ToneMappingPass::ToneMappingPass(scene::Scene* scene, scene::Camera* camera)
        : RenderPass<ToneMappingPassData>("Tone Mapping Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _toneMappingPipeline.Parse("PipelineDescriptions\\ToneMappingPipeline.tech");
    }

    void ToneMappingPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.HDRTarget = builder.ReadResource(HDR_TARGET);
        _data.AverageLuminance = builder.ReadResource(AVERAGE_LUM);

        dx12::ResourceDescription targetDesc;
        {
            D3D12_CLEAR_VALUE clearValue;
            clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            clearValue.Color[0] = 0.0f;
            clearValue.Color[1] = 0.0f;
            clearValue.Color[2] = 0.0f;
            clearValue.Color[3] = 0.0f;

            targetDesc.SetSize(_camera->GetViewport().GetSize());
            targetDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
            targetDesc.SetClearValue(clearValue);
            targetDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::RenderTarget | dx12::ResourceType::Unordered);
        }
        _data.Target = builder.CreateResource(TARGET, targetDesc);
    }

    void ToneMappingPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Tone mapping command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 7, "Tone Mapping");
        {
            // Copy and setup needed resources

            std::shared_ptr<dx12::Resource> hdrTarget = context.GetResource(_data.HDRTarget);
            std::shared_ptr<dx12::Resource> avgLuminance = context.GetResource(_data.AverageLuminance);
            std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);

            D3D12_GPU_DESCRIPTOR_HANDLE hdrTargetHandle = context.GetGPUHandle(hdrTarget->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE targetHandle = context.GetGPUHandle(target->GetAsUAV());

            // Setup pipeline state

            commandList.SetPipelineState(_toneMappingPipeline);

            // Setup root signature components

            commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });

            commandList.SetConstants(0, 1, &MIDDLE_GREY);
            commandList.SetConstants(0, 1, &WHITE, 1);
            commandList.SetSRV(1, avgLuminance->OffsetGPU());
            commandList.SetDescriptorTable(2, hdrTargetHandle);
            commandList.SetDescriptorTable(3, targetHandle);

            // Execute

            DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(TONE_MAPPING_THREADS_NUM));
            std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(TONE_MAPPING_THREADS_NUM));

            commandList.Dispatch(xThreadGroups, yThreadGroups);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
