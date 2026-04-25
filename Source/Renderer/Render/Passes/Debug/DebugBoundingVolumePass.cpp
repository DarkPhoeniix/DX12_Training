#include "RendererPCH.h"

#include "DebugBoundingVolumePass.h"

#include "Render/Helpers/DrawHelpers.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Transformation.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugBoundingVolumePass::DebugBoundingVolumePass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugBoundingVolumePassData>(device, "debug_volumes_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
	}

	void DebugBoundingVolumePass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.Target = builder.RenderTarget("render_target");
		_data.Depth = builder.DepthStencilWrite("depth_target");
	}

	void DebugBoundingVolumePass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "Debug View Pass - Bounding Volumes", 9);

			rhi::CPUDescriptor targetHandle = context.GetDescriptor(_data.Target, rhi::ResourceViewType::RTV);
			rhi::CPUDescriptor depthHandle = context.GetDescriptor(_data.Depth, rhi::ResourceViewType::DSV);

			commandList->SetViewport(_camera->GetViewport(), _camera->GetScissorRectangle());
			commandList->SetRenderTarget(&targetHandle, &depthHandle);

			auto lights = _scene->FilterNodesByComponent("Light");
			for (auto& entity : lights)
			{
				std::shared_ptr<scene::Light> light = entity->GetComponentAs<scene::Light>("Light");
				std::shared_ptr<scene::Transformation> t = entity->GetComponentAs<scene::Transformation>("Transformation");

				switch (light->Type)
				{
				case scene::LightType::Point:
					DrawHelper::DrawSphere(commandList, context.GetFrameBuffer()->GetVirtualAddress(), light->Range, t->Transform.r[3], light->Color);
					break;
				case scene::LightType::Spot:
					DrawHelper::DrawCone(commandList, context.GetFrameBuffer()->GetVirtualAddress(), light->OuterAngle, light->Range, t->Transform.r[3], light->Direction, light->Color);
					break;
				}
			}

			auto meshes = _scene->FilterNodesByComponent("Mesh");
			for (auto& entity : meshes)
			{
				std::shared_ptr<scene::Mesh> mesh = entity->GetComponentAs<scene::Mesh>("Mesh");
				scene::AABBVolume aabb = mesh->GlobalAABB;

				DrawHelper::DrawBox(commandList, context.GetFrameBuffer()->GetVirtualAddress(), aabb.Min, aabb.Max, DirectX::XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
			}
		}

		commandList->Close();
	}
} // namespace render
