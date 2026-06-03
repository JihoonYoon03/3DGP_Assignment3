#include "GameFramework.h"

#include <cstring>


void AssignmentGame::CreateMesh(MeshResource& mesh, const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, D3D12_PRIMITIVE_TOPOLOGY topology)
{
    // 정점 버퍼와 인덱스 버퍼는 작은 과제 규모에 맞춰 업로드 힙에 직접 보관합니다.
    const UINT vertexBufferSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));
    const UINT indexBufferSize = static_cast<UINT>(indices.size() * sizeof(std::uint32_t));

    const D3D12_HEAP_PROPERTIES uploadHeap = HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    const D3D12_RESOURCE_DESC vertexDesc = BufferResourceDesc(vertexBufferSize);
    const D3D12_RESOURCE_DESC indexDesc = BufferResourceDesc(indexBufferSize);

    ThrowIfFailed(m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &vertexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mesh.vertexBuffer)));
    ThrowIfFailed(m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &indexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mesh.indexBuffer)));

    void* mappedVertices = nullptr;
    ThrowIfFailed(mesh.vertexBuffer->Map(0, nullptr, &mappedVertices));
    std::memcpy(mappedVertices, vertices.data(), vertexBufferSize);
    mesh.vertexBuffer->Unmap(0, nullptr);

    void* mappedIndices = nullptr;
    ThrowIfFailed(mesh.indexBuffer->Map(0, nullptr, &mappedIndices));
    std::memcpy(mappedIndices, indices.data(), indexBufferSize);
    mesh.indexBuffer->Unmap(0, nullptr);

    mesh.vertexBufferView.BufferLocation = mesh.vertexBuffer->GetGPUVirtualAddress();
    mesh.vertexBufferView.StrideInBytes = sizeof(Vertex);
    mesh.vertexBufferView.SizeInBytes = vertexBufferSize;
    mesh.indexBufferView.BufferLocation = mesh.indexBuffer->GetGPUVirtualAddress();
    mesh.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    mesh.indexBufferView.SizeInBytes = indexBufferSize;
    mesh.indexCount = static_cast<UINT>(indices.size());
    mesh.topology = topology;
}
