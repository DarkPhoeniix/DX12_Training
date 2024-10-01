#pragma once

namespace SceneLayer
{
    class Viewport
    {
    public:
        Viewport();
        Viewport(const DirectX::XMFLOAT2& size);
        Viewport(const CD3DX12_VIEWPORT& DXViewport);
        ~Viewport() = default;

        CD3DX12_VIEWPORT GetDXViewport() const;
        float GetAspectRatio() const;

        void SetSize(const DirectX::XMFLOAT2& size);
        DirectX::XMFLOAT2 GetSize() const;

        void SetDepth(const DirectX::XMFLOAT2& depth);
        DirectX::XMFLOAT2 GetDepth() const;

        void SetScissorRectangle(const CD3DX12_RECT& rect);
        CD3DX12_RECT GetScissorRectangle() const;

    private:
        CD3DX12_VIEWPORT _viewport;
        CD3DX12_RECT _scissorRectangle;
        float _aspectRatio;
    };
} // namespace SceneLayer
