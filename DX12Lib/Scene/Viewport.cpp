#include "stdafx.h"

#include "Viewport.h"

Viewport::Viewport()
    : _viewport(0.0f, 0.0f, 0.0f, 0.0f)
    , _scissorRectangle(0, 0, LONG_MAX, LONG_MAX)
    , _aspectRatio(0.0f)
{
}

Viewport::Viewport(const DirectX::XMFLOAT2& size)
    : _viewport(0.0f, 0.0f, size.x, size.y)
    , _scissorRectangle(0, 0, LONG_MAX, LONG_MAX)
    , _aspectRatio(size.x / size.y)
{
}

Viewport::Viewport(const CD3DX12_VIEWPORT& DXViewport)
    : _viewport(DXViewport)
    , _scissorRectangle(0, 0, LONG_MAX, LONG_MAX)
    , _aspectRatio(DXViewport.Width / DXViewport.Height)
{
}

CD3DX12_VIEWPORT Viewport::GetDXViewport() const
{
    return _viewport;
}

float Viewport::GetAspectRatio() const
{
    return _aspectRatio;
}

void Viewport::SetSize(const DirectX::XMFLOAT2& size)
{
    _viewport.Width = size.x;
    _viewport.Height = size.y;
}

DirectX::XMFLOAT2 Viewport::GetSize() const
{
    return { _viewport.Width, _viewport.Height };
}

void Viewport::SetDepth(const DirectX::XMFLOAT2& depth)
{
    _viewport.MinDepth = depth.x;
    _viewport.MaxDepth = depth.y;
}

DirectX::XMFLOAT2 Viewport::GetDepth() const
{
    return { _viewport.MinDepth, _viewport.MaxDepth };
}

void Viewport::SetScissorRectangle(const CD3DX12_RECT& rect)
{
    _scissorRectangle = rect;
}

CD3DX12_RECT Viewport::GetScissorRectangle() const
{
    return _scissorRectangle;
}
