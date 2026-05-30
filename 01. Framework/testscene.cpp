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
    m_overlayShader = make_shared<Shader>(device, rootSignature, "PIXEL_UNLIT", false);
    m_capsuleMesh = make_shared<CapsuleIndexMesh>(device, commandList, Settings::CapsuleRadius, Settings::CapsuleHeight, 32);
    m_firstPersonGunMesh = make_shared<FirstPersonGunMesh>(device, commandList);
    m_crosshairMesh = make_shared<CrosshairMesh>(device, commandList);
    m_bulletMesh = make_shared<BulletMesh>(device, commandList);

    m_firstPersonGun = make_shared<GameObject>();
    m_firstPersonGun->SetMesh(m_firstPersonGunMesh);

    m_crosshair = make_shared<GameObject>();
    m_crosshair->SetMesh(m_crosshairMesh);

    m_mainLight = make_shared<PointLight>();
    m_mainLight->SetPosition(XMFLOAT3{ 0.0f, -10.0f, 0.0f });
    m_mainLight->SetRange(360.0f);
    m_mainLight->SetIntensity(1.f);
    m_mainLight->SetColor(XMFLOAT3{ 0.82f, 0.90f, 1.0f });
    m_lights.push_back(m_mainLight);

    m_flashlight = make_shared<SpotLight>();
    m_flashlight->SetRange(30.0f);
    m_flashlight->SetIntensity(1.45f);
    m_flashlight->SetColor(XMFLOAT3{ 1.0f, 0.94f, 0.82f });
    m_lights.push_back(m_flashlight);

    m_player = make_shared<Player>();
    m_player->SetMesh(m_capsuleMesh);
    m_player->SetPosition(XMFLOAT3{ 0.0f, 0.0f, 0.0f });

    auto playerCollider = make_shared<CapsuleCollider>(Settings::CapsuleRadius, Settings::CapsuleHeight);
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
                object->SetMesh(m_capsuleMesh);
                object->SetPosition(XMFLOAT3{ static_cast<FLOAT>(x), static_cast<FLOAT>(y), static_cast<FLOAT>(z) });

                auto collider = make_shared<CapsuleCollider>(Settings::CapsuleRadius, Settings::CapsuleHeight);
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

    SetActiveCamera(make_shared<FirstPersonCamera>());
}
