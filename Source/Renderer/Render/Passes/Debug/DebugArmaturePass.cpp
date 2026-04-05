#include "RendererPCH.h"

#include "DebugArmaturePass.h"

#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Transformation.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
	struct PassConstants
	{
		DirectX::XMVECTOR BoneStart;
		DirectX::XMVECTOR BoneEnd;
	};
} // namespace unnamed

namespace render
{
	DebugArmaturePass::DebugArmaturePass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugArmaturePassData>(device, "debug_armature_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugArmaturePipeline = _device->CreatePipelineState("PipelineDescriptions\\ArmatureDebugPipeline.tech");
	}

	void DebugArmaturePass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.Target = builder.RenderTarget("render_target");
		_data.Depth = builder.DepthStencilWrite("depth_target");
	}

	void DebugArmaturePass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "Debug View Pass - Armature", 9);
			\
			commandList->SetGraphicsPipelineState(_debugArmaturePipeline.get());

			rhi::CPUDescriptor targetHandle = context.GetDescriptor(_data.Target, rhi::ResourceViewType::RTV);
			rhi::CPUDescriptor depthHandle = context.GetDescriptor(_data.Depth, rhi::ResourceViewType::DSV);

			commandList->SetViewport(_camera->GetViewport().GetDXViewport(), _camera->GetViewport().GetScissorRectangle());
			commandList->SetRenderTarget(&targetHandle, &depthHandle);

			commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::PointList);

			auto meshes = _scene->FilterNodesByComponent("Mesh");
			for (size_t i = 0; i < meshes.size(); ++i)
			{
				std::shared_ptr<scene::Armature> armature = meshes[i]->GetComponentAs<scene::Armature>("Armature");
				std::shared_ptr<scene::Transformation> transform = meshes[i]->GetComponentAs<scene::Transformation>("Transformation");

				if (armature)
				{
					const auto& sortedBones = armature->GetSortedBones();
					for (const auto& bone : sortedBones)
					{
						for (const auto& child : bone->Children)
						{
							PassConstants passCB
							{
								.BoneStart = DirectX::XMVector4Transform(bone->GlobalTransform.r[3], transform->Transform),
								.BoneEnd = DirectX::XMVector4Transform(child->GlobalTransform.r[3], transform->Transform)
							};
							commandList->SetGraphicsConstants(1, 8, &passCB);

							commandList->Draw(1);
						}
					}
				}
			}
		}

		commandList->Close();
	}
} // namespace render
