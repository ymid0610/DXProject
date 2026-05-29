#include "mesh.h"

namespace
{
    template <typename Vertex>
    void CreateVertexBuffer(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList,
        const vector<Vertex>& vertices,
        ComPtr<ID3D12Resource>& vertexBuffer,
        ComPtr<ID3D12Resource>& vertexUploadBuffer,
        D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
    {
        const UINT vertexBufferSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));

        Utiles::ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&vertexBuffer)));

        Utiles::ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexUploadBuffer)));

        D3D12_SUBRESOURCE_DATA vertexData{};
        vertexData.pData = vertices.data();
        vertexData.RowPitch = vertexBufferSize;
        vertexData.SlicePitch = vertexData.RowPitch;
        UpdateSubresources<1>(commandList.Get(), vertexBuffer.Get(), vertexUploadBuffer.Get(), 0, 0, 1, &vertexData);

        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.SizeInBytes = vertexBufferSize;
        vertexBufferView.StrideInBytes = sizeof(Vertex);
    }

    void CreateIndexBuffer(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList,
        const vector<UINT>& indices,
        ComPtr<ID3D12Resource>& indexBuffer,
        ComPtr<ID3D12Resource>& indexUploadBuffer,
        D3D12_INDEX_BUFFER_VIEW& indexBufferView)
    {
        const UINT indexBufferSize = static_cast<UINT>(indices.size() * sizeof(UINT));

        Utiles::ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&indexBuffer)));

        Utiles::ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&indexUploadBuffer)));

        D3D12_SUBRESOURCE_DATA indexData{};
        indexData.pData = indices.data();
        indexData.RowPitch = indexBufferSize;
        indexData.SlicePitch = indexData.RowPitch;
        UpdateSubresources<1>(commandList.Get(), indexBuffer.Get(), indexUploadBuffer.Get(), 0, 0, 1, &indexData);

        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_INDEX_BUFFER));

        indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
        indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        indexBufferView.SizeInBytes = indexBufferSize;
    }
}

void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->DrawInstanced(m_vertices, 1, 0, 0);
}

void Mesh::ReleaseUploadBuffer()
{
    if (m_vertexUploadBuffer) m_vertexUploadBuffer.Reset();
}

void Mesh::CreateBoundingBox(const void* vertices, UINT vertexCount, UINT stride)
{
    if (vertexCount == 0 || !vertices) return;

    XMFLOAT3 vMin{ +FLT_MAX, +FLT_MAX, +FLT_MAX };
    XMFLOAT3 vMax{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

    const BYTE* vertexBytes = static_cast<const BYTE*>(vertices);

    for (UINT i = 0; i < vertexCount; ++i)
    {
        const XMFLOAT3* position = reinterpret_cast<const XMFLOAT3*>(vertexBytes);

        vMin.x = min(vMin.x, position->x);
        vMin.y = min(vMin.y, position->y);
        vMin.z = min(vMin.z, position->z);

        vMax.x = max(vMax.x, position->x);
        vMax.y = max(vMax.y, position->y);
        vMax.z = max(vMax.z, position->z);

        vertexBytes += stride;
    }

    BoundingBox::CreateFromPoints(m_localAABB, XMLoadFloat3(&vMin), XMLoadFloat3(&vMax));

    m_localOBB.Center = m_localAABB.Center;
    m_localOBB.Extents = m_localAABB.Extents;
    m_localOBB.Orientation = XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f };
}

void IndexMesh::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->DrawIndexedInstanced(m_indices, 1, 0, 0, 0);
}

void IndexMesh::ReleaseUploadBuffer()
{
    Mesh::ReleaseUploadBuffer();
    if (m_indexUploadBuffer) m_indexUploadBuffer.Reset();
}

CubeMesh::CubeMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    vector<Vertex> vertices;
    vertices.reserve(36);

    const XMFLOAT3 LEFTDOWNFRONT = { -1.f, -1.f, -1.f };
    const XMFLOAT3 LEFTDOWNBACK = { -1.f, -1.f, +1.f };
    const XMFLOAT3 LEFTUPFRONT = { -1.f, +1.f, -1.f };
    const XMFLOAT3 LEFTUPBACK = { -1.f, +1.f, +1.f };
    const XMFLOAT3 RIGHTDOWNFRONT = { +1.f, -1.f, -1.f };
    const XMFLOAT3 RIGHTDOWNBACK = { +1.f, -1.f, +1.f };
    const XMFLOAT3 RIGHTUPFRONT = { +1.f, +1.f, -1.f };
    const XMFLOAT3 RIGHTUPBACK = { +1.f, +1.f, +1.f };

    const XMFLOAT4 cLDF{ 1.0f, 0.0f, 0.0f, 1.0f };
    const XMFLOAT4 cLDB{ 0.0f, 1.0f, 0.0f, 1.0f };
    const XMFLOAT4 cLUF{ 0.0f, 0.0f, 1.0f, 1.0f };
    const XMFLOAT4 cLUB{ 1.0f, 1.0f, 0.0f, 1.0f };
    const XMFLOAT4 cRDF{ 1.0f, 0.0f, 1.0f, 1.0f };
    const XMFLOAT4 cRDB{ 0.0f, 1.0f, 1.0f, 1.0f };
    const XMFLOAT4 cRUF{ 0.5f, 0.5f, 0.0f, 1.0f };
    const XMFLOAT4 cRUB{ 0.0f, 0.5f, 0.5f, 1.0f };

    auto addFace = [&vertices](const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& c, const XMFLOAT3& d,
        const XMFLOAT3& normal, const XMFLOAT4& ca, const XMFLOAT4& cb, const XMFLOAT4& cc, const XMFLOAT4& cd)
    {
        vertices.push_back({ a, normal, ca });
        vertices.push_back({ b, normal, cb });
        vertices.push_back({ c, normal, cc });
        vertices.push_back({ a, normal, ca });
        vertices.push_back({ c, normal, cc });
        vertices.push_back({ d, normal, cd });
    };

    addFace(LEFTUPFRONT, RIGHTUPFRONT, RIGHTDOWNFRONT, LEFTDOWNFRONT, { 0.0f, 0.0f, -1.0f }, cLUF, cRUF, cRDF, cLDF);
    addFace(LEFTUPBACK, RIGHTUPBACK, RIGHTUPFRONT, LEFTUPFRONT, { 0.0f, 1.0f, 0.0f }, cLUB, cRUB, cRUF, cLUF);
    addFace(LEFTDOWNBACK, RIGHTDOWNBACK, RIGHTUPBACK, LEFTUPBACK, { 0.0f, 0.0f, 1.0f }, cLDB, cRDB, cRUB, cLUB);
    addFace(LEFTDOWNFRONT, RIGHTDOWNFRONT, RIGHTDOWNBACK, LEFTDOWNBACK, { 0.0f, -1.0f, 0.0f }, cLDF, cRDF, cRDB, cLDB);
    addFace(LEFTUPBACK, LEFTUPFRONT, LEFTDOWNFRONT, LEFTDOWNBACK, { -1.0f, 0.0f, 0.0f }, cLUB, cLUF, cLDF, cLDB);
    addFace(RIGHTUPFRONT, RIGHTUPBACK, RIGHTDOWNBACK, RIGHTDOWNFRONT, { 1.0f, 0.0f, 0.0f }, cRUF, cRUB, cRDB, cRDF);

    m_vertices = static_cast<UINT>(vertices.size());
    CreateBoundingBox(vertices.data(), m_vertices, sizeof(Vertex));
    CreateVertexBuffer(device, commandList, vertices, m_vertexBuffer, m_vertexUploadBuffer, m_vertexBufferView);
}

CubeIndexMesh::CubeIndexMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    vector<Vertex> vertices;
    vector<UINT> indices;
    vertices.reserve(24);
    indices.reserve(36);

    const XMFLOAT3 LEFTDOWNFRONT = { -1.f, -1.f, -1.f };
    const XMFLOAT3 LEFTDOWNBACK = { -1.f, -1.f, +1.f };
    const XMFLOAT3 LEFTUPFRONT = { -1.f, +1.f, -1.f };
    const XMFLOAT3 LEFTUPBACK = { -1.f, +1.f, +1.f };
    const XMFLOAT3 RIGHTDOWNFRONT = { +1.f, -1.f, -1.f };
    const XMFLOAT3 RIGHTDOWNBACK = { +1.f, -1.f, +1.f };
    const XMFLOAT3 RIGHTUPFRONT = { +1.f, +1.f, -1.f };
    const XMFLOAT3 RIGHTUPBACK = { +1.f, +1.f, +1.f };

    const XMFLOAT4 cLDF{ 1.0f, 0.0f, 0.0f, 1.0f };
    const XMFLOAT4 cLDB{ 0.0f, 1.0f, 0.0f, 1.0f };
    const XMFLOAT4 cLUF{ 0.0f, 0.0f, 1.0f, 1.0f };
    const XMFLOAT4 cLUB{ 1.0f, 1.0f, 0.0f, 1.0f };
    const XMFLOAT4 cRDF{ 1.0f, 0.0f, 1.0f, 1.0f };
    const XMFLOAT4 cRDB{ 0.0f, 1.0f, 1.0f, 1.0f };
    const XMFLOAT4 cRUF{ 0.5f, 0.5f, 0.0f, 1.0f };
    const XMFLOAT4 cRUB{ 0.0f, 0.5f, 0.5f, 1.0f };

    auto addFace = [&vertices, &indices](const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& c, const XMFLOAT3& d,
        const XMFLOAT3& normal, const XMFLOAT4& ca, const XMFLOAT4& cb, const XMFLOAT4& cc, const XMFLOAT4& cd)
    {
        const UINT baseIndex = static_cast<UINT>(vertices.size());
        vertices.push_back({ a, normal, ca });
        vertices.push_back({ b, normal, cb });
        vertices.push_back({ c, normal, cc });
        vertices.push_back({ d, normal, cd });

        indices.push_back(baseIndex + 0); indices.push_back(baseIndex + 1); indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 0); indices.push_back(baseIndex + 2); indices.push_back(baseIndex + 3);
    };

    addFace(LEFTUPBACK, RIGHTUPBACK, RIGHTUPFRONT, LEFTUPFRONT, { 0.0f, 1.0f, 0.0f }, cLUB, cRUB, cRUF, cLUF);
    addFace(LEFTUPFRONT, RIGHTUPFRONT, RIGHTDOWNFRONT, LEFTDOWNFRONT, { 0.0f, 0.0f, -1.0f }, cLUF, cRUF, cRDF, cLDF);
    addFace(LEFTDOWNFRONT, RIGHTDOWNFRONT, RIGHTDOWNBACK, LEFTDOWNBACK, { 0.0f, -1.0f, 0.0f }, cLDF, cRDF, cRDB, cLDB);
    addFace(RIGHTUPBACK, LEFTUPBACK, LEFTDOWNBACK, RIGHTDOWNBACK, { 0.0f, 0.0f, 1.0f }, cRUB, cLUB, cLDB, cRDB);
    addFace(LEFTUPBACK, LEFTUPFRONT, LEFTDOWNFRONT, LEFTDOWNBACK, { -1.0f, 0.0f, 0.0f }, cLUB, cLUF, cLDF, cLDB);
    addFace(RIGHTUPFRONT, RIGHTUPBACK, RIGHTDOWNBACK, RIGHTDOWNFRONT, { 1.0f, 0.0f, 0.0f }, cRUF, cRUB, cRDB, cRDF);

    m_vertices = static_cast<UINT>(vertices.size());
    m_indices = static_cast<UINT>(indices.size());

    CreateBoundingBox(vertices.data(), m_vertices, sizeof(Vertex));
    CreateVertexBuffer(device, commandList, vertices, m_vertexBuffer, m_vertexUploadBuffer, m_vertexBufferView);
    CreateIndexBuffer(device, commandList, indices, m_indexBuffer, m_indexUploadBuffer, m_indexBufferView);
}

PlaneMesh::PlaneMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    vector<Vertex> vertices;

    float w = 15.0f;
    float d = 15.0f;
    float y = 0.0f;

    XMFLOAT3 leftTop = { -w, y, +d };
    XMFLOAT3 rightTop = { +w, y, +d };
    XMFLOAT3 leftBottom = { -w, y, -d };
    XMFLOAT3 rightBottom = { +w, y, -d };

    XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };
    XMFLOAT4 planeColor = { 0.4f, 0.6f, 0.4f, 1.0f };

    vertices.push_back({ leftTop, normal, planeColor });
    vertices.push_back({ rightTop, normal, planeColor });
    vertices.push_back({ rightBottom, normal, planeColor });

    vertices.push_back({ leftTop, normal, planeColor });
    vertices.push_back({ rightBottom, normal, planeColor });
    vertices.push_back({ leftBottom, normal, planeColor });

    m_vertices = static_cast<UINT>(vertices.size());
    CreateBoundingBox(vertices.data(), m_vertices, sizeof(Vertex));
    CreateVertexBuffer(device, commandList, vertices, m_vertexBuffer, m_vertexUploadBuffer, m_vertexBufferView);
}

CapsuleIndexMesh::CapsuleIndexMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, float radius, float height, int segments)
{
    vector<Vertex> vertices;
    vector<UINT> indices;

    int rings = segments / 2;
    float halfHeight = height * 0.5f;
    XMFLOAT4 color = { 0.8f, 0.2f, 0.2f, 1.0f };

    for (int i = 0; i <= rings; ++i)
    {
        float theta = XM_PIDIV2 - (static_cast<float>(i) / rings) * XM_PIDIV2;
        for (int j = 0; j <= segments; ++j)
        {
            float phi = (static_cast<float>(j) / segments) * XM_2PI;
            XMFLOAT3 normal{ cosf(theta) * cosf(phi), sinf(theta), cosf(theta) * sinf(phi) };
            XMFLOAT3 pos{ radius * normal.x, radius * normal.y + halfHeight, radius * normal.z };
            vertices.push_back({ pos, normal, color });
        }
    }

    for (int i = 0; i <= rings; ++i)
    {
        float theta = -(static_cast<float>(i) / rings) * XM_PIDIV2;
        for (int j = 0; j <= segments; ++j)
        {
            float phi = (static_cast<float>(j) / segments) * XM_2PI;
            XMFLOAT3 normal{ cosf(theta) * cosf(phi), sinf(theta), cosf(theta) * sinf(phi) };
            XMFLOAT3 pos{ radius * normal.x, radius * normal.y - halfHeight, radius * normal.z };
            vertices.push_back({ pos, normal, color });
        }
    }

    int ringVertexCount = segments + 1;
    for (int i = 0; i < (rings * 2 + 1); ++i)
    {
        for (int j = 0; j < segments; ++j)
        {
            indices.push_back(i * ringVertexCount + j);
            indices.push_back(i * ringVertexCount + j + 1);
            indices.push_back((i + 1) * ringVertexCount + j);

            indices.push_back(i * ringVertexCount + j + 1);
            indices.push_back((i + 1) * ringVertexCount + j + 1);
            indices.push_back((i + 1) * ringVertexCount + j);
        }
    }

    m_vertices = static_cast<UINT>(vertices.size());
    m_indices = static_cast<UINT>(indices.size());

    CreateBoundingBox(vertices.data(), m_vertices, sizeof(Vertex));
    CreateVertexBuffer(device, commandList, vertices, m_vertexBuffer, m_vertexUploadBuffer, m_vertexBufferView);
    CreateIndexBuffer(device, commandList, indices, m_indexBuffer, m_indexUploadBuffer, m_indexBufferView);
}

FirstPersonGunMesh::FirstPersonGunMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    vector<Vertex> vertices;
    vertices.reserve(36 * 4);

    auto addBox = [&vertices](XMFLOAT3 minCorner, XMFLOAT3 maxCorner, XMFLOAT4 color)
    {
        XMFLOAT3 ldf{ minCorner.x, minCorner.y, minCorner.z };
        XMFLOAT3 ldb{ minCorner.x, minCorner.y, maxCorner.z };
        XMFLOAT3 luf{ minCorner.x, maxCorner.y, minCorner.z };
        XMFLOAT3 lub{ minCorner.x, maxCorner.y, maxCorner.z };
        XMFLOAT3 rdf{ maxCorner.x, minCorner.y, minCorner.z };
        XMFLOAT3 rdb{ maxCorner.x, minCorner.y, maxCorner.z };
        XMFLOAT3 ruf{ maxCorner.x, maxCorner.y, minCorner.z };
        XMFLOAT3 rub{ maxCorner.x, maxCorner.y, maxCorner.z };

        auto addFace = [&vertices, color](const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& c, const XMFLOAT3& d, XMFLOAT3 normal)
        {
            vertices.push_back({ a, normal, color });
            vertices.push_back({ b, normal, color });
            vertices.push_back({ c, normal, color });
            vertices.push_back({ a, normal, color });
            vertices.push_back({ c, normal, color });
            vertices.push_back({ d, normal, color });
        };

        addFace(luf, ruf, rdf, ldf, { 0.0f, 0.0f, -1.0f });
        addFace(lub, rub, ruf, luf, { 0.0f, 1.0f, 0.0f });
        addFace(ldb, rdb, rub, lub, { 0.0f, 0.0f, 1.0f });
        addFace(ldf, rdf, rdb, ldb, { 0.0f, -1.0f, 0.0f });
        addFace(lub, luf, ldf, ldb, { -1.0f, 0.0f, 0.0f });
        addFace(ruf, rub, rdb, rdf, { 1.0f, 0.0f, 0.0f });
    };

    addBox({ 0.26f, -0.34f, 0.55f }, { 0.54f, -0.22f, 1.10f }, { 0.16f, 0.17f, 0.18f, 1.0f });
    addBox({ 0.34f, -0.26f, 1.00f }, { 0.46f, -0.18f, 1.35f }, { 0.07f, 0.08f, 0.09f, 1.0f });
    addBox({ 0.34f, -0.58f, 0.58f }, { 0.46f, -0.32f, 0.78f }, { 0.09f, 0.08f, 0.07f, 1.0f });
    addBox({ 0.36f, -0.19f, 0.72f }, { 0.44f, -0.14f, 0.92f }, { 0.05f, 0.18f, 0.15f, 1.0f });

    m_vertices = static_cast<UINT>(vertices.size());
    CreateBoundingBox(vertices.data(), m_vertices, sizeof(Vertex));
    CreateVertexBuffer(device, commandList, vertices, m_vertexBuffer, m_vertexUploadBuffer, m_vertexBufferView);
}

CrosshairMesh::CrosshairMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    vector<Vertex> vertices;
    vertices.reserve(12);

    constexpr float distance = 0.60f;
    constexpr float length = 0.026f;
    constexpr float thickness = 0.0025f;
    const XMFLOAT3 normal{ 0.0f, 0.0f, -1.0f };
    const XMFLOAT4 color{ 0.92f, 0.98f, 1.0f, 1.0f };

    auto addQuad = [&vertices, normal, color, distance](float left, float top, float right, float bottom)
    {
        XMFLOAT3 lt{ left, top, distance };
        XMFLOAT3 rt{ right, top, distance };
        XMFLOAT3 rb{ right, bottom, distance };
        XMFLOAT3 lb{ left, bottom, distance };

        vertices.push_back({ lt, normal, color });
        vertices.push_back({ rt, normal, color });
        vertices.push_back({ rb, normal, color });
        vertices.push_back({ lt, normal, color });
        vertices.push_back({ rb, normal, color });
        vertices.push_back({ lb, normal, color });
    };

    addQuad(-length, thickness, length, -thickness);
    addQuad(-thickness, length, thickness, -length);

    m_vertices = static_cast<UINT>(vertices.size());
    CreateBoundingBox(vertices.data(), m_vertices, sizeof(Vertex));
    CreateVertexBuffer(device, commandList, vertices, m_vertexBuffer, m_vertexUploadBuffer, m_vertexBufferView);
}
