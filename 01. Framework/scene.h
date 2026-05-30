#pragma once
#include "stdafx.h"
#include "shader.h"
#include "mesh.h"
#include "object.h"
#include "player.h"
#include "camera.h"
#include "collisionmanager.h"
#include "physicsmanager.h"
#include "light.h"
#include "bullet.h"

class Scene
{
public:
    Scene();
    virtual ~Scene() = default;

    virtual void Update(FLOAT timeElapsed);
    virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
    virtual void BuildObjects(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList,
        const ComPtr<ID3D12RootSignature>& rootSignature);

    void ReleaseObjects();
    void ReleaseUploadBuffer();
    void UpdateSpringCamera();

    void MouseEvent(HWND hWnd, FLOAT timeElapsed);
    void KeyboardEvent(FLOAT timeElapsed);
    void MouseWheelEvent(WPARAM wParam);

protected:
    void HandleSceneShortcuts();
    void ToggleFlashlight();
    void ToggleCameraMode();
    void SetActiveCamera(const shared_ptr<Camera>& camera);
    shared_ptr<Camera> CreateFirstPersonCamera() const;
    shared_ptr<Camera> CreateThirdPersonCamera() const;
    void ConfigureCameraLens(const shared_ptr<Camera>& camera) const;
    void HandleFireInput();
    void ToggleDebugCamera();
    void UpdateDebugCamera(FLOAT timeElapsed);
    void FireBullet();
    void UpdateBullets(FLOAT timeElapsed);
    void UpdateFlashlight();
    XMFLOAT3 GetBulletSpawnPosition(const XMFLOAT3& direction) const;
    XMFLOAT3 GetBulletDirection() const;
    XMFLOAT3 GetFlashlightPosition() const;
    XMFLOAT3 GetFlashlightDirection() const;
    void RenderFirstPersonOverlay(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

protected:
    shared_ptr<Shader> m_shader;
    shared_ptr<Shader> m_overlayShader;
    shared_ptr<Camera> m_camera;
    shared_ptr<Camera> m_savedGameplayCamera;
    shared_ptr<Player> m_player;
    vector<shared_ptr<GameObject>> m_objects;
    shared_ptr<Mesh> m_cube;
    shared_ptr<Mesh> m_capsuleMesh;
    shared_ptr<Mesh> m_firstPersonGunMesh;
    shared_ptr<Mesh> m_crosshairMesh;
    shared_ptr<Mesh> m_bulletMesh;
    shared_ptr<GameObject> m_firstPersonGun;
    shared_ptr<GameObject> m_crosshair;
    vector<shared_ptr<Bullet>> m_bullets;

    unique_ptr<CollisionManager> m_collisionManager;
    unique_ptr<PhysicsManager> m_physicsManager;
    vector<shared_ptr<Light>> m_lights;
    shared_ptr<PointLight> m_mainLight;
    shared_ptr<SpotLight> m_flashlight;

    bool m_flashlightKeyDown = false;
    bool m_cameraToggleKeyDown = false;
    bool m_fireKeyDown = false;
    bool m_debugCameraEnabled = false;
    bool m_debugCameraKeyDown = false;
};

class TestScene : public Scene
{
public:
    TestScene();
    ~TestScene() = default;

    void Update(FLOAT timeElapsed) override;
    void BuildObjects(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList,
        const ComPtr<ID3D12RootSignature>& rootSignature) override;
};
