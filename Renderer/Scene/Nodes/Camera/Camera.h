#pragma once

#include "Scene/Nodes/Camera/Viewport.h"
#include "Scene/Nodes/ISceneNode.h"
#include "Scene/Volumes/FrustumVolume.h"

// TODO: refactor Camera

namespace SceneLayer
{
	class Scene;

	class Camera : public ISceneNode
	{
	public:
		Camera();

		const DirectX::XMMATRIX& View() const;
		const DirectX::XMMATRIX& Projection() const;
		const DirectX::XMMATRIX& ViewProjection() const;

		const DirectX::XMVECTOR& Right() const;
		const DirectX::XMVECTOR& Up() const;
		const DirectX::XMVECTOR& Look() const;

		const FrustumVolume& GetViewFrustum() const;

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

	private:
		using Base = ISceneNode;

		// Constructs the view matrix based on the camera's basis
		// vectors and origin, relative to the world space
		void _BuildView();
		void _BuildProjection();
		void _UpdateFrustum();

		// Save camera related matrices.
		DirectX::XMMATRIX _view;
		DirectX::XMMATRIX _projection;
		DirectX::XMMATRIX _viewProjection;

		// Relative to world space.
		DirectX::XMVECTOR _position;
		DirectX::XMVECTOR _right;
		DirectX::XMVECTOR _up;
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
} // namespace SceneLayer
