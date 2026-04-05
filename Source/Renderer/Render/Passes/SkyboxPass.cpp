#include "RendererPCH.h"

#include "SkyboxPass.h"

#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Skybox.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	namespace
	{
		struct PassConstants
		{
			std::uint32_t DepthTextureIndex;
			std::uint32_t SkyboxTextureIndex;
			std::uint32_t TargetTextureIndex;
		};
	}

	SkyboxPass::SkyboxPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<SkyboxPassData>(device, "skybox_pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_skyboxPipeline = _device->CreatePipelineState("PipelineDescriptions\\SkyboxPipeline.tech");
	}

	void SkyboxPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.Depth = builder.DepthStencilRead("depth_target");
		_data.HDRTarget = builder.WriteTexture("hdr_target");
		_data.Skybox = builder.ReadTexture("skybox");
	}

	void SkyboxPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		if (std::shared_ptr<scene::Entity> entity = _scene->FindNodeByComponentName("Skybox"))
		{
            GPU_SCOPED_EVENT(commandList, "Skybox Pass", 2);

			commandList->SetComputePipelineState(_skyboxPipeline.get());

			PassConstants passCB =
			{
				.DepthTextureIndex = context.GetBindlessIndex(_data.Depth, rhi::ResourceViewType::SRV),
				.SkyboxTextureIndex = context.GetBindlessIndex(_data.Skybox, rhi::ResourceViewType::SRV),
				.TargetTextureIndex = context.GetBindlessIndex(_data.HDRTarget, rhi::ResourceViewType::UAV)
			};
			commandList->SetComputeConstants(1, 3, &passCB);

			DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
			int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

			commandList->Dispatch(xThreadGroups, yThreadGroups);
		}

		commandList->Close();
	}
} // namespace render
