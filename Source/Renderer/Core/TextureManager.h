#pragma once

#include "RHI/Heap.h"

#include <shared_mutex>

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

	static void Create(rhi::Device* device);
	static void Destroy();
	static TextureManager& Get();

	[[nodiscard]] TextureHandle EnqueueTexture(const std::string& filepath);
	void UploadTextures(rhi::CommandList* commandList);

    bool AreTexturesPendingUpload() const;

    void ClearIntermediates();
    void Clear();

    [[nodiscard]] TextureHandle AddTexture(std::shared_ptr<rhi::Texture> texture);
    [[nodiscard]] std::shared_ptr<rhi::Texture> GetTexture(TextureHandle handle) const;

private:
	TextureManager(rhi::Device* device);

	struct UploadInfo
	{
		TextureHandle Handle;
		std::string Name;
		rhi::TextureDescription Description;
	};

	std::unordered_map<TextureHandle, std::shared_ptr<rhi::Texture>> _handleToTexture;
	TextureHandle _nextTextureHandle;

    std::unordered_map<std::string, UploadInfo> _uploadQueue; // TODO: use set to remove duplicates

    std::unique_ptr<rhi::Heap> _texturesHeap;	// TODO: this heap should be bigger and reused for multiple texture uploads
	std::unordered_map<TextureHandle, std::shared_ptr<rhi::Buffer>> _intermediateResources;

	rhi::Device* _device;

	mutable std::mutex _queueMutex;
	mutable std::shared_mutex _textureMutex;

	static std::unique_ptr<TextureManager> _instance;
};
