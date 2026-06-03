#include "GameFramework.h"

#include <vector>

using namespace DirectX;


void AssignmentGame::CreateMeshResources()
{
    // 큐브는 모든 임시 헬리콥터, 글씨, 탄환, 표적의 기본 도형입니다.
    const XMFLOAT4 white{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::vector<Vertex> cubeVertices =
    {
        { { -0.5f, -0.5f, -0.5f }, white }, { { -0.5f,  0.5f, -0.5f }, white }, { {  0.5f,  0.5f, -0.5f }, white }, { {  0.5f, -0.5f, -0.5f }, white },
        { { -0.5f, -0.5f,  0.5f }, white }, { {  0.5f, -0.5f,  0.5f }, white }, { {  0.5f,  0.5f,  0.5f }, white }, { { -0.5f,  0.5f,  0.5f }, white },
        { { -0.5f,  0.5f, -0.5f }, white }, { { -0.5f,  0.5f,  0.5f }, white }, { {  0.5f,  0.5f,  0.5f }, white }, { {  0.5f,  0.5f, -0.5f }, white },
        { { -0.5f, -0.5f, -0.5f }, white }, { {  0.5f, -0.5f, -0.5f }, white }, { {  0.5f, -0.5f,  0.5f }, white }, { { -0.5f, -0.5f,  0.5f }, white },
        { { -0.5f, -0.5f,  0.5f }, white }, { { -0.5f,  0.5f,  0.5f }, white }, { { -0.5f,  0.5f, -0.5f }, white }, { { -0.5f, -0.5f, -0.5f }, white },
        { {  0.5f, -0.5f, -0.5f }, white }, { {  0.5f,  0.5f, -0.5f }, white }, { {  0.5f,  0.5f,  0.5f }, white }, { {  0.5f, -0.5f,  0.5f }, white }
    };

    std::vector<std::uint32_t> cubeIndices =
    {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };

    CreateMesh(m_meshes[static_cast<std::size_t>(MeshKind::Cube)], cubeVertices, cubeIndices, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 지형은 PPT의 행/열 기반 그리드 방식을 따라 정점과 삼각형 스트립 인덱스를 분리합니다.
    constexpr int terrainWidth = GP_TERRAIN_GRID_VERTEX_COUNT;
    constexpr int terrainLength = GP_TERRAIN_GRID_VERTEX_COUNT;
    constexpr float cellSpacing = GP_TERRAIN_CELL_METERS * GP_WORLD_UNITS_PER_METER;
    const float halfWidth = (terrainWidth - 1) * cellSpacing * 0.5f;
    const float halfLength = (terrainLength - 1) * cellSpacing * 0.5f;

    std::vector<Vertex> terrainVertices;
    terrainVertices.reserve(terrainWidth * terrainLength);

    for (int z = 0; z < terrainLength; ++z)
    {
        for (int x = 0; x < terrainWidth; ++x)
        {
            // 현재는 과제 조건대로 평면 높이 0을 사용하고, 추후 높이맵 함수로 교체할 수 있게 위치 계산을 분리합니다.
            const float worldX = x * cellSpacing - halfWidth;
            const float worldZ = z * cellSpacing - halfLength;
            const float height = 0.0f;
            const float checker = ((x + z) % 2 == 0) ? 0.08f : 0.0f;
            const XMFLOAT4 terrainColor{ 0.16f + checker, 0.45f + 0.25f * (static_cast<float>(z) / terrainLength), 0.18f, 1.0f };
            terrainVertices.push_back({ { worldX, height, worldZ }, terrainColor });
        }
    }

    std::vector<std::uint32_t> terrainIndices;
    terrainIndices.reserve((terrainLength - 1) * terrainWidth * 2);

    for (int z = 0; z < terrainLength - 1; ++z)
    {
        // PPT의 삼각형 스트립 아이디어에 맞춰 한 줄은 좌->우, 다음 줄은 우->좌로 진행합니다.
        if (z % 2 == 0)
        {
            for (int x = 0; x < terrainWidth; ++x)
            {
                terrainIndices.push_back(static_cast<std::uint32_t>(z * terrainWidth + x));
                terrainIndices.push_back(static_cast<std::uint32_t>((z + 1) * terrainWidth + x));
            }
        }
        else
        {
            for (int x = terrainWidth - 1; x >= 0; --x)
            {
                terrainIndices.push_back(static_cast<std::uint32_t>(z * terrainWidth + x));
                terrainIndices.push_back(static_cast<std::uint32_t>((z + 1) * terrainWidth + x));
            }
        }
    }

    CreateMesh(m_meshes[static_cast<std::size_t>(MeshKind::Terrain)], terrainVertices, terrainIndices, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}
