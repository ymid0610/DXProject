#include "pysicsmanager.h"

void PysicsManager::Update(FLOAT timeElapsed)
{
	for (const auto& rigidbody : m_rigidbodies)
	{
		if (rigidbody) {
			rigidbody->Update(timeElapsed, m_globalGravity);
		}
	}
}

void PysicsManager::AddRigidbody(const shared_ptr<Rigidbody>& rigidbody)
{
	if (rigidbody) {
		m_rigidbodies.push_back(rigidbody);
	}
}