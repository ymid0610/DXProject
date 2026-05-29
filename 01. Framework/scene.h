#pragma once
#include "stdafx.h"
#include "shader.h"
#include "mesh.h"
#include "object.h"
#include "player.h"
#include "camera.h"
#include "collisionmanager.h"
#include "physicsmanager.h"

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
    shared_ptr<Shader> m_shader;
    shared_ptr<Camera> m_camera;
    shared_ptr<Player> m_player;
    vector<shared_ptr<GameObject>> m_objects;
    shared_ptr<Mesh> m_cube;

    unique_ptr<CollisionManager> m_collisionManager;
    unique_ptr<PhysicsManager> m_physicsManager;
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