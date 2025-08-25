#include "RendererPCH.h"

#include "ShadowClearPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Core/TextureManager.h"
#include "Scene/Entity/Components/Light.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
	constexpr std::uint32_t CULLING_PASS_THREADS_NUM = 16;

	// Data structure to match the command signature used for ExecuteIndirect.
	struct IndirectCommand
	{
		D3D12_GPU_VIRTUAL_ADDRESS VertexBufferAddress;
		UINT VertexBufferSize;
		UINT VertexBufferStride;
		D3D12_GPU_VIRTUAL_ADDRESS SkinBufferAddress;
		UINT SkinBufferSize;
		UINT SkinBufferStride;
		D3D12_GPU_VIRTUAL_ADDRESS IndexBufferAddress;
		UINT IndexBufferSize;
		UINT IndexBufferFormat;
		D3D12_GPU_VIRTUAL_ADDRESS SceneBufferAddress;
		D3D12_GPU_VIRTUAL_ADDRESS ModelBufferAddress;
		D3D12_GPU_VIRTUAL_ADDRESS BonesBufferAddress;
		D3D12_GPU_VIRTUAL_ADDRESS LightsBufferAddress;
		UINT LightIndex;

		D3D12_DRAW_ARGUMENTS DrawArguments;
	};

	std::uint32_t AlignToUAVCounterOffset(std::uint32_t size)
	{
		return Math::AlignUp(size, D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);
	}
} // namespace unnamed

namespace render
{
	ShadowClearPass::ShadowClearPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<ShadowClearPassData>("Shadow Clear Pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
	}

	void ShadowClearPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.ShadowMaps = builder.CreateResourceVirtual("shadow_maps");
	}

	void ShadowClearPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("Shadow pass command list - clear");

		std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");

		PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Shadow Pass | Clear");
		for (uint32_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
		{
			if (std::shared_ptr<scene::Light> light = lightEntities[lightIndex]->GetComponentAs<scene::Light>("Light"); light->CastShadows)
			{
				PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, lightEntities[lightIndex]->GetName().c_str());

				std::shared_ptr<dx12::Resource> shadowMap = context.GetTextureManager().GetTexture(light->ShadowMapHandle);

				// Transition resources
				commandList.TransitionBarrier({ shadowMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE });

				DescriptorHandle depthHandle = context.GetStaticResourceHandle(shadowMap->GetAsDSV());
				commandList.ClearDSV(depthHandle.CpuHandle, D3D12_CLEAR_FLAG_DEPTH);

				commandList.TransitionBarrier({ shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE });


				PIXEndEvent(commandList.GetDXCommandList().Get());
			}
		}
		PIXEndEvent(commandList.GetDXCommandList().Get());

		commandList.Close();
	}
} // namespace render
