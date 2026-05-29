#include "collider.h"

bool BoxCollider::Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outDist) const
{
	XMVECTOR O = XMLoadFloat3(&origin);
	XMVECTOR D = XMVector3Normalize(XMLoadFloat3(&direction)); 

	return m_worldOBB.Intersects(O, D, outDist);
}

void CapsuleCollider::Update(const XMFLOAT4X4& worldMatrix)
{
	// 캡슐의 정중앙 좌표 추출
	m_worldCenter = XMFLOAT3(worldMatrix._41, worldMatrix._42, worldMatrix._43);

	// Y축 위쪽(Up) 벡터 추출
	XMVECTOR up = XMVectorSet(worldMatrix._21, worldMatrix._22, worldMatrix._23, 0.0f);
	up = XMVector3Normalize(up);

	// 위쪽(A) 구체 위치와 아래쪽(B) 구체 위치 계산
	XMVECTOR pos = XMLoadFloat3(&m_worldCenter);
	XMVECTOR halfHeight = XMVectorScale(up, m_height * 0.5f);

	XMStoreFloat3(&m_worldPointA, XMVectorAdd(pos, halfHeight));      // 머리(Top)
	XMStoreFloat3(&m_worldPointB, XMVectorSubtract(pos, halfHeight)); // 발(Bottom)
	XMFLOAT3 minPoint{
		min(m_worldPointA.x, m_worldPointB.x) - m_radius,
		min(m_worldPointA.y, m_worldPointB.y) - m_radius,
		min(m_worldPointA.z, m_worldPointB.z) - m_radius
	};
	XMFLOAT3 maxPoint{
		max(m_worldPointA.x, m_worldPointB.x) + m_radius,
		max(m_worldPointA.y, m_worldPointB.y) + m_radius,
		max(m_worldPointA.z, m_worldPointB.z) + m_radius
	};

	BoundingBox::CreateFromPoints(m_worldAABB, XMLoadFloat3(&minPoint), XMLoadFloat3(&maxPoint));
}

bool CapsuleCollider::Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outDist) const
{
	// 플레이어 레이캐스트용 임시 처리 (Box 광선 검사와 동일한 원통 근사식 생략)
	return false;
}