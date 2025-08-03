#pragma once

#include "Heap.h"
#include "Core/ResourceTable.h"

#include <set>

namespace dx12
{
	class CommandList;
	class Texture;
}

namespace DirectX
{
	class ScratchImage;
}

using TextureHandle = std::uint32_t;
constexpr TextureHandle InvalidTextureHandle = TextureHandle(-1);

class TextureManager
{
public:
	TextureManager(ResourceTable& resourceTable);
	TextureManager(const TextureManager& other) = delete;
	TextureManager(TextureManager&& other) noexcept = default;

	TextureManager& operator=(const TextureManager& other) = delete;
	TextureManager& operator=(TextureManager&& other) noexcept = default;

	[[nodiscard]] TextureHandle EnqueueTexture(const std::string& filepath);
	void UploadTextures(dx12::CommandList& commandList);

	void ClearIntermediates();
	void Clear();

	[[nodiscard]] TextureHandle AddTexture(std::shared_ptr<dx12::Resource> texture, dx12::ResourceViewType viewType);
	[[nodiscard]] std::shared_ptr<dx12::Resource> GetTexture(TextureHandle handle) const;

private:
	std::unordered_map<TextureHandle, std::shared_ptr<dx12::Resource>> _handleToTexture;
	TextureHandle _nextTextureHandle;

	std::unordered_map<std::string, TextureHandle> _uploadQueue; // use set to remove duplicates

	ResourceTable& _resourceTable;
	dx12::Heap _texturesHeap;
	std::unordered_map<TextureHandle, std::shared_ptr<dx12::Resource>> _intermediateResources;
};
