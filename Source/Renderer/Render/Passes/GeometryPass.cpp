#include "RendererPCH.h"

#include "GeometryPass.h"

#include "Core/RenderSettings.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Helpers/DebugInfo.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
	struct PassConstants
	{
		std::uint32_t InstanceIndex;
	};
} // namespace unnamed

namespace render
{
	GeometryPass::GeometryPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<GeometryPassData>(device, "geometry_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_geometryPipeline = _device->CreatePipelineState("PipelineDescriptions\\GPassPipeline.tech");
	}

	void GeometryPass::Setup(rg::RenderPassBuilder& builder)
	{
		rhi::TextureDescription depthDesc =
		{
			.Width = _camera->GetSize().x,
			.Height = _camera->GetSize().y,
			.ClearValue = { .DepthStencil = { 1.0f, 0 } },
			.Format = rhi::Format::D32_FLOAT,
			.Dimension = rhi::TextureDimension::Texture2D,
			.Flags = rhi::ResourceFlags::AllowDepthStencil
		};
        builder.DeclareTexture("depth_target", depthDesc);

		rhi::TextureDescription albedoMetallicDesc =
		{
			.Width = _camera->GetSize().x,
			.Height = _camera->GetSize().y,
			.ClearValue = { .Color = { 0.0f, 0.0f, 0.0f, 1.0f } },
			.Format = rhi::Format::R8G8B8A8_UNORM,
			.Dimension = rhi::TextureDimension::Texture2D,
			.Flags = rhi::ResourceFlags::AllowRenderTarget
		};
        builder.DeclareTexture("albedo_metallic_target", albedoMetallicDesc);

		rhi::TextureDescription normalRoughnessDesc =
		{
			.Width = _camera->GetSize().x,
			.Height = _camera->GetSize().y,
			.Format = rhi::Format::R32G32B32A32_FLOAT,
			.Dimension = rhi::TextureDimension::Texture2D,
			.Flags = rhi::ResourceFlags::AllowRenderTarget
		};
        builder.DeclareTexture("normal_roughness_target", normalRoughnessDesc);

		rhi::TextureDescription emissionDesc =
		{
			.Width = _camera->GetSize().x,
			.Height = _camera->GetSize().y,
			.ClearValue = { .Color = { 0.0f, 0.0f, 0.0f, 1.0f } },
			.Format = rhi::Format::R11G11B10_FLOAT,
			.Dimension = rhi::TextureDimension::Texture2D,
			.Flags = rhi::ResourceFlags::AllowRenderTarget
		};
        builder.DeclareTexture("emission_target", emissionDesc);

        _data.AlbedoMetallic = builder.RenderTarget("albedo_metallic_target");
        _data.NormalRoughness = builder.RenderTarget("normal_roughness_target");
        _data.Emission = builder.RenderTarget("emission_target");
        _data.Depth = builder.DepthStencilWrite("depth_target");
	}

	void GeometryPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
			GPU_SCOPED_EVENT(commandList, "Geometry Pass", 0);

            // Prepare all targets and pipeline state
            _SetupPipelineState(context, commandList);

#if ENABLE_GPU_PROFILING
            DebugInfo::StartStatCollecting(commandList);	// Start collecting pipeline statistics (primitive counts, etc.)
#endif // ENABLE_GPU_PROFILING

            // Filter all mesh entities
			std::vector<std::shared_ptr<scene::Entity>> entities = _scene->FilterNodesByComponent("Mesh");
			if (RenderSettings::EnableCPUFrustumCulling())
			{
				_CullPassEntities(entities);
			}

            // Issue draw calls for all selected entities
            _PopulateDrawCommands(commandList, entities);

#if ENABLE_GPU_PROFILING
			DebugInfo::EndStatCollecting(commandList);
#endif // ENABLE_GPU_PROFILING
		}

		commandList->Close();
	}

	void GeometryPass::_SetupPipelineState(rg::RenderContext& context, rhi::CommandList* commandList)
	{
		rhi::CPUDescriptor albedoMetallicHandle = context.GetDescriptor(_data.AlbedoMetallic, rhi::ResourceViewType::RTV);
		rhi::CPUDescriptor normalRoughnessHandle = context.GetDescriptor(_data.NormalRoughness, rhi::ResourceViewType::RTV);
		rhi::CPUDescriptor emissionHandle = context.GetDescriptor(_data.Emission, rhi::ResourceViewType::RTV);
		rhi::CPUDescriptor depthHandle = context.GetDescriptor(_data.Depth, rhi::ResourceViewType::DSV);

		commandList->ClearDSV(depthHandle);
		float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		commandList->ClearRTV(albedoMetallicHandle, color, &_camera->GetScissorRectangle());
		commandList->ClearRTV(emissionHandle, color, &_camera->GetScissorRectangle());
		color[3] = 0.0f;
		commandList->ClearRTV(normalRoughnessHandle, color, &_camera->GetScissorRectangle());

		commandList->SetGraphicsPipelineState(_geometryPipeline.get());

		commandList->SetViewport(_camera->GetViewport(), _camera->GetScissorRectangle());
		commandList->SetRenderTargets({ albedoMetallicHandle, normalRoughnessHandle, emissionHandle }, &depthHandle);

		commandList->SetGraphicsCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
	}

	void GeometryPass::_CullPassEntities(std::vector<std::shared_ptr<scene::Entity>>& entities)
	{
        const scene::FrustumVolume& frustum = _camera->GetViewFrustum();

		auto it = std::remove_if(entities.begin(), entities.end(),
			[&](const std::shared_ptr<scene::Entity>& entity)
			{
				if (std::shared_ptr<scene::Mesh> mesh = entity->GetComponentAs<scene::Mesh>("Mesh"))
				{
					return !Intersect(frustum, mesh->GlobalAABB);
				}

                // skip culling if no mesh component found
                return true;
			});
        entities.erase(it, entities.end());
	}

	void GeometryPass::_PopulateDrawCommands(rhi::CommandList* commandList, const std::vector<std::shared_ptr<scene::Entity>>& entities)
	{
		auto DrawMeshEntity = [&](rhi::CommandList* commandList, std::uint32_t instanceIndex)
			{
                std::shared_ptr<scene::Entity> entity = entities[instanceIndex];
				if (std::shared_ptr<scene::Mesh> mesh = entity->GetComponentAs<scene::Mesh>("Mesh"))
				{
					PassConstants passConstants =
					{
						.InstanceIndex = entity->GetInstanceID()
					};
					commandList->SetGraphicsConstants(1, sizeof(PassConstants), &passConstants);

					commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::TriangleList);
					commandList->SetVertexBuffer(0, mesh->VertexBufferView);
					if (!mesh->SkinningVertexData.empty())
					{
						commandList->SetVertexBuffer(1, mesh->SkinningVertexBufferView);
					}
					else
					{
						commandList->SetVertexBuffer(1, mesh->VertexBufferView);
					}
					commandList->SetIndexBuffer(mesh->IndexBufferView);

					commandList->DrawIndexed(mesh->IndexData.size());
				}
				else
				{
					LOG_CRITICAL("Entity '{}' does not have a Mesh component.", entities[instanceIndex]->GetName());
				}
			};

		for (size_t i = 0; i < entities.size(); ++i)
		{
			DrawMeshEntity(commandList, i);
		}
	}
} // namespace render
