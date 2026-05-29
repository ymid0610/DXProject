#include "scene.h"
#include "framework.h"

Scene::Scene()
{
}

void Scene::Update(FLOAT timeElapsed)
{
    if (m_player) m_player->Update(timeElapsed);

    for (auto& object : m_objects)
    {
        if (object) object->Update(timeElapsed);
    }

    if (m_physicsManager) m_physicsManager->Update(timeElapsed);
    if (m_collisionManager) m_collisionManager->Update();
}

void Scene::UpdateSpringCamera()
{
    auto springCamera = dynamic_pointer_cast<SpringArmCamera>(m_camera);
    if (!springCamera || !m_collisionManager || !m_player || !m_player->GetCollider()) return;

    XMFLOAT3 origin = springCamera->GetLookAtPosition();
    XMFLOAT3 direction = springCamera->GetDirectionToCamera();
    float maxDistance = springCamera->GetArmLength();
    float hitDist = 0.0f;

    if (m_collisionManager->Raycast(origin, direction, hitDist, m_player->GetCollider()) && hitDist < maxDistance)
    {
        springCamera->SetCollisionDistance(hitDist);
    }
}

void Scene::MouseEvent(HWND hWnd, FLOAT timeElapsed)
{
    if (!m_camera || !m_player) return;

    SetCursor(NULL);

    RECT windowRect{};
    GetWindowRect(hWnd, &windowRect);

    POINT center{
        windowRect.left + static_cast<LONG>(g_framework->GetWindowWidth() / 2),
        windowRect.top + static_cast<LONG>(g_framework->GetWindowHeight() / 2)
    };

    POINT mousePosition{};
    GetCursorPos(&mousePosition);

    float dx = XMConvertToRadians(0.15f * static_cast<FLOAT>(center.x - mousePosition.x));
    float dy = XMConvertToRadians(0.15f * static_cast<FLOAT>(center.y - mousePosition.y));

    auto springCamera = dynamic_pointer_cast<SpringArmCamera>(m_camera);
    if (springCamera && springCamera->GetArmLength() <= 1.0f)
    {
        m_player->Rotate(0.0f, XMConvertToDegrees(-dx), 0.0f);
    }

    m_camera->RotateYaw(dx);
    m_camera->RotatePitch(dy);
    SetCursorPos(center.x, center.y);

    m_player->MouseEvent(timeElapsed, 0);
}

void Scene::KeyboardEvent(FLOAT timeElapsed)
{
    if (m_player) m_player->KeyboardEvent(timeElapsed);
}

void Scene::MouseWheelEvent(WPARAM wParam)
{
    if (!m_player) return;

    short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    m_player->MouseEvent(0.0f, wheelDelta);
}

void Scene::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    if (m_camera) m_camera->UpdateShaderVariable(commandList);
    if (m_shader) m_shader->UpdateShaderVariable(commandList);

    for (const auto& object : m_objects)
    {
        if (object) object->Render(commandList);
    }

    if (m_player) m_player->Render(commandList);
}

void Scene::BuildObjects(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList,
    const ComPtr<ID3D12RootSignature>& rootSignature)
{
    (void)device;
    (void)commandList;
    (void)rootSignature;
}

void Scene::ReleaseObjects()
{
    m_objects.clear();
    m_player.reset();
    m_camera.reset();
    m_shader.reset();
    m_cube.reset();
    m_collisionManager.reset();
    m_physicsManager.reset();
}

void Scene::ReleaseUploadBuffer()
{
    if (m_cube) m_cube->ReleaseUploadBuffer();
}