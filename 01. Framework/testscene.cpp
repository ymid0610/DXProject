#include "scene.h"
#include "framework.h"

TestScene::TestScene()
{
}

void TestScene::Update(FLOAT timeElapsed)
{
    Scene::Update(timeElapsed);
    UpdateSpringCamera();
}

void TestScene::BuildObjects(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList,
    const ComPtr<ID3D12RootSignature>& rootSignature)
{
    m_collisionManager = make_unique<CollisionManager>();
    m_physicsManager = make_unique<PhysicsManager>();

    m_shader = make_shared<Shader>(device, rootSignature);
    m_cube = make_shared<CubeIndexMesh>(device, commandList);

    auto capsuleMesh = make_shared<CapsuleIndexMesh>(device, commandList, 1.0f, 1.0f);
    m_player = make_shared<Player>();
    m_player->SetMesh(capsuleMesh);
    m_player->SetPosition(XMFLOAT3{ 0.0f, 0.0f, 0.0f });

    auto playerCollider = make_shared<CapsuleCollider>(1.0f, 1.0f);
    auto playerRigidbody = make_shared<Rigidbody>();
    playerRigidbody->SetMass(3.0f);
    playerRigidbody->SetRestitution(0.0f);
    playerRigidbody->SetDrag(0.15f);

    m_player->SetCollider(playerCollider);
    m_player->SetRigidbody(playerRigidbody);
    m_collisionManager->AddCollider(playerCollider);
    m_physicsManager->AddRigidbody(playerRigidbody);

    for (int x = -15; x <= 15; x += 5)
    {
        for (int y = -15; y <= 15; y += 5)
        {
            for (int z = -15; z <= 15; z += 5)
            {
                if (x == 0 && y == 0 && z == 0) continue;

                auto object = make_shared<GameObject>();
                object->SetMesh(m_cube);
                object->SetPosition(XMFLOAT3{ static_cast<FLOAT>(x), static_cast<FLOAT>(y), static_cast<FLOAT>(z) });

                auto collider = make_shared<BoxCollider>(m_cube);
                auto rigidbody = make_shared<Rigidbody>();
                rigidbody->SetRestitution(0.05f);
                rigidbody->SetDrag(0.10f);

                object->SetCollider(collider);
                object->SetRigidbody(rigidbody);

                m_objects.push_back(object);
                m_collisionManager->AddCollider(collider);
                m_physicsManager->AddRigidbody(rigidbody);
            }
        }
    }

    auto planeMesh = make_shared<PlaneMesh>(device, commandList);

    auto floor = make_shared<GameObject>();
    floor->SetMesh(planeMesh);
    floor->SetPosition(XMFLOAT3{ 0.0f, -17.5f, 0.0f });
    auto floorCollider = make_shared<BoxCollider>(planeMesh);
    floor->SetCollider(floorCollider);
    m_objects.push_back(floor);
    m_collisionManager->AddCollider(floorCollider);

    auto rightWall = make_shared<GameObject>();
    rightWall->SetMesh(planeMesh);
    rightWall->SetPosition(XMFLOAT3{ 15.0f, -2.5f, 0.0f });
    rightWall->Rotate(0.0f, 0.0f, 90.0f);
    auto rightWallCollider = make_shared<BoxCollider>(planeMesh);
    rightWall->SetCollider(rightWallCollider);
    m_objects.push_back(rightWall);
    m_collisionManager->AddCollider(rightWallCollider);

    m_camera = make_shared<SpringArmCamera>();
    m_camera->SetLens(0.25f * XM_PI, g_framework->GetAspectRatio(), 0.1f, 1000.0f);
    m_player->SetCamera(m_camera);
}