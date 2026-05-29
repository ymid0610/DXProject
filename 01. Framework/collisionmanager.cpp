#include "collisionmanager.h"
#include "object.h"

using namespace Utiles::Physics;

namespace
{
    struct CollisionCandidate
    {
        shared_ptr<Collider> colliderA;
        shared_ptr<Collider> colliderB;
        shared_ptr<GameObject> ownerA;
        shared_ptr<GameObject> ownerB;
    };

    struct ColliderProxy
    {
        shared_ptr<Collider> collider;
        shared_ptr<GameObject> owner;
    };

    bool IsDynamicObject(const shared_ptr<GameObject>& object)
    {
        return object && object->GetInverseMass() > 0.0f;
    }

    bool BroadPhaseOverlap(const shared_ptr<Collider>& colA, const shared_ptr<Collider>& colB)
    {
        BoundingBox aabbA = colA->GetWorldAABB();
        BoundingBox aabbB = colB->GetWorldAABB();
        return aabbA.Intersects(aabbB);
    }

    void AddCandidateIfBroadPhaseOverlaps(vector<CollisionCandidate>& candidates,
        const ColliderProxy& proxyA, const ColliderProxy& proxyB)
    {
        if (!BroadPhaseOverlap(proxyA.collider, proxyB.collider)) return;
        candidates.push_back({ proxyA.collider, proxyB.collider, proxyA.owner, proxyB.owner });
    }

    vector<CollisionCandidate> BuildCollisionCandidates(const vector<shared_ptr<Collider>>& colliders)
    {
        vector<ColliderProxy> dynamicColliders;
        vector<ColliderProxy> staticColliders;
        dynamicColliders.reserve(colliders.size());
        staticColliders.reserve(colliders.size());

        for (const auto& collider : colliders)
        {
            if (!collider) continue;

            auto owner = collider->m_owner.lock();
            if (!owner) continue;

            ColliderProxy proxy{ collider, owner };
            if (IsDynamicObject(owner)) dynamicColliders.push_back(proxy);
            else staticColliders.push_back(proxy);
        }

        vector<CollisionCandidate> candidates;
        candidates.reserve(dynamicColliders.size() * 2);

        for (size_t i = 0; i < dynamicColliders.size(); ++i)
        {
            const auto& dynamicA = dynamicColliders[i];

            for (size_t j = i + 1; j < dynamicColliders.size(); ++j)
            {
                AddCandidateIfBroadPhaseOverlaps(candidates, dynamicA, dynamicColliders[j]);
            }

            for (const auto& staticCollider : staticColliders)
            {
                AddCandidateIfBroadPhaseOverlaps(candidates, dynamicA, staticCollider);
            }
        }

        return candidates;
    }

    bool ComputeOBBOBBContact(const BoundingOrientedBox& obbA, const BoundingOrientedBox& obbB, ContactInfo& outContact)
    {
        XMVECTOR axesA[3]{};
        XMVECTOR axesB[3]{};
        GetOBBAxes(obbA, axesA);
        GetOBBAxes(obbB, axesB);

        XMVECTOR centerA = XMLoadFloat3(&obbA.Center);
        XMVECTOR centerB = XMLoadFloat3(&obbB.Center);
        XMVECTOR centerDelta = XMVectorSubtract(centerB, centerA);

        float minOverlap = FLT_MAX;
        XMVECTOR minAxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        for (int i = 0; i < 3; ++i)
        {
            if (!TestOBBAxis(obbA, axesA, obbB, axesB, centerDelta, axesA[i], minOverlap, minAxis)) return false;
            if (!TestOBBAxis(obbA, axesA, obbB, axesB, centerDelta, axesB[i], minOverlap, minAxis)) return false;
        }

        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                XMVECTOR crossAxis = XMVector3Cross(axesA[i], axesB[j]);
                if (!TestOBBAxis(obbA, axesA, obbB, axesB, centerDelta, crossAxis, minOverlap, minAxis)) return false;
            }
        }

        outContact.normal = ToFloat3(SafeNormalize(minAxis, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
        outContact.penetration = max(minOverlap, 0.0f);
        return true;
    }

    bool ComputeCapsuleOBBContact(const CapsuleCollider& capsule, const BoundingOrientedBox& obb, ContactInfo& outContact)
    {
        XMFLOAT3 pointA = capsule.GetPointA();
        XMFLOAT3 pointB = capsule.GetPointB();
        XMFLOAT3 capsuleCenterFloat = capsule.GetCenter();

        XMVECTOR segmentA = XMLoadFloat3(&pointA);
        XMVECTOR segmentB = XMLoadFloat3(&pointB);
        XMVECTOR capsuleCenter = XMLoadFloat3(&capsuleCenterFloat);
        XMVECTOR boxCenter = XMLoadFloat3(&obb.Center);

        XMVECTOR axes[3]{};
        GetOBBAxes(obb, axes);

        XMVECTOR segmentPoint = ClosestPointOnSegment(boxCenter, segmentA, segmentB);
        XMVECTOR boxPoint = ClosestPointOnOBB(segmentPoint, obb, axes);

        for (int i = 0; i < 4; ++i)
        {
            segmentPoint = ClosestPointOnSegment(boxPoint, segmentA, segmentB);
            boxPoint = ClosestPointOnOBB(segmentPoint, obb, axes);
        }

        XMVECTOR capToBox = XMVectorSubtract(boxPoint, segmentPoint);
        float distanceSq = VectorLengthSq(capToBox);
        float radius = capsule.GetRadius();
        float radiusSq = radius * radius;

        if (distanceSq > radiusSq) return false;

        XMVECTOR normal{};
        float penetration = 0.0f;

        if (distanceSq > Epsilon)
        {
            float distance = sqrtf(distanceSq);
            normal = XMVectorScale(capToBox, 1.0f / distance);
            penetration = radius - distance;
        }
        else
        {
            XMFLOAT3 localPoint{};
            XMFLOAT3 localCenter{};
            XMStoreFloat3(&localPoint, GetOBBLocalCoordinates(segmentPoint, obb, axes));
            XMStoreFloat3(&localCenter, GetOBBLocalCoordinates(capsuleCenter, obb, axes));

            float coords[3] = { localPoint.x, localPoint.y, localPoint.z };
            float centerCoords[3] = { localCenter.x, localCenter.y, localCenter.z };
            float extents[3] = { obb.Extents.x, obb.Extents.y, obb.Extents.z };

            int bestAxis = 0;
            float bestDistance = FLT_MAX;
            float bestSign = -1.0f;

            for (int i = 0; i < 3; ++i)
            {
                float faceDistance = max(extents[i] - fabsf(coords[i]), 0.0f);
                if (faceDistance < bestDistance)
                {
                    bestAxis = i;
                    bestDistance = faceDistance;

                    float reference = fabsf(centerCoords[i]) > Epsilon ? centerCoords[i] : coords[i];
                    bestSign = reference >= 0.0f ? -1.0f : 1.0f;
                }
            }

            normal = XMVectorScale(axes[bestAxis], bestSign);
            penetration = radius + bestDistance;
        }

        outContact.normal = ToFloat3(SafeNormalize(normal, XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f)));
        outContact.penetration = max(penetration, 0.0f);
        return true;
    }

    void ClosestPointsBetweenSegments(FXMVECTOR p1, FXMVECTOR q1, FXMVECTOR p2, FXMVECTOR q2,
        XMVECTOR& outPoint1, XMVECTOR& outPoint2)
    {
        XMVECTOR d1 = XMVectorSubtract(q1, p1);
        XMVECTOR d2 = XMVectorSubtract(q2, p2);
        XMVECTOR r = XMVectorSubtract(p1, p2);

        float a = VectorDot(d1, d1);
        float e = VectorDot(d2, d2);
        float f = VectorDot(d2, r);

        float s = 0.0f;
        float t = 0.0f;

        if (a <= Epsilon && e <= Epsilon)
        {
            outPoint1 = p1;
            outPoint2 = p2;
            return;
        }

        if (a <= Epsilon)
        {
            t = clamp(f / e, 0.0f, 1.0f);
        }
        else
        {
            float c = VectorDot(d1, r);

            if (e <= Epsilon)
            {
                s = clamp(-c / a, 0.0f, 1.0f);
            }
            else
            {
                float b = VectorDot(d1, d2);
                float denom = a * e - b * b;

                if (fabsf(denom) > Epsilon) s = clamp((b * f - c * e) / denom, 0.0f, 1.0f);

                float tNumerator = b * s + f;
                if (tNumerator < 0.0f)
                {
                    t = 0.0f;
                    s = clamp(-c / a, 0.0f, 1.0f);
                }
                else if (tNumerator > e)
                {
                    t = 1.0f;
                    s = clamp((b - c) / a, 0.0f, 1.0f);
                }
                else
                {
                    t = tNumerator / e;
                }
            }
        }

        outPoint1 = XMVectorAdd(p1, XMVectorScale(d1, s));
        outPoint2 = XMVectorAdd(p2, XMVectorScale(d2, t));
    }

    bool ComputeCapsuleCapsuleContact(const CapsuleCollider& capsuleA, const CapsuleCollider& capsuleB, ContactInfo& outContact)
    {
        XMFLOAT3 capsuleAPointA = capsuleA.GetPointA();
        XMFLOAT3 capsuleAPointB = capsuleA.GetPointB();
        XMFLOAT3 capsuleBPointA = capsuleB.GetPointA();
        XMFLOAT3 capsuleBPointB = capsuleB.GetPointB();

        XMVECTOR segmentA0 = XMLoadFloat3(&capsuleAPointA);
        XMVECTOR segmentA1 = XMLoadFloat3(&capsuleAPointB);
        XMVECTOR segmentB0 = XMLoadFloat3(&capsuleBPointA);
        XMVECTOR segmentB1 = XMLoadFloat3(&capsuleBPointB);

        XMVECTOR pointA{};
        XMVECTOR pointB{};
        ClosestPointsBetweenSegments(segmentA0, segmentA1, segmentB0, segmentB1, pointA, pointB);

        XMVECTOR delta = XMVectorSubtract(pointB, pointA);
        float distanceSq = VectorLengthSq(delta);
        float radiusSum = capsuleA.GetRadius() + capsuleB.GetRadius();
        float radiusSumSq = radiusSum * radiusSum;

        if (distanceSq > radiusSumSq) return false;

        XMVECTOR normal{};
        float penetration = 0.0f;

        if (distanceSq > Epsilon)
        {
            float distance = sqrtf(distanceSq);
            normal = XMVectorScale(delta, 1.0f / distance);
            penetration = radiusSum - distance;
        }
        else
        {
            XMFLOAT3 centerAFloat = capsuleA.GetCenter();
            XMFLOAT3 centerBFloat = capsuleB.GetCenter();
            XMVECTOR centerA = XMLoadFloat3(&centerAFloat);
            XMVECTOR centerB = XMLoadFloat3(&centerBFloat);

            normal = SafeNormalize(XMVectorSubtract(centerB, centerA), XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
            penetration = radiusSum;
        }

        outContact.normal = ToFloat3(normal);
        outContact.penetration = max(penetration, 0.0f);
        return true;
    }

    void ApplyImpulse(const shared_ptr<GameObject>& objA, const shared_ptr<GameObject>& objB, const XMFLOAT3& impulse)
    {
        if (objA && objA->GetInverseMass() > 0.0f) objA->AddImpulse(Utiles::Vector3::Mul(impulse, -1.0f));
        if (objB && objB->GetInverseMass() > 0.0f) objB->AddImpulse(impulse);
    }

    void ResolveVelocity(const shared_ptr<GameObject>& objA, const shared_ptr<GameObject>& objB, const ContactInfo& contact)
    {
        float invMassA = objA->GetInverseMass();
        float invMassB = objB->GetInverseMass();
        float invMassSum = invMassA + invMassB;
        if (invMassSum <= 0.0f) return;

        XMFLOAT3 velA = objA->GetVelocity();
        XMFLOAT3 velB = objB->GetVelocity();
        XMFLOAT3 relativeVel = Utiles::Vector3::Sub(velB, velA);
        float velAlongNormal = Utiles::Vector3::Dot(relativeVel, contact.normal);
        float normalImpulse = 0.0f;

        if (velAlongNormal < 0.0f)
        {
            float restitution = min(objA->GetRestitution(), objB->GetRestitution());
            if (fabsf(velAlongNormal) < RestingVelocity) restitution = 0.0f;

            normalImpulse = -(1.0f + restitution) * velAlongNormal / invMassSum;
            ApplyImpulse(objA, objB, Utiles::Vector3::Mul(contact.normal, normalImpulse));
        }

        if (normalImpulse <= 0.0f) return;

        velA = objA->GetVelocity();
        velB = objB->GetVelocity();
        relativeVel = Utiles::Vector3::Sub(velB, velA);

        float normalSpeed = Utiles::Vector3::Dot(relativeVel, contact.normal);
        XMFLOAT3 tangent = Utiles::Vector3::Sub(relativeVel, Utiles::Vector3::Mul(contact.normal, normalSpeed));
        float tangentLengthSq = Utiles::Vector3::Dot(tangent, tangent);
        if (tangentLengthSq <= Epsilon) return;

        tangent = Utiles::Vector3::Mul(tangent, 1.0f / sqrtf(tangentLengthSq));
        float tangentImpulse = -Utiles::Vector3::Dot(relativeVel, tangent) / invMassSum;
        float maxFriction = normalImpulse * Friction;
        tangentImpulse = clamp(tangentImpulse, -maxFriction, maxFriction);

        ApplyImpulse(objA, objB, Utiles::Vector3::Mul(tangent, tangentImpulse));
    }

    void ApplyPositionCorrection(const shared_ptr<GameObject>& objA, const shared_ptr<GameObject>& objB, const ContactInfo& contact)
    {
        float invMassA = objA->GetInverseMass();
        float invMassB = objB->GetInverseMass();
        float invMassSum = invMassA + invMassB;
        if (invMassSum <= 0.0f) return;

        float correctionAmount = max(contact.penetration - ContactSlop, 0.0f) * PositionCorrectionPercent / invMassSum;
        if (correctionAmount <= 0.0f) return;

        XMFLOAT3 correction = Utiles::Vector3::Mul(contact.normal, correctionAmount);
        if (invMassA > 0.0f) objA->Transform(Utiles::Vector3::Mul(correction, -invMassA));
        if (invMassB > 0.0f) objB->Transform(Utiles::Vector3::Mul(correction, invMassB));
    }

    void MarkGrounded(const shared_ptr<GameObject>& object)
    {
        auto rigidbody = object ? object->GetRigidbody() : nullptr;
        if (!rigidbody) return;

        rigidbody->SetGrounded(true);
        XMFLOAT3 velocity = rigidbody->GetVelocity();
        if (velocity.y < 0.0f)
        {
            velocity.y = 0.0f;
            rigidbody->SetVelocity(velocity);
        }
    }

    void UpdateGroundedState(const shared_ptr<GameObject>& objA, const shared_ptr<GameObject>& objB, const ContactInfo& contact)
    {
        if (objA->GetInverseMass() > 0.0f && contact.normal.y < -0.5f) MarkGrounded(objA);
        if (objB->GetInverseMass() > 0.0f && contact.normal.y > 0.5f) MarkGrounded(objB);
    }
}

void CollisionManager::AddCollider(const shared_ptr<Collider>& collider)
{
    if (collider) m_colliders.push_back(collider);
}

void CollisionManager::ClearColliders()
{
    m_colliders.clear();
}

void CollisionManager::Update()
{
    for (int iteration = 0; iteration < SolverIterations; ++iteration)
    {
        vector<CollisionCandidate> candidates = BuildCollisionCandidates(m_colliders);

        for (const auto& candidate : candidates)
        {
            ContactInfo contact{};
            if (!CheckCollision(candidate.colliderA, candidate.colliderB, contact)) continue;

            ResolveVelocity(candidate.ownerA, candidate.ownerB, contact);
            ApplyPositionCorrection(candidate.ownerA, candidate.ownerB, contact);
            UpdateGroundedState(candidate.ownerA, candidate.ownerB, contact);

            if (iteration == 0) m_eventQueue.push({ candidate.colliderA, candidate.colliderB });
        }
    }

    ProcessCollisions();
}

void CollisionManager::ProcessCollisions()
{
    while (!m_eventQueue.empty())
    {
        const auto& event = m_eventQueue.front();
        m_eventQueue.pop();

        if (auto ownerA = event.colliderA->m_owner.lock()) ownerA->OnCollisionEnter(event.colliderB);
        if (auto ownerB = event.colliderB->m_owner.lock()) ownerB->OnCollisionEnter(event.colliderA);
    }
}

bool CollisionManager::CheckCollision(const shared_ptr<Collider>& a, const shared_ptr<Collider>& b, ContactInfo& outContact)
{
    if (!a || !b) return false;

    if (a->GetType() == ColliderType::Box && b->GetType() == ColliderType::Box)
    {
        auto boxA = static_pointer_cast<BoxCollider>(a);
        auto boxB = static_pointer_cast<BoxCollider>(b);
        return ComputeOBBOBBContact(boxA->GetWorldOBB(), boxB->GetWorldOBB(), outContact);
    }

    bool isACapsule = a->GetType() == ColliderType::Capsule;
    bool isBCapsule = b->GetType() == ColliderType::Capsule;

    if (isACapsule && isBCapsule)
    {
        auto capsuleA = static_pointer_cast<CapsuleCollider>(a);
        auto capsuleB = static_pointer_cast<CapsuleCollider>(b);
        return ComputeCapsuleCapsuleContact(*capsuleA, *capsuleB, outContact);
    }

    if ((isACapsule && b->GetType() == ColliderType::Box) ||
        (a->GetType() == ColliderType::Box && isBCapsule))
    {
        auto capsule = static_pointer_cast<CapsuleCollider>(isACapsule ? a : b);
        auto box = static_pointer_cast<BoxCollider>(isACapsule ? b : a);

        ContactInfo capsuleContact{};
        if (!ComputeCapsuleOBBContact(*capsule, box->GetWorldOBB(), capsuleContact)) return false;

        outContact.normal = isACapsule ? capsuleContact.normal : Utiles::Vector3::Mul(capsuleContact.normal, -1.0f);
        outContact.penetration = capsuleContact.penetration;
        return true;
    }

    return false;
}

bool CollisionManager::Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outHitDist, const shared_ptr<Collider>& ignoreCollider) const
{
    bool isHit = false;
    float minHitDist = FLT_MAX;

    for (const auto& collider : m_colliders)
    {
        if (!collider || collider == ignoreCollider) continue;

        float hitDist = 0.0f;
        if (collider->Raycast(origin, direction, hitDist) && hitDist < minHitDist)
        {
            minHitDist = hitDist;
            isHit = true;
        }
    }

    if (isHit) outHitDist = minHitDist;
    return isHit;
}
