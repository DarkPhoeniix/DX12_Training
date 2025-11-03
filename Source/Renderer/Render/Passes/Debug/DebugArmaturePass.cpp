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
	DebugArmaturePass::DebugArmaturePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugArmaturePassData>("debug_armature_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugArmaturePipeline.Parse("PipelineDescriptions\\ArmatureDebugPipeline.tech");
	}

	void DebugArmaturePass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.Target = builder.RenderTarget("render_target");
		_data.Depth = builder.DepthStencilWrite("depth_target");
	}

	void DebugArmaturePass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("debug_armature_cmd_list");

		{
            PIXScopedEvent(commandList.GetDXCommandList().Get(), 9, "Debug View Pass - Armature");

			std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);
			std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);

			DescriptorHandle rtv = context.GetStaticResourceHandle(target->GetAsRTV());
			DescriptorHandle dsv = context.GetStaticResourceHandle(depth->GetAsDSV());

			commandList.SetPipelineState(_debugArmaturePipeline);

			commandList.SetViewport(_camera->GetViewport().GetDXViewport(), _camera->GetViewport().GetScissorRectangle());
			commandList.SetRenderTarget(&rtv.CpuHandle, &dsv.CpuHandle);

			commandList.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());

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
							commandList.SetConstants(1, 8, &passCB);

							commandList.Draw(1);
						}
					}
				}
			}
		}

		commandList.Close();
	}
} // namespace render
