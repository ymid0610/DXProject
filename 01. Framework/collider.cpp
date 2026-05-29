#include "collider.h"

using namespace Utiles::Physics;

namespace
{
    bool IntersectRaySphere(FXMVECTOR origin, FXMVECTOR direction, FXMVECTOR center, float radius, float& outDist)
    {
        XMVECTOR toOrigin = XMVectorSubtract(origin, center);
        float b = VectorDot(toOrigin, direction);
        float c = VectorDot(toOrigin, toOrigin) - radius * radius;

        if (c > 0.0f && b > 0.0f) return false;

        float discriminant = b * b - c;
        if (discriminant < 0.0f) return false;

        outDist = -b - sqrtf(discriminant);
        if (outDist < 0.0f) outDist = 0.0f;
        return true;
    }
}

bool BoxCollider::Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outDist) const
{
    XMVECTOR rayOrigin = XMLoadFloat3(&origin);
    XMVECTOR rayDirection = SafeNormalize(XMLoadFloat3(&direction), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));

    return m_worldOBB.Intersects(rayOrigin, rayDirection, outDist);
}

void CapsuleCollider::Update(const XMFLOAT4X4& worldMatrix)
{
    m_worldCenter = XMFLOAT3(worldMatrix._41, worldMatrix._42, worldMatrix._43);

    XMVECTOR up = XMVectorSet(worldMatrix._21, worldMatrix._22, worldMatrix._23, 0.0f);
    up = SafeNormalize(up, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

    XMVECTOR pos = XMLoadFloat3(&m_worldCenter);
    XMVECTOR halfHeight = XMVectorScale(up, m_height * 0.5f);

    XMStoreFloat3(&m_worldPointA, XMVectorAdd(pos, halfHeight));
    XMStoreFloat3(&m_worldPointB, XMVectorSubtract(pos, halfHeight));

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
    XMVECTOR rayOrigin = XMLoadFloat3(&origin);
    XMVECTOR rayDirection = SafeNormalize(XMLoadFloat3(&direction), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
    XMVECTOR segmentA = XMLoadFloat3(&m_worldPointA);
    XMVECTOR segmentB = XMLoadFloat3(&m_worldPointB);
    XMVECTOR capsuleAxis = XMVectorSubtract(segmentB, segmentA);

    XMVECTOR originClosest = ClosestPointOnSegment(rayOrigin, segmentA, segmentB);
    if (VectorLengthSq(XMVectorSubtract(rayOrigin, originClosest)) <= m_radius * m_radius)
    {
        outDist = 0.0f;
        return true;
    }

    float nearestHit = FLT_MAX;
    bool hit = false;

    float sphereHit = 0.0f;
    if (IntersectRaySphere(rayOrigin, rayDirection, segmentA, m_radius, sphereHit))
    {
        nearestHit = min(nearestHit, sphereHit);
        hit = true;
    }
    if (IntersectRaySphere(rayOrigin, rayDirection, segmentB, m_radius, sphereHit))
    {
        nearestHit = min(nearestHit, sphereHit);
        hit = true;
    }

    float axisLengthSq = VectorLengthSq(capsuleAxis);
    if (axisLengthSq > Epsilon)
    {
        XMVECTOR originToSegment = XMVectorSubtract(rayOrigin, segmentA);
        float axisDotDirection = VectorDot(capsuleAxis, rayDirection);
        float axisDotOrigin = VectorDot(capsuleAxis, originToSegment);
        float directionDotOrigin = VectorDot(rayDirection, originToSegment);
        float originLengthSq = VectorDot(originToSegment, originToSegment);

        float a = axisLengthSq - axisDotDirection * axisDotDirection;
        float b = axisLengthSq * directionDotOrigin - axisDotOrigin * axisDotDirection;
        float c = axisLengthSq * originLengthSq - axisDotOrigin * axisDotOrigin - m_radius * m_radius * axisLengthSq;

        if (fabsf(a) > Epsilon)
        {
            float discriminant = b * b - a * c;
            if (discriminant >= 0.0f)
            {
                float cylinderHit = (-b - sqrtf(discriminant)) / a;
                float capsuleAxisProjection = axisDotOrigin + cylinderHit * axisDotDirection;

                if (cylinderHit >= 0.0f && capsuleAxisProjection >= 0.0f && capsuleAxisProjection <= axisLengthSq)
                {
                    nearestHit = min(nearestHit, cylinderHit);
                    hit = true;
                }
            }
        }
    }

    if (!hit) return false;

    outDist = nearestHit;
    return true;
}