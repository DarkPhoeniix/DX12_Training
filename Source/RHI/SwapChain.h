#pragma once

namespace rhi
{
    // Number of back buffers in the swap chain (triple buffering).
    constexpr std::uint32_t BACK_BUFFER_COUNT = 3;


    class Texture;

    class SwapChain
    {
    public:
        SwapChain() = default;
        SwapChain(const SwapChain&) = delete;
        SwapChain(SwapChain&&) noexcept = default;
        virtual ~SwapChain() = default;

        SwapChain& operator=(const SwapChain&) = delete;
        SwapChain& operator=(SwapChain&&) noexcept = default;

        virtual void Init(HWND windowHandle, std::uint32_t width, std::uint32_t height, bool vSync = false) = 0;

        virtual std::shared_ptr<Texture> GetBuffer(std::uint32_t index) = 0;
        virtual std::shared_ptr<Texture> GetBackBuffer() = 0;

        virtual void UpdateRenderTargetViews() = 0;
        virtual std::uint32_t Present() = 0;

        virtual void OnResize(std::uint32_t width, std::uint32_t height) = 0;

        virtual void* GetNative() const = 0;
    };
} // namespace rhi
