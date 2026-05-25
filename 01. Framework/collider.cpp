#include "collider.h"

bool BoxCollider::Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outDist) const
{
	XMVECTOR O = XMLoadFloat3(&origin);
	XMVECTOR D = XMVector3Normalize(XMLoadFloat3(&direction)); 

	return m_worldOBB.Intersects(O, D, outDist);
}