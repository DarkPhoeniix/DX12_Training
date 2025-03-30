#include "RendererPCH.h"

#include "Camera.h"

using namespace DirectX;

namespace
{
	constexpr float CAMERA_ROTATION_SPEED = 0.1f;
} // namespace unnamed

namespace scene
{
	Viewport::Viewport()
		: _viewport(0.0f, 0.0f, 0.0f, 0.0f)
		, _scissorRectangle(0, 0, LONG_MAX, LONG_MAX)
		, _aspectRatio(0.0f)
	{
	}

	Viewport::Viewport(const DirectX::XMUINT2& size)
		: _viewport(0.0f, 0.0f, (FLOAT)size.x, (FLOAT)size.y)
		, _scissorRectangle(0, 0, LONG_MAX, LONG_MAX)
		, _aspectRatio(size.x / (FLOAT)size.y)
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

	void Viewport::SetSize(const DirectX::XMUINT2& size)
	{
		_viewport.Width = (FLOAT)size.x;
		_viewport.Height = (FLOAT)size.y;
		_aspectRatio = (size.x / (FLOAT)size.y);
	}

	DirectX::XMUINT2 Viewport::GetSize() const
	{
		return { (UINT)_viewport.Width, (UINT)_viewport.Height };
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

	CD3DX12_RECT& Viewport::GetScissorRectangle()
	{
		return _scissorRectangle;
	}

	const CD3DX12_RECT& Viewport::GetScissorRectangle() const
	{
		return _scissorRectangle;
	}
	
	Camera::Camera()
		: IComponent("Camera")
		, _view(XMMatrixIdentity())
		, _projection(XMMatrixIdentity())
		, _viewProjection(XMMatrixIdentity())
		, _position(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f))
		, _up(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
		, _viewport({ 1, 1 })
		, Speed(100.0f)
	{
		_UpdateFrustum();
	}

	void Camera::Update()
	{
		_BuildView();
		_BuildProjection();
	}

	void Camera::Update(XMVECTOR direction)
	{
		XMMATRIX movement = XMMatrixTranslationFromVector(direction * Speed);

		_position = XMVector4Transform(_position, movement);
		_target = XMVector4Transform(_target, movement);

		// Rebuild the view matrix to reflect changes.
		_BuildView();
	}

	void Camera::Update(int xDelta, int yDelta)
	{
		float xRotationDelta = XMConvertToRadians((yDelta) * CAMERA_ROTATION_SPEED);
		float yRotationDelta = XMConvertToRadians((xDelta) * CAMERA_ROTATION_SPEED);

		XMMATRIX position = XMMatrixTranslationFromVector(_position);
		XMMATRIX invPosition = XMMatrixTranslationFromVector(XMVectorNegate(_position));
		XMMATRIX rotateAroundUp = XMMatrixRotationAxis(_up, yRotationDelta);
		XMMATRIX rotateAroundRight = XMMatrixRotationAxis(Right(), xRotationDelta);

		XMMATRIX positionTransform = invPosition * rotateAroundUp * rotateAroundRight * position;

		_target = XMVector4Transform(_target, positionTransform);
		//_up = XMVector3Transform(_up, positionTransform);

		// Rebuild the view matrix to reflect changes
		_BuildView();
	}

	const XMMATRIX& Camera::View() const
	{
		return _view;
	}

	const XMMATRIX& Camera::Projection() const
	{
		return _projection;
	}

	const XMMATRIX& Camera::ViewProjection() const
	{
		return _viewProjection;
	}

	XMVECTOR Camera::Right() const
	{
		return XMMatrixTranspose(_view).r[0];
	}

	XMVECTOR Camera::Up() const
	{
		return XMMatrixTranspose(_view).r[1];
	}

	XMVECTOR Camera::Look() const
	{
		return XMMatrixTranspose(_view).r[2];
	}

	const XMVECTOR& Camera::Position() const
	{
		return _position;
	}

	XMVECTOR& Camera::Position()
	{
		return _position;
	}

	const XMVECTOR& Camera::Target() const
	{
		return _target;
	}

	XMVECTOR& Camera::Target()
	{
		return _target;
	}

	void Camera::LookAt(const XMVECTOR& pos, const XMVECTOR& target, const XMVECTOR& up)
	{
		_position = pos;
		_target = target;
		_up = XMVector3Normalize(up);

		_BuildView();
	}

	const FrustumVolume& Camera::GetViewFrustum() const
	{
		return _frustum;
	}

	void Camera::SetViewport(const Viewport& viewport)
	{
		_viewport = viewport;

		_BuildProjection();
	}

	Viewport& Camera::GetViewport()
	{
		return _viewport;
	}

	void Camera::SetLens(float fov, float nearZ, float farZ)
	{
		FoV = fov;
		NearZ = nearZ;
		FarZ = farZ;

		_BuildProjection();
	}

	void Camera::_BuildView()
	{
		_view = XMMatrixLookAtLH(_position, _target, _up);

		_UpdateFrustum();
	}

	void Camera::_BuildProjection()
	{
		_projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(FoV), _viewport.GetAspectRatio(), NearZ, FarZ);
		_UpdateFrustum();
	}

	void Camera::_UpdateFrustum()
	{
		_frustum.transform = _view;

		_viewProjection = _view * _projection;
		_frustum.BuildFromProjMatrix(_viewProjection);
	}
} // namespace scene
