#pragma once

#include "Heap.h"

namespace dx12
{
	class CommandList;
}

using TextureHandle = std::uint32_t;
constexpr TextureHandle InvalidTextureHandle = TextureHandle(-1);

class TextureManager
{
public:
	TextureManager(TextureManager&& other) noexcept = default;
	TextureManager(const TextureManager& other) = delete;

    TextureManager& operator=(const TextureManager& other) = delete;
    TextureManager& operator=(TextureManager&& other) noexcept = default;

	static void Create();
	static void Destroy();
	static TextureManager& Get();

	[[nodiscard]] TextureHandle EnqueueTexture(const std::string& filepath);
	void UploadTextures(dx12::CommandList& commandList);

    void ClearIntermediates();
    void Clear();

    [[nodiscard]] TextureHandle AddTexture(std::shared_ptr<dx12::Resource> texture);
    [[nodiscard]] std::shared_ptr<dx12::Resource> GetTexture(TextureHandle handle) const;

private:
	TextureManager();

	std::unordered_map<TextureHandle, std::shared_ptr<dx12::Resource>> _handleToTexture;
	TextureHandle _nextTextureHandle;

    std::unordered_map<std::string, TextureHandle> _uploadQueue; // use set to remove duplicates

	dx12::Heap _texturesHeap;
	std::unordered_map<TextureHandle, std::shared_ptr<dx12::Resource>> _intermediateResources;

	static std::unique_ptr<TextureManager> _instance;
};
