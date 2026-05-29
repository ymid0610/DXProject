#pragma once
#include "stdafx.h"
#include "mesh.h"

enum class ColliderType
{
    Box,
    Capsule
};

class GameObject;

class Collider
{
public:
    explicit Collider(ColliderType type) : m_type(type) {}
    virtual ~Collider() = default;

    virtual void Update(const XMFLOAT4X4& worldMatrix) = 0;
    virtual XMFLOAT3 GetCenter() const = 0;
    virtual BoundingBox GetWorldAABB() const = 0;
    virtual bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outDist) const = 0;

    ColliderType GetType() const { return m_type; }
    void SetOwner(const shared_ptr<GameObject>& owner) { m_owner = owner; }

public:
    weak_ptr<GameObject> m_owner;

protected:
    ColliderType m_type;
};

class BoxCollider : public Collider
{
public:
    explicit BoxCollider(const shared_ptr<Mesh>& mesh) : Collider(ColliderType::Box)
    {
        if (mesh) m_localOBB = mesh->GetLocalOBB();
    }

    void Update(const XMFLOAT4X4& worldMatrix) override
    {
        m_localOBB.Transform(m_worldOBB, XMLoadFloat4x4(&worldMatrix));

        XMFLOAT3 corners[8]{};
        m_worldOBB.GetCorners(corners);
        BoundingBox::CreateFromPoints(m_worldAABB, 8, corners, sizeof(XMFLOAT3));
    }

    BoundingOrientedBox GetWorldOBB() const { return m_worldOBB; }
    XMFLOAT3 GetCenter() const override { return m_worldOBB.Center; }
    BoundingBox GetWorldAABB() const override { return m_worldAABB; }
    void SetOffset(XMFLOAT3 offset) { m_localOBB.Center = offset; }

    bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outDist) const override;

private:
    BoundingOrientedBox m_localOBB{};
    BoundingOrientedBox m_worldOBB{};
    BoundingBox m_worldAABB{};
};

class CapsuleCollider : public Collider
{
public:
    CapsuleCollider(float radius, float height)
        : Collider(ColliderType::Capsule), m_radius(radius), m_height(height) {}

    void Update(const XMFLOAT4X4& worldMatrix) override;
    XMFLOAT3 GetCenter() const override { return m_worldCenter; }
    BoundingBox GetWorldAABB() const override { return m_worldAABB; }
    bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outDist) const override;

    float GetRadius() const { return m_radius; }
    XMFLOAT3 GetPointA() const { return m_worldPointA; }
    XMFLOAT3 GetPointB() const { return m_worldPointB; }

private:
    float m_radius = 0.5f;
    float m_height = 1.0f;
    XMFLOAT3 m_worldCenter{};
    XMFLOAT3 m_worldPointA{};
    XMFLOAT3 m_worldPointB{};
    BoundingBox m_worldAABB{};
};
