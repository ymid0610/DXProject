#include "scene.h"
#include "framework.h"

namespace
{
    constexpr float FirstPersonFlashlightOffset = 0.05f;
    constexpr float ThirdPersonFlashlightForwardOffset = 0.18f;

    XMFLOAT4X4 BuildCameraAnchoredMatrix(const shared_ptr<Camera>& camera)
    {
        const XMFLOAT3 right = camera->GetU();
        const XMFLOAT3 up = camera->GetV();
        const XMFLOAT3 forward = camera->GetN();
        const XMFLOAT3 eye = camera->GetEye();

        return XMFLOAT4X4{
            right.x, right.y, right.z, 0.0f,
            up.x, up.y, up.z, 0.0f,
            forward.x, forward.y, forward.z, 0.0f,
            eye.x, eye.y, eye.z, 1.0f
        };
    }
}

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
    if (m_camera && m_player) m_camera->UpdateEye(m_player->GetPosition());
    UpdateFlashlight();
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
    if (m_camera->IsFirstPerson() || (springCamera && springCamera->GetArmLength() <= 1.0f))
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
    HandleSceneShortcuts();
    if (m_player) m_player->KeyboardEvent(timeElapsed);
}

void Scene::HandleSceneShortcuts()
{
    bool flashlightKeyDown = (GetAsyncKeyState('F') & 0x8000) != 0;
    bool cameraToggleKeyDown = (GetAsyncKeyState('V') & 0x8000) != 0;

    if (flashlightKeyDown && !m_flashlightKeyDown) ToggleFlashlight();
    if (cameraToggleKeyDown && !m_cameraToggleKeyDown) ToggleCameraMode();

    m_flashlightKeyDown = flashlightKeyDown;
    m_cameraToggleKeyDown = cameraToggleKeyDown;
}

void Scene::ToggleFlashlight()
{
    if (m_flashlight) m_flashlight->SetEnabled(!m_flashlight->IsEnabled());
}

void Scene::ToggleCameraMode()
{
    if (!m_camera) return;

    SetActiveCamera(m_camera->IsFirstPerson() ? CreateThirdPersonCamera() : CreateFirstPersonCamera());
}

void Scene::SetActiveCamera(const shared_ptr<Camera>& camera)
{
    if (!camera) return;

    ConfigureCameraLens(camera);
    m_camera = camera;

    if (m_player)
    {
        m_player->SetCamera(m_camera);
        m_camera->UpdateEye(m_player->GetPosition());
        UpdateFlashlight();
    }
}

shared_ptr<Camera> Scene::CreateFirstPersonCamera() const
{
    return make_shared<FirstPersonCamera>();
}

shared_ptr<Camera> Scene::CreateThirdPersonCamera() const
{
    return make_shared<SpringArmCamera>();
}

void Scene::ConfigureCameraLens(const shared_ptr<Camera>& camera) const
{
    if (!camera || !g_framework) return;
    camera->SetLens(0.25f * XM_PI, g_framework->GetAspectRatio(), 0.1f, 1000.0f);
}

void Scene::UpdateFlashlight()
{
    if (!m_flashlight) return;

    m_flashlight->SetPosition(GetFlashlightPosition());
    m_flashlight->SetDirection(GetFlashlightDirection());
}

XMFLOAT3 Scene::GetFlashlightPosition() const
{
    if (m_camera && m_camera->IsFirstPerson())
    {
        return Utiles::Vector3::Add(
            m_camera->GetEye(),
            Utiles::Vector3::Mul(m_camera->GetN(), FirstPersonFlashlightOffset));
    }

    if (m_player)
    {
        XMFLOAT3 eyePosition = Utiles::Vector3::Add(
            m_player->GetPosition(),
            XMFLOAT3{ 0.0f, Settings::FirstPersonEyeHeight, 0.0f });
        return Utiles::Vector3::Add(
            eyePosition,
            Utiles::Vector3::Mul(GetFlashlightDirection(), ThirdPersonFlashlightForwardOffset));
    }

    return m_camera ? m_camera->GetEye() : XMFLOAT3{ 0.0f, 0.0f, 0.0f };
}

XMFLOAT3 Scene::GetFlashlightDirection() const
{
    if (m_camera && m_camera->IsFirstPerson()) return m_camera->GetN();
    if (m_player) return Utiles::Vector3::Normalize(m_player->GetFront());
    return m_camera ? m_camera->GetN() : XMFLOAT3{ 0.0f, 0.0f, 1.0f };
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
    if (m_shader)
    {
        vector<LightShaderData> lightData;
        lightData.reserve(m_lights.size());
        for (const auto& light : m_lights)
        {
            if (light) lightData.push_back(light->BuildShaderData());
        }

        m_shader->SetLights(lightData);
        m_shader->UpdateShaderVariable(commandList);
    }

    for (const auto& object : m_objects)
    {
        if (object) object->Render(commandList);
    }

    const bool isFirstPerson = m_camera && m_camera->IsFirstPerson();
    if (m_player && !isFirstPerson) m_player->Render(commandList);
    if (isFirstPerson) RenderFirstPersonOverlay(commandList);
}

void Scene::RenderFirstPersonOverlay(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    if (!m_camera || !m_overlayShader) return;

    const XMFLOAT4X4 cameraAnchoredMatrix = BuildCameraAnchoredMatrix(m_camera);
    m_overlayShader->UpdateShaderVariable(commandList);

    if (m_firstPersonGun)
    {
        m_firstPersonGun->SetWorldMatrix(cameraAnchoredMatrix);
        m_firstPersonGun->Render(commandList);
    }

    if (m_crosshair)
    {
        m_crosshair->SetWorldMatrix(cameraAnchoredMatrix);
        m_crosshair->Render(commandList);
    }
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
    m_overlayShader.reset();
    m_cube.reset();
    m_capsuleMesh.reset();
    m_firstPersonGunMesh.reset();
    m_crosshairMesh.reset();
    m_firstPersonGun.reset();
    m_crosshair.reset();
    m_collisionManager.reset();
    m_physicsManager.reset();
    m_lights.clear();
    m_mainLight.reset();
    m_flashlight.reset();
}

void Scene::ReleaseUploadBuffer()
{
    if (m_cube) m_cube->ReleaseUploadBuffer();
    if (m_capsuleMesh) m_capsuleMesh->ReleaseUploadBuffer();
    if (m_firstPersonGunMesh) m_firstPersonGunMesh->ReleaseUploadBuffer();
    if (m_crosshairMesh) m_crosshairMesh->ReleaseUploadBuffer();
}
