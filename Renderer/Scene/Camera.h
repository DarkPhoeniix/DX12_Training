#pragma once

#include "Scene/Viewport.h"
#include "Volumes/FrustumVolume.h"

// TODO: refactor Camera

class Camera
{
public:
	// By default, the camera starts out with its basis vectors 
	// aligned with the world space axes, and its origin positioned
	// at the world space origin.
	Camera();

	const DirectX::XMMATRIX& View() const;
	const DirectX::XMMATRIX& Projection() const;
	const DirectX::XMMATRIX& ViewProjection() const;

	const DirectX::XMVECTOR& Right() const;
	const DirectX::XMVECTOR& Up() const;
	const DirectX::XMVECTOR& Look() const;

	const FrustumVolume& GetViewFrustum() const;

	DirectX::XMVECTOR& Position();

	void LookAt(DirectX::XMVECTOR& pos, DirectX::XMVECTOR& target, DirectX::XMVECTOR& up);

	void SetViewport(const Viewport& viewport);
	Viewport GetViewport() const;

	CD3DX12_VIEWPORT GetDXViewport() const;
	CD3DX12_RECT GetDXScissorRectangle() const;

	void SetLens(float fov, float nearZ, float farZ);

	void SetFOV(float fov);
	float GetFOV() const;

	void SetNearZ(float nearZ);
	float GetNearZ() const;

	void SetFarZ(float farZ);
	float GetFarZ() const;

	void SetSpeed(float s);
	float GetSpeed() const;

	void Update(DirectX::XMVECTOR direction);
	void Update(int pitch, int yaw);

protected:
	// Constructs the view matrix based on the camera's basis
	// vectors and origin, relative to the world space
	void _buildView();
	void _buildProjection();
	void _updateFrustum();

protected:
	// Save camera related matrices.
	DirectX::XMMATRIX _view;
	DirectX::XMMATRIX _projection;
	DirectX::XMMATRIX _viewProjection;

	// Relative to world space.
	DirectX::XMVECTOR _position;
	DirectX::XMVECTOR _right;
	DirectX::XMVECTOR _up;
	DirectX::XMVECTOR _target;
	DirectX::XMVECTOR _look;

	// Frustum
	FrustumVolume _frustum;

	Viewport _viewport;

	// Lens params
	float _fov;
	float _nearZ;
	float _farZ;

	// Camera speed.
	float _speed;

	int _prevX;
	int _prevY;
};
