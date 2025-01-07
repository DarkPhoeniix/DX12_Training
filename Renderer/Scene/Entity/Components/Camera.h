#pragma once

#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/IComponent.h"
#include "Scene/Volumes/FrustumVolume.h"

// TODO: refactor Camera

namespace SceneLayer
{
	class Viewport
	{
	public:
		Viewport();
		Viewport(const DirectX::XMUINT2& size);
		Viewport(const CD3DX12_VIEWPORT& DXViewport);
		~Viewport() = default;

		CD3DX12_VIEWPORT GetDXViewport() const;
		float GetAspectRatio() const;

		void SetSize(const DirectX::XMUINT2& size);
		DirectX::XMUINT2 GetSize() const;

		void SetDepth(const DirectX::XMFLOAT2& depth);
		DirectX::XMFLOAT2 GetDepth() const;

		void SetScissorRectangle(const CD3DX12_RECT& rect);
		CD3DX12_RECT GetScissorRectangle() const;

	private:
		CD3DX12_VIEWPORT _viewport;
		CD3DX12_RECT _scissorRectangle;
		float _aspectRatio;
	};

	class Camera : public IComponent
	{
	public:
		Camera();

		const DirectX::XMMATRIX& View() const;
		const DirectX::XMMATRIX& Projection() const;
		const DirectX::XMMATRIX& ViewProjection() const;

		const DirectX::XMVECTOR& Right() const;
		const DirectX::XMVECTOR& Up() const;
		const DirectX::XMVECTOR& Look() const;
		const DirectX::XMVECTOR& Poisition() const;

		const FrustumVolume& GetViewFrustum() const;

		void LookAt(DirectX::XMVECTOR& pos, DirectX::XMVECTOR& target, DirectX::XMVECTOR& up);

		void SetViewport(const Viewport& viewport);
		Viewport& GetViewport();

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

		void Update();
		void Update(DirectX::XMVECTOR direction);
		void Update(int pitch, int yaw);

	private:
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
