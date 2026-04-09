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
	AmbientLightingPass::AmbientLightingPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<AmbientLightingPassData>(device, "ambient_lighting_pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
        , _useIBL(RenderSettings::UseIBL())
	{
        bool hasSkybox = _scene->FindNodeByComponentName("Skybox") != nullptr;
		if (_useIBL && hasSkybox)
		{
			_ambientLightingPipeline = _device->CreatePipelineState("PipelineDescriptions\\AmbientLightingIBLPipeline.tech");
		}
		else
		{
			_ambientLightingPipeline = _device->CreatePipelineState("PipelineDescriptions\\AmbientLightingPipeline.tech");
		}
	}

	void AmbientLightingPass::Setup(rg::RenderPassBuilder& builder)
	{
		rhi::TextureDescription targetDesc =
		{
			.Width = _camera->GetViewport().GetSize().x,
			.Height = _camera->GetViewport().GetSize().y,
			.Format = rhi::Format::R16G16B16A16_FLOAT,
			.Dimension = rhi::TextureDimension::Texture2D,
			.Flags = rhi::ResourceFlags::AllowUnorderedAccess
		};
        builder.DeclareTexture("hdr_target", targetDesc);

        _data.HDRTarget				= builder.WriteTexture("hdr_target");
        _data.AlbedoMetallic		= builder.ReadTexture("albedo_metallic_target");
        _data.NormalRoughness		= builder.ReadTexture("normal_roughness_target");
        _data.Depth					= builder.DepthStencilRead("depth_target");
        _data.DiffuseIrradianceMap	= _useIBL ? builder.ReadTexture("diffuse_irradiance_map")		: rg::RGTextureId::InvalidID;
        _data.PreFilteredMap		= _useIBL ? builder.ReadTexture("prefiltered_environment_map")	: rg::RGTextureId::InvalidID;
        _data.BRDF_LUT				= _useIBL ? builder.ReadTexture("brdf_lut")						: rg::RGTextureId::InvalidID;
	}

	void AmbientLightingPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
			GPU_SCOPED_EVENT(commandList, "Ambient Lighting Pass", 3);

			commandList->SetComputePipelineState(_ambientLightingPipeline.get());

			PassConstants passCB =
			{
				.DepthTextureIndex = context.GetBindlessIndex(_data.Depth, rhi::ResourceViewType::SRV),
				.AlbedoMetallicTextureIndex = context.GetBindlessIndex(_data.AlbedoMetallic, rhi::ResourceViewType::SRV),
				.NormalRoughnessTextureIndex = context.GetBindlessIndex(_data.NormalRoughness, rhi::ResourceViewType::SRV),
				.DiffuseIrradianceCubemapIndex = context.GetBindlessIndex(_data.DiffuseIrradianceMap, rhi::ResourceViewType::SRV),
				.PreFilteredEnvironmentCubemapIndex = context.GetBindlessIndex(_data.PreFilteredMap, rhi::ResourceViewType::SRV),
				.BRDFLUTTextureIndex = context.GetBindlessIndex(_data.BRDF_LUT, rhi::ResourceViewType::SRV),
				.TargetTextureIndex = context.GetBindlessIndex(_data.HDRTarget, rhi::ResourceViewType::UAV),
			};

			commandList->SetComputeConstants(1, 7, &passCB);

			DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
			int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

			commandList->Dispatch(xThreadGroups, yThreadGroups);
		}

		commandList->Close();
	}
} // namespace render