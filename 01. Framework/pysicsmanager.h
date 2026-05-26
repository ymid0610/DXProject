#pragma once
#include "stdafx.h"
#include "rigidbody.h"

class PysicsManager
{
public:
	// 생성자 소멸자
	PysicsManager() = default;
	~PysicsManager() = default;

	// 업데이트 함수
	void Update(FLOAT timeElapsed);

	// 멤버 함수

	void AddRigidbody(const shared_ptr<Rigidbody>& rigidbody);
	// void ClearRigidbodies();

	
private:

	vector<shared_ptr<Rigidbody>> m_rigidbodies;

	float m_gravity = -9.81f; 
	XMFLOAT3 m_globalGravity{ 0.0f, m_gravity, 0.0f };

};



