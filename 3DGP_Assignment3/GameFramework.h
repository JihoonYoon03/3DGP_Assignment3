#pragma once

// 과제 3 게임 전체를 관리하는 Direct3D 12 게임 프레임워크 헤더입니다.
#include "pch.h"
#include "Collision.h"

// Direct3D 12 장치와 DXGI 스왑 체인을 사용합니다.
#include <d3d12.h>
#include <dxgi1_6.h>

// 카메라, 월드 행렬, 3D 벡터 계산에 DirectXMath를 사용합니다.
#include <DirectXMath.h>

// COM 객체 수명 관리는 ComPtr로 처리합니다.
#include <wrl/client.h>

// 렌더 항목, 메시, 입력 상태, 게임 오브젝트 목록을 보관합니다.
#include <array>
#include <cstdint>
#include <string>
#include <vector>

// D3D12 초기화, 씬 전환, 입력, 렌더링을 묶는 과제용 게임 클래스입니다.
class AssignmentGame final
{
public:
    // 창 생성 뒤 Initialize에서 GPU 리소스를 준비합니다.
    AssignmentGame() = default;

    // 종료 시 GPU 작업을 기다리고 Win32 핸들을 정리합니다.
    ~AssignmentGame();

    // D3D12 장치, 스왑 체인, 파이프라인, 기본 메시를 초기화합니다.
    void Initialize(HWND hwnd, UINT width, UINT height);

    // 한 프레임의 입력 갱신, 게임 갱신, 렌더링을 수행합니다.
    void Tick(float deltaSeconds);

    // 창 크기 변경에 맞춰 렌더 타깃과 깊이 버퍼를 다시 만듭니다.
    void OnResize(UINT width, UINT height);

    // 키보드 입력 상태를 갱신합니다.
    void OnKeyDown(WPARAM key);
    void OnKeyUp(WPARAM key);

    // 마우스 이동과 클릭을 씬 상태에 맞게 처리합니다.
    void OnMouseMove(int x, int y);
    void OnMouseDown(int x, int y);
    void OnRightMouseDown(int x, int y);

private:
    // 더블 버퍼링을 사용합니다.
    static constexpr UINT FrameCount = 2;

    // 한 프레임에서 업로드할 수 있는 최대 3D 도형 수입니다.
    static constexpr UINT MaxDrawItems = 8192;
    static constexpr std::size_t MaxMissileTrailParticles = 384;

    // D3D12 상수 버퍼는 256바이트 정렬이 필요합니다.
    static constexpr UINT ConstantBufferAlignment = 256;

    // 과제에서 요구한 시작 화면, 메뉴 화면, Level-1 화면입니다.
    enum class SceneMode
    {
        Start,
        Menu,
        Level1
    };

    // 현재 프레임워크가 보유한 기본 메시 종류입니다.
    enum class MeshKind : std::size_t
    {
        Cube = 0,
        Terrain = 1,
        Apache = 2,
        Count = 3
    };

    // 모든 3D 도형은 위치, 색상, 노멀 정점으로 표현합니다.
    struct Vertex
    {
        DirectX::XMFLOAT3 position{};
        DirectX::XMFLOAT4 color{};
        DirectX::XMFLOAT3 normal{ 0.0f, 1.0f, 0.0f };
    };

    // 객체별 행렬, 색상, 조명 파라미터입니다.
    struct ObjectConstants
    {
        DirectX::XMFLOAT4X4 world{};
        DirectX::XMFLOAT4X4 worldInverseTranspose{};
        DirectX::XMFLOAT4X4 worldViewProjection{};
        DirectX::XMFLOAT4 color{};
        DirectX::XMFLOAT4 cameraPosition{};
        DirectX::XMFLOAT4 lightDirection{};
        DirectX::XMFLOAT4 ambientColor{};
        DirectX::XMFLOAT4 diffuseColor{};
        DirectX::XMFLOAT4 specularColor{};
        DirectX::XMFLOAT4 lightingOptions{};
    };

    // 정점/인덱스 버퍼와 D3D12 뷰 정보를 묶은 메시 리소스입니다.
    struct MeshResource
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
        D3D12_INDEX_BUFFER_VIEW indexBufferView{};
        UINT indexCount = 0;
        D3D12_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    };

    // Apache 파일에서 분리해 읽은 파트 메시입니다.
    struct ApacheMeshPart
    {
        MeshResource mesh;
        DirectX::XMFLOAT3 center{};
        DirectX::XMFLOAT3 extents{};
        std::string name;
        bool mainRotor = false;
        bool tailRotor = false;
    };

    // 한 번의 DrawIndexedInstanced 호출에 필요한 렌더 항목입니다.
    struct DrawItem
    {
        MeshKind mesh = MeshKind::Cube;
        std::size_t meshPartIndex = 0;
        DirectX::XMFLOAT4X4 world{};
        DirectX::XMFLOAT4 color{};
    };

    // 총알은 위치, 속도, 남은 수명을 가집니다.
    struct Bullet
    {
        DirectX::XMFLOAT3 position{};
        DirectX::XMFLOAT3 velocity{};
        float lifeSeconds = 0.0f;
        float homingDelaySeconds = 0.0f;
        float trailSpawnAccumulator = 0.0f;
        bool homing = false;
        int targetIndex = -1;
    };

    // 임시 표적은 현재 박스로 표현하며, 나중에 모델 로더로 교체할 수 있습니다.
    struct Target
    {
        DirectX::XMFLOAT3 position{};
        bool active = true;
    };

    // 폭발 효과는 일정 시간 동안 흩어지는 큐브 파티클 묶음입니다.
    struct Explosion
    {
        DirectX::XMFLOAT3 position{};
        DirectX::XMFLOAT4 color{};
        float elapsedSeconds = 0.0f;
        float durationSeconds = 0.0f;
        float radius = 0.0f;
    };

    // 미사일 트레일은 고정 슬롯을 재사용하는 간단한 연기 입자입니다.
    struct MissileTrailParticle
    {
        DirectX::XMFLOAT3 position{};
        DirectX::XMFLOAT3 velocity{};
        float elapsedSeconds = 0.0f;
        float durationSeconds = 0.0f;
        float startSize = 0.0f;
        bool active = false;
    };

    // 메뉴 항목은 표시 문자열과 3D 배치 y값을 보관합니다.
    struct MenuEntry
    {
        std::wstring label;
        float y = 0.0f;
    };

    // GameFramework.cpp: D3D12 장치와 프레임 루프를 관리합니다.
    void CreateDeviceResources();
    void CreateWindowSizeDependentResources();
    void CreatePipelineState();
    void Render();
    void PopulateCommandList(const DirectX::XMMATRIX& viewProjection);
    void UploadObjectConstants(const DirectX::XMMATRIX& viewProjection);
    void WaitForGpu();
    void FlushCommandQueue();
    static UINT AlignConstantBufferSize(UINT byteSize);
    static D3D12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE type);
    static D3D12_RESOURCE_DESC BufferResourceDesc(UINT64 byteSize);
    static D3D12_RESOURCE_BARRIER TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
    static void ThrowIfFailed(HRESULT hr);

    // Mesh.cpp / Maps.cpp: 기본 메시와 지형 메시를 생성합니다.
    void CreateMeshResources();
    void CreateMesh(MeshResource& mesh, const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, D3D12_PRIMITIVE_TOPOLOGY topology);
    bool CreateApacheMesh();
    bool LoadApacheModelFile(const std::wstring& filePath);
    DirectX::XMMATRIX ApacheModelWorldMatrix() const;

    // GameObject.cpp: 게임 상태 갱신과 3D 오브젝트 배치를 담당합니다.
    void Update(float deltaSeconds);
    void UpdateStart(float deltaSeconds);
    void UpdateLevel(float deltaSeconds);
    void UpdateAimRay();
    void FireBulletAtAim();
    void BuildDrawItems();
    void BuildStartScene();
    void BuildMenuScene();
    void BuildLevelScene();
    void AddHelicopter();
    void AddTargets();
    void AddMissileTrails();
    void AddBullets();
    void AddExplosions();
    void AddCrosshair();
    void AddLockOnIndicator();
    void AddBox(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& size, const DirectX::XMFLOAT4& color, float yaw = 0.0f, float pitch = 0.0f, float roll = 0.0f);
    void AddBoxWithWorld(const DirectX::XMMATRIX& world, const DirectX::XMFLOAT4& color);
    void AddText3D(const std::wstring& text, const DirectX::XMFLOAT3& origin, float unitSize, float depth, const DirectX::XMFLOAT4& color, float yaw = 0.0f, bool centered = true, float glyphSpacing = 0.25f);
    void AddExplodingText3D(const std::wstring& text, const DirectX::XMFLOAT3& origin, float unitSize, float depth, const DirectX::XMFLOAT4& color, float yaw, float explosionTime);
    void SpawnMissileTrail(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& missileDirection);
    void SpawnExplosion(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color, float radius);
    bool HitStartName(int x, int y) const;
    int HitMenuEntry(int x, int y) const;
    void ResetLevel();
    void SetLevelCursorCapture(bool enabled);
    bool IsTargetIndexValid(int targetIndex) const;
    float ScreenConstantScaleAt(const DirectX::XMFLOAT3& position, float scalePerMeter) const;
    float TerrainHeightAt(float worldX, float worldZ) const;
    bool RaycastTerrain(const Collision::Ray& ray, float maxDistance, Collision::HitResult& hit, float heightOffset = 0.0f) const;
    DirectX::XMFLOAT3 LevelCameraPosition() const;
    DirectX::XMFLOAT3 ForwardDirection() const;
    DirectX::XMFLOAT3 MuzzlePosition() const;

    // Camera.cpp: 씬별 카메라 행렬을 계산합니다.
    DirectX::XMMATRIX ProjectionMatrix() const;
    DirectX::XMMATRIX LevelViewMatrix() const;

    // Direct3D가 렌더링할 Win32 창 핸들입니다.
    HWND m_hwnd = nullptr;

    // 현재 백 버퍼 크기입니다.
    UINT m_width = 1280;
    UINT m_height = 720;

    // D3D12 핵심 객체입니다.
    Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencil;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;

    // CPU에서 매 프레임 상수 버퍼를 갱신하기 위한 매핑 주소입니다.
    std::uint8_t* m_mappedConstantBuffer = nullptr;

    // 디스크립터 크기와 현재 백 버퍼 인덱스입니다.
    UINT m_rtvDescriptorSize = 0;
    UINT m_dsvDescriptorSize = 0;
    UINT m_frameIndex = 0;

    // GPU 동기화에 사용하는 Fence와 이벤트입니다.
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue = 0;
    HANDLE m_fenceEvent = nullptr;

    // 현재 렌더링 영역입니다.
    D3D12_VIEWPORT m_viewport{};
    D3D12_RECT m_scissorRect{};

    // 큐브, 지형, Apache 모델 메시 리소스입니다.
    std::array<MeshResource, static_cast<std::size_t>(MeshKind::Count)> m_meshes{};
    std::vector<ApacheMeshPart> m_apacheParts;
    bool m_apacheModelLoaded = false;

    // 매 프레임 새로 구성되는 렌더 항목 목록입니다.
    std::vector<DrawItem> m_drawItems;

    // 하이트맵 지형 충돌과 높이 질의를 위한 CPU 높이 데이터입니다.
    std::vector<float> m_terrainHeights;
    UINT m_terrainWidth = 0;
    UINT m_terrainLength = 0;
    float m_terrainCellX = 1.0f;
    float m_terrainCellZ = 1.0f;
    float m_terrainHalfWidth = 0.0f;
    float m_terrainHalfLength = 0.0f;

    // 키보드와 마우스 입력 상태입니다.
    std::array<bool, 256> m_keyDown{};
    int m_mouseX = 0;
    int m_mouseY = 0;
    int m_lastMouseX = 0;
    int m_lastMouseY = 0;
    bool m_hasLastMousePosition = false;
    bool m_cursorCaptured = false;

    // 현재 씬과 시작 화면 전환 애니메이션 상태입니다.
    SceneMode m_scene = SceneMode::Start;
    float m_totalTime = 0.0f;
    bool m_nameExploding = false;
    float m_nameExplosionTime = 0.0f;
    float m_nameExplosionYaw = 0.0f;

    // PDF 요구사항의 메뉴 항목 목록입니다.
    std::vector<MenuEntry> m_menuEntries;

    // Level-1 헬리콥터와 슈팅 상태입니다.
    DirectX::XMFLOAT3 m_helicopterPosition{ 0.0f, 1.3f, -8.0f };
    float m_helicopterYaw = 0.0f;
    float m_helicopterPitch = 0.0f;
    float m_rotorAngle = 0.0f;
    float m_shotCooldown = 0.0f;
    std::vector<Bullet> m_bullets;
    std::vector<Target> m_targets;
    std::vector<Explosion> m_explosions;
    std::array<MissileTrailParticle, MaxMissileTrailParticles> m_missileTrails{};
    std::size_t m_nextMissileTrailIndex = 0;
    int m_lockedTargetIndex = -1;
    bool m_lockPinned = false;

    // 총구에서 쏜 조준 광선의 결과 위치입니다.
    bool m_crosshairValid = false;
    DirectX::XMFLOAT3 m_crosshairPosition{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_aimDirection{ 0.0f, 0.0f, 1.0f };
};
