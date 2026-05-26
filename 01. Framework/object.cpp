#include "object.h"

GameObject::GameObject() : m_right{1.f, 0.f, 0.f}, m_up{0.f, 1.f, 0.f}, m_front{0.f, 0.f, 1.f}
{
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
}

// 업데이트 함수
void GameObject::Update(FLOAT timeElapsed)
{
	if (m_collider)
	{
		m_collider->Update(m_worldMatrix);
	}
}

void GameObject::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	XMFLOAT4X4 worldMatrix;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_worldMatrix)));
	commandList->SetGraphicsRoot32BitConstants(0, 16, &worldMatrix, 0);
}

// 렌더링 함수
void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	UpdateShaderVariable(commandList);
	m_mesh->Render(commandList);
}

// 멤버 함수
void GameObject::Transform(XMFLOAT3 shift)
{
	SetPosition(Utiles::Vector3::Add(GetPosition(), shift));
}

void GameObject::Rotate(FLOAT pitch, FLOAT yaw, FLOAT roll)
{
	XMMATRIX rotate{ XMMatrixRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll)) };
	XMStoreFloat4x4(&m_worldMatrix, rotate * XMLoadFloat4x4(&m_worldMatrix));

	XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), rotate));
	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), rotate));
	XMStoreFloat3(&m_front, XMVector3TransformNormal(XMLoadFloat3(&m_front), rotate));
}


RotatingObject::RotatingObject() : GameObject(), m_rotatingSpeed{ Utiles::Random::GetFloat(10.f, 50.f) }
{
}

void RotatingObject::Update(FLOAT timeElapsed)
{
	Rotate(0.f, m_rotatingSpeed * timeElapsed, 0.f);

	GameObject::Update(timeElapsed);
}

void RotatingObject::OnCollisionEnter(const shared_ptr<Collider>& other)
{
	m_rotatingSpeed = -m_rotatingSpeed;
}