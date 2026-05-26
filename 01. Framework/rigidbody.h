#pragma once
#include "stdafx.h"

class GameObject;

enum class ForceMode {
    Force,      // 지속적인 힘 (예: 바람, 로켓 엔진) (질량 적용)
    Impulse,    // 순간적인 타격 (예: 폭발, 총알 피격) (질량 적용)
    VelocityChange // 질량 무시하고 즉시 속도 변경 (예: 점프 패드)
};

class Rigidbody 
{
public:
	// 생성자 소멸자
	Rigidbody() = default;
    ~Rigidbody() = default;

	// 업데이트 함수
    void Update(FLOAT timeElapsed, const XMFLOAT3& globalGravity);

    // Getters
    std::weak_ptr<GameObject> GetOwner() const { return m_owner; }
    XMFLOAT3 GetVelocity() const { return velocity; }
    float GetMass() const { return mass; }
    float GetDrag() const { return drag; }
    float GetRestitution() const { return restitution; }
    bool IsKinematic() const { return isKinematic; }

	// Setters
	void SetOwner(const shared_ptr<GameObject>& owner) { m_owner = owner; }
    void SetMass(float m) { mass = max(0.001f, m); }
    void SetDrag(float d) { drag = d; }
    void SetRestitution(float r) { restitution = max(0.0f, min(1.0f, r)); }
    void SetKinematic(bool kinematic) { isKinematic = kinematic; }
    void SetUseGravity(bool gravity) { useGravity = gravity; }
    void SetVelocity(DirectX::XMFLOAT3 v) { velocity = v; }

    // 멤버 함수
    void Rigidbody::AddForce(const XMFLOAT3& force, ForceMode mode);

private:
    float mass = 1.0f;          // 질량
    float drag = 1.0f;          // 이동 저항력 (0이면 저항 없음)
    float restitution = 0.5f; // 반발 계수 (0: 찰흙, 1: 완벽한 탄성구)
    float gravityScale = 1.0f;  // 중력 배율
    bool isKinematic = false;   // 물리 연산 무시 여부
    bool useGravity = true;     // 중력 적용 여부

    DirectX::XMFLOAT3 velocity{ 0.f, 0.f, 0.f };
    DirectX::XMFLOAT3 acceleration{ 0.f, 0.f, 0.f };
    DirectX::XMFLOAT3 accumulatedForce{ 0.f, 0.f, 0.f }; 

    std::weak_ptr<GameObject> m_owner;
};
