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
	GeometryPass::GeometryPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<GeometryPassData>("geometry_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_geometryPipeline.Parse("PipelineDescriptions\\GPassPipeline.tech");
	}

	void GeometryPass::Setup(rg::RenderPassBuilder& builder)
	{
		dx12::ResourceDescription depthDesc;
		{
			D3D12_CLEAR_VALUE clearValue;
			clearValue.Format = DXGI_FORMAT_D32_FLOAT;
			clearValue.DepthStencil.Depth = 1;
			clearValue.DepthStencil.Stencil = 0;

			depthDesc.SetSize(_camera->GetViewport().GetSize());
			depthDesc.SetFormat(DXGI_FORMAT_D32_FLOAT);
			depthDesc.SetClearValue(clearValue);
			depthDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::DepthStencil);
		}
        builder.DeclareTexture("depth_target", depthDesc);

		dx12::ResourceDescription albedoMetallicDesc;
		{
			D3D12_CLEAR_VALUE clearValue;
			clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			clearValue.Color[0] = 0.0f;
			clearValue.Color[1] = 0.0f;
			clearValue.Color[2] = 0.0f;
			clearValue.Color[3] = 1.0f;

			albedoMetallicDesc.SetSize(_camera->GetViewport().GetSize());
			albedoMetallicDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
			albedoMetallicDesc.SetClearValue(clearValue);
			albedoMetallicDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::RenderTarget);
		}
        builder.DeclareTexture("albedo_metallic_target", albedoMetallicDesc);

		dx12::ResourceDescription normalRoughnessDesc;
		{
			D3D12_CLEAR_VALUE clearValue;
			clearValue.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			clearValue.Color[0] = 0.0f;
			clearValue.Color[1] = 0.0f;
			clearValue.Color[2] = 0.0f;
			clearValue.Color[3] = 0.0f;

			normalRoughnessDesc.SetSize(_camera->GetViewport().GetSize());
			normalRoughnessDesc.SetFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
			normalRoughnessDesc.SetClearValue(clearValue);
			normalRoughnessDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::RenderTarget);
		}
        builder.DeclareTexture("normal_roughness_target", normalRoughnessDesc);

		dx12::ResourceDescription emissionDesc;
		{
			D3D12_CLEAR_VALUE clearValue;
			clearValue.Format = DXGI_FORMAT_R11G11B10_FLOAT;
			clearValue.Color[0] = 0.0f;
			clearValue.Color[1] = 0.0f;
			clearValue.Color[2] = 0.0f;
			clearValue.Color[3] = 1.0f;

			emissionDesc.SetSize(_camera->GetViewport().GetSize());
			emissionDesc.SetFormat(DXGI_FORMAT_R11G11B10_FLOAT);
			emissionDesc.SetClearValue(clearValue);
			emissionDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::RenderTarget);
		}
        builder.DeclareTexture("emission_target", emissionDesc);

        _data.AlbedoMetallic = builder.RenderTarget("albedo_metallic_target");
        _data.NormalRoughness = builder.RenderTarget("normal_roughness_target");
        _data.Emission = builder.RenderTarget("emission_target");
        _data.Depth = builder.DepthStencilWrite("depth_target");
	}

	void GeometryPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("geometry_pass_cmd_list");

		{
			PIXScopedEvent(commandList.GetDXCommandList().Get(), 0, "Geometry Pass");

            // Prepare all targets and pipeline state
            _SetupPipelineState(context, commandList);

#if ENABLE_PROFILING
            DebugInfo::StartStatCollecting(commandList);	// Start collecting pipeline statistics (primitive counts, etc.)
#endif

            // Set frame constant buffer
			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());

            // Filter all mesh entities
			std::vector<std::shared_ptr<scene::Entity>> entities = _scene->FilterNodesByComponent("Mesh");
			if (RenderSettings::EnableCPUFrustumCulling())
			{
				_CullPassEntities(entities);
			}

            // Issue draw calls for all selected entities
            _PopulateDrawCommands(commandList, entities);

#if ENABLE_PROFILING
			DebugInfo::EndStatCollecting(commandList);
#endif
		}

		commandList.Close();
	}

	void GeometryPass::_SetupPipelineState(rg::RenderContext& context, dx12::CommandList& commandList)
	{
		std::shared_ptr<dx12::Resource> albedoMetallic = context.GetResource(_data.AlbedoMetallic);
		std::shared_ptr<dx12::Resource> emission = context.GetResource(_data.Emission);
		std::shared_ptr<dx12::Resource> normalRoughness = context.GetResource(_data.NormalRoughness);
		std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);

		DescriptorHandle albedoMetallicHandle = context.GetStaticResourceHandle(albedoMetallic->GetAsRTV());
		DescriptorHandle emissionHandle = context.GetStaticResourceHandle(emission->GetAsRTV());
		DescriptorHandle normalRoughnessHandle = context.GetStaticResourceHandle(normalRoughness->GetAsRTV());
		DescriptorHandle depthHandle = context.GetStaticResourceHandle(depth->GetAsDSV());

		commandList.ClearDSV(depthHandle.CpuHandle, D3D12_CLEAR_FLAG_DEPTH);
		float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		commandList.ClearRTV(albedoMetallicHandle.CpuHandle, color, &_camera->GetViewport().GetScissorRectangle());
		commandList.ClearRTV(emissionHandle.CpuHandle, color, &_camera->GetViewport().GetScissorRectangle());
		color[3] = 0.0f;
		commandList.ClearRTV(normalRoughnessHandle.CpuHandle, color, &_camera->GetViewport().GetScissorRectangle());

		context.BindBindlessTable(commandList);

		commandList.SetPipelineState(_geometryPipeline);

		commandList.SetViewport(_camera->GetViewport().GetDXViewport(), _camera->GetViewport().GetScissorRectangle());
		commandList.SetRenderTargets({ albedoMetallicHandle.CpuHandle, normalRoughnessHandle.CpuHandle, emissionHandle.CpuHandle }, &depthHandle.CpuHandle);
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

	void GeometryPass::_PopulateDrawCommands(dx12::CommandList& commandList, const std::vector<std::shared_ptr<scene::Entity>>& entities)
	{
		auto DrawMeshEntity = [&](dx12::CommandList& commandList, std::uint32_t instanceIndex)
			{
                std::shared_ptr<scene::Entity> entity = entities[instanceIndex];
				if (std::shared_ptr<scene::Mesh> mesh = entity->GetComponentAs<scene::Mesh>("Mesh"))
				{
					PassConstants passConstants =
					{
						.InstanceIndex = entity->GetInstanceID()
					};

					commandList.SetConstants(1, sizeof(PassConstants), &passConstants);

					commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					commandList.SetVertexBuffer(0, mesh->VertexBufferView);
					if (!mesh->SkinningVertexData.empty())
					{
						commandList.SetVertexBuffer(1, mesh->SkinningVertexBufferView);
					}
					else
					{
						commandList.SetVertexBuffer(1, mesh->VertexBufferView);
					}
					commandList.SetIndexBuffer(mesh->IndexBufferView);

					commandList.DrawIndexed(mesh->IndexData.size());
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
