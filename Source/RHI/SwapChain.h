#pragma once

namespace rhi
{
    class Texture;
    struct ScissorRect;

    // Number of back buffers in the swap chain (triple buffering).
    constexpr std::uint32_t BACK_BUFFER_COUNT = 3;

    // SwapChain is an abstract interface representing a swap chain, which manages the presentation of rendered frames to the screen
    class SwapChain
    {
    public:
        SwapChain() = default;
        SwapChain(const SwapChain&) = delete;
        SwapChain(SwapChain&&) noexcept = default;
        virtual ~SwapChain() = default;

        SwapChain& operator=(const SwapChain&) = delete;
        SwapChain& operator=(SwapChain&&) noexcept = default;

        // Retrieves the buffer texture at the specified index in the swap chain, allowing the application to access the individual back buffers
        virtual std::shared_ptr<Texture> GetBuffer(std::uint32_t index) = 0;
        // Retrieves the current back buffer texture, which is the render target for the next frame to be presented
        virtual std::shared_ptr<Texture> GetBackBuffer() = 0;

        // Presents the rendered frame to the screen, typically by swapping the back buffer with the front buffer in the swap chain. 
        // The return value indicates current backbuffer index after the present operation
        virtual std::uint32_t Present() = 0;

        // Handles resizing of the swap chain's back buffer, allowing the application to adjust its resources and state accordingly when the window size changes
        virtual void OnResize(std::uint32_t width, std::uint32_t height) = 0;

        // Retrieves the coordinates of the desktop area associated with the swap chain
        virtual rhi::ScissorRect GetDesktopCoordinates() = 0;

        // Retrieves the native swap chain object, allowing the application to access the underlying API-specific swap chain
        virtual void* GetNative() const = 0;
    };
} // namespace rhi
