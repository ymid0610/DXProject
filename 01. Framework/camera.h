#pragma once
#include "stdafx.h"

class Camera
{
public:
	Camera();
	~Camera() = default;

	virtual void Update(FLOAT timeElapsed) = 0;
	virtual void UpdateEye(XMFLOAT3 position) = 0;
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList);

	virtual void RotatePitch(FLOAT radian) = 0;
	virtual void RotateYaw(FLOAT radian) = 0;

	void SetLens(FLOAT fovy, FLOAT aspect, FLOAT minZ, FLOAT maxZ);

	XMFLOAT3 GetEye() const;
	XMFLOAT3 GetU() const;
	XMFLOAT3 GetV() const;
	XMFLOAT3 GetN() const;

protected:
	void UpdateBasis();

protected:
	XMFLOAT4X4 m_viewMatrix;
	XMFLOAT4X4 m_projectionMatrix;

	XMFLOAT3 m_eye;
	XMFLOAT3 m_at;
	XMFLOAT3 m_up;

	XMFLOAT3 m_u;
	XMFLOAT3 m_v;
	XMFLOAT3 m_n;
};

class ThirdPersonCamera : public Camera
{
public:
	ThirdPersonCamera();
	~ThirdPersonCamera() = default;

	void Update(FLOAT timeElapsed) override;
	void UpdateEye(XMFLOAT3 position) override;

	void RotatePitch(FLOAT radian) override;
	void RotateYaw(FLOAT radian) override;
private:
	FLOAT m_radius;
	FLOAT m_phi;
	FLOAT m_theta;
};

class SpringArmCamera : public Camera
{
public:
	SpringArmCamera();
	~SpringArmCamera() = default;

	void Update(FLOAT timeElapsed) override;
	void UpdateEye(XMFLOAT3 position) override;

	void RotatePitch(FLOAT radian) override;
	void RotateYaw(FLOAT radian) override;

	void SetArmLength(FLOAT length);
	void AddArmLength(FLOAT length);
	FLOAT GetArmLength() const { return m_targetArmLength; }

private:
	FLOAT m_currentArmLength; // 현재 적용중인 길이 (보간용)
	FLOAT m_targetArmLength;  // 목표 길이

	FLOAT m_phi;   // Pitch (위/아래)
	FLOAT m_theta; // Yaw (좌/우)

	XMFLOAT3 m_offset;  // 캐릭터의 발바닥이 아닌 상체(머리/어깨)를 찍기 위한 오프셋
};
