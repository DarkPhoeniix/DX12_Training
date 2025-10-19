#include "RendererPCH.h"

#include "AmbientLightingPass.h"

#include "Core/RenderSettings.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
	struct PassConstants
	{
		std::uint32_t DepthTextureIndex;
		std::uint32_t AlbedoMetallicTextureIndex;
		std::uint32_t NormalRoughnessTextureIndex;
		std::uint32_t DiffuseIrradianceCubemapIndex;
		std::uint32_t PreFilteredEnvironmentCubemapIndex;
		std::uint32_t BRDFLUTTextureIndex;
		std::uint32_t TargetTextureIndex;
	};
}

namespace render
{
	AmbientLightingPass::AmbientLightingPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<AmbientLightingPassData>("ambient_lighting_pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		if (RenderSettings::UseIBL())
		{
			_ambientLightingPipeline.Parse("PipelineDescriptions\\AmbientLightingIBLPipeline.tech");
		}
		else
		{
			_ambientLightingPipeline.Parse("PipelineDescriptions\\AmbientLightingPipeline.tech");
		}
	}

	void AmbientLightingPass::Setup(rg::RenderPassBuilder& builder)
	{
		dx12::ResourceDescription targetDesc;
		{
			targetDesc.SetSize(_camera->GetViewport().GetSize());
			targetDesc.SetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
			targetDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
		}
        builder.DeclareTexture("hdr_target", targetDesc);

        _data.HDRTarget = builder.WriteTexture("hdr_target");
        _data.AlbedoMetallic = builder.ReadTexture("albedo_metallic_target");
        _data.NormalRoughness = builder.ReadTexture("normal_roughness_target");
        _data.Depth = builder.DepthStencilRead("depth_target");
        _data.DiffuseIrradianceMap = builder.ReadTexture("diffuse_irradiance_map");
        _data.PreFilteredMap = builder.ReadTexture("prefiltered_environment_map");
        _data.BRDF_LUT = builder.ReadTexture("brdf_lut");
	}

	void AmbientLightingPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("ambient_pass_cmd_list");

		{
			PIXScopedEvent(commandList.GetDXCommandList().Get(), 3, "Ambient Lighting Pass");

			std::shared_ptr<dx12::Resource> hdrTarget = context.GetResource(_data.HDRTarget);
			std::shared_ptr<dx12::Resource> albedoMetallic = context.GetResource(_data.AlbedoMetallic);
			std::shared_ptr<dx12::Resource> normalRoughness = context.GetResource(_data.NormalRoughness);
			std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);
			std::shared_ptr<dx12::Resource> diffuseIrradianceMap = context.GetResource(_data.DiffuseIrradianceMap);
			std::shared_ptr<dx12::Resource> preFilteredEnv = context.GetResource(_data.PreFilteredMap);
			std::shared_ptr<dx12::Resource> brdfLUT = context.GetResource(_data.BRDF_LUT);

			DescriptorHandle hdrTargetHandle = context.GetStaticResourceHandle(hdrTarget->GetAsUAV());
			DescriptorHandle albedoMetallicHandle = context.GetStaticResourceHandle(albedoMetallic->GetAsSRV());
			DescriptorHandle normalSpecularHandle = context.GetStaticResourceHandle(normalRoughness->GetAsSRV());
			DescriptorHandle depthHandle = context.GetStaticResourceHandle(depth->GetAsSRV());
			DescriptorHandle diffuseIrradianceMapHandle = context.GetStaticResourceHandle(diffuseIrradianceMap->GetAsSRV());
			DescriptorHandle preFilteredEnvHandle = context.GetStaticResourceHandle(preFilteredEnv->GetAsSRV());
			DescriptorHandle brdfLUTHandle = context.GetStaticResourceHandle(brdfLUT->GetAsSRV());

			context.BindBindlessTable(commandList);
			commandList.SetPipelineState(_ambientLightingPipeline);

			PassConstants passCB =
			{
				.DepthTextureIndex = depthHandle.Index,
				.AlbedoMetallicTextureIndex = albedoMetallicHandle.Index,
				.NormalRoughnessTextureIndex = normalSpecularHandle.Index,
				.DiffuseIrradianceCubemapIndex = diffuseIrradianceMapHandle.Index,
				.PreFilteredEnvironmentCubemapIndex = preFilteredEnvHandle.Index,
				.BRDFLUTTextureIndex = brdfLUTHandle.Index,
				.TargetTextureIndex = hdrTargetHandle.Index
			};

			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());
			commandList.SetConstants(1, 7, &passCB);

			DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
			int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

			commandList.Dispatch(xThreadGroups, yThreadGroups);
		}

		commandList.Close();
	}
} // namespace render