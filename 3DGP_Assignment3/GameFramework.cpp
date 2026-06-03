#include "GameFramework.h"

#include <d3dcompiler.h>

#include <algorithm>
#include <array>
#include <cwctype>
#include <cstring>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

using Microsoft::WRL::ComPtr;
using namespace DirectX;


namespace
{
    // 파이 값은 회전 애니메이션과 카메라 계산에 사용합니다.
    constexpr float Pi = 3.1415926535f;

    // 5x7 도트 글리프 하나를 표현하는 타입입니다.
    using GlyphPattern = std::array<std::string_view, 7>;

    // 기본 글리프는 알 수 없는 문자를 물음표 모양의 3D 블록으로 표시합니다.
    const GlyphPattern FallbackGlyph =
    {
        "11110",
        "00001",
        "00001",
        "01110",
        "00100",
        "00000",
        "00100"
    };

    // ASCII와 과제 제목에 필요한 일부 한글 글리프를 5x7 도트로 정의합니다.
    const std::unordered_map<wchar_t, GlyphPattern>& GlyphTable()
    {
        static const std::unordered_map<wchar_t, GlyphPattern> table =
        {
            { L'0', { "11111", "10001", "10011", "10101", "11001", "10001", "11111" } },
            { L'1', { "00100", "01100", "00100", "00100", "00100", "00100", "01110" } },
            { L'2', { "11110", "00001", "00001", "11110", "10000", "10000", "11111" } },
            { L'3', { "11110", "00001", "00001", "01110", "00001", "00001", "11110" } },
            { L'4', { "10010", "10010", "10010", "11111", "00010", "00010", "00010" } },
            { L'5', { "11111", "10000", "10000", "11110", "00001", "00001", "11110" } },
            { L'6', { "01111", "10000", "10000", "11110", "10001", "10001", "01110" } },
            { L'7', { "11111", "00001", "00010", "00100", "01000", "01000", "01000" } },
            { L'8', { "01110", "10001", "10001", "01110", "10001", "10001", "01110" } },
            { L'9', { "01110", "10001", "10001", "01111", "00001", "00001", "11110" } },
            { L'A', { "01110", "10001", "10001", "11111", "10001", "10001", "10001" } },
            { L'B', { "11110", "10001", "10001", "11110", "10001", "10001", "11110" } },
            { L'C', { "01111", "10000", "10000", "10000", "10000", "10000", "01111" } },
            { L'D', { "11110", "10001", "10001", "10001", "10001", "10001", "11110" } },
            { L'E', { "11111", "10000", "10000", "11110", "10000", "10000", "11111" } },
            { L'F', { "11111", "10000", "10000", "11110", "10000", "10000", "10000" } },
            { L'G', { "01111", "10000", "10000", "10011", "10001", "10001", "01111" } },
            { L'H', { "10001", "10001", "10001", "11111", "10001", "10001", "10001" } },
            { L'I', { "11111", "00100", "00100", "00100", "00100", "00100", "11111" } },
            { L'J', { "00111", "00010", "00010", "00010", "10010", "10010", "01100" } },
            { L'K', { "10001", "10010", "10100", "11000", "10100", "10010", "10001" } },
            { L'L', { "10000", "10000", "10000", "10000", "10000", "10000", "11111" } },
            { L'M', { "10001", "11011", "10101", "10101", "10001", "10001", "10001" } },
            { L'N', { "10001", "11001", "10101", "10011", "10001", "10001", "10001" } },
            { L'O', { "01110", "10001", "10001", "10001", "10001", "10001", "01110" } },
            { L'P', { "11110", "10001", "10001", "11110", "10000", "10000", "10000" } },
            { L'Q', { "01110", "10001", "10001", "10001", "10101", "10010", "01101" } },
            { L'R', { "11110", "10001", "10001", "11110", "10100", "10010", "10001" } },
            { L'S', { "01111", "10000", "10000", "01110", "00001", "00001", "11110" } },
            { L'T', { "11111", "00100", "00100", "00100", "00100", "00100", "00100" } },
            { L'U', { "10001", "10001", "10001", "10001", "10001", "10001", "01110" } },
            { L'V', { "10001", "10001", "10001", "10001", "10001", "01010", "00100" } },
            { L'W', { "10001", "10001", "10001", "10101", "10101", "10101", "01010" } },
            { L'X', { "10001", "10001", "01010", "00100", "01010", "10001", "10001" } },
            { L'Y', { "10001", "10001", "01010", "00100", "00100", "00100", "00100" } },
            { L'Z', { "11111", "00001", "00010", "00100", "01000", "10000", "11111" } },
            { L'-', { "00000", "00000", "00000", "11111", "00000", "00000", "00000" } },
            { L':', { "00000", "00100", "00100", "00000", "00100", "00100", "00000" } },
            { L'게', { "11110", "01010", "11110", "01000", "11111", "01001", "01001" } },
            { L'임', { "11111", "00100", "00100", "11111", "10001", "10001", "11111" } },
            { L'프', { "11111", "10001", "11111", "00000", "11111", "00100", "00100" } },
            { L'로', { "11111", "00001", "11111", "10000", "11111", "00000", "11111" } },
            { L'그', { "11111", "10000", "10000", "11111", "00000", "11111", "00100" } },
            { L'래', { "11110", "01010", "11110", "01000", "11111", "01001", "01001" } },
            { L'밍', { "10001", "11111", "10101", "11111", "10001", "10001", "11111" } },
            { L'이', { "11111", "00100", "00100", "00100", "00100", "00100", "11111" } },
            { L'름', { "11111", "10001", "11111", "10000", "11111", "10001", "11111" } }
        };

        return table;
    }

    // 글리프 테이블에서 문자를 찾고 없으면 기본 글리프를 반환합니다.
    const GlyphPattern& FindGlyph(wchar_t ch)
    {
        const auto& table = GlyphTable();
        const auto found = table.find(static_cast<wchar_t>(std::towupper(ch)));
        if (found != table.end())
        {
            return found->second;
        }

        const auto original = table.find(ch);
        if (original != table.end())
        {
            return original->second;
        }

        return FallbackGlyph;
    }

    // 두 3D 벡터를 더합니다.
    XMFLOAT3 AddVector(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return { a.x + b.x, a.y + b.y, a.z + b.z };
    }

    // 3D 벡터에 스칼라를 곱합니다.
    XMFLOAT3 ScaleVector(const XMFLOAT3& v, float scale)
    {
        return { v.x * scale, v.y * scale, v.z * scale };
    }

    // 두 점 사이의 제곱 거리를 계산해 충돌 검사에 사용합니다.
    float DistanceSquared(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        const float dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }

    // 상수 버퍼 크기를 D3D12가 요구하는 256바이트 단위로 올림합니다.
    UINT AlignConstantBufferSize(UINT byteSize)
    {
        constexpr UINT alignment = 256;
        return (byteSize + alignment - 1) & ~(alignment - 1);
    }
}

AssignmentGame::~AssignmentGame()
{
    // 게임 종료 시 숨김/고정된 커서를 반드시 원래 상태로 돌립니다.
    SetLevelCursorCapture(false);

    // GPU가 아직 백 버퍼나 상수 버퍼를 읽는 중일 수 있으므로 종료 전에 기다립니다.
    if (m_device && m_commandQueue && m_fence)
    {
        FlushCommandQueue();
    }

    // 매핑된 업로드 버퍼는 프로세스 종료 전 명시적으로 해제합니다.
    if (m_constantBuffer && m_mappedConstantBuffer)
    {
        m_constantBuffer->Unmap(0, nullptr);
        m_mappedConstantBuffer = nullptr;
    }

    // Fence 이벤트 핸들은 Win32 리소스이므로 직접 닫습니다.
    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }
}

void AssignmentGame::Initialize(HWND hwnd, UINT width, UINT height)
{
    // 창 정보와 기본 해상도를 저장합니다.
    m_hwnd = hwnd;
    m_width = std::max(1u, width);
    m_height = std::max(1u, height);

    // PDF 메뉴 항목 순서를 그대로 둡니다.
    m_menuEntries =
    {
        { L"TUTORIAL", 1.30f },
        { L"LEVEL-1", 0.82f },
        { L"LEVEL-2", 0.34f },
        { L"LEVEL-3", -0.14f },
        { L"START", -0.62f },
        { L"END", -1.10f }
    };

    // 장치, 파이프라인, 메시를 차례로 준비합니다.
    CreateDeviceResources();
    CreateWindowSizeDependentResources();
    CreatePipelineState();
    CreateMeshResources();
    ResetLevel();
}

void AssignmentGame::Tick(float deltaSeconds)
{
    // 프레임 시간이 튀면 물리 이동이 과도해지므로 적당히 제한합니다.
    const float clampedDelta = std::clamp(deltaSeconds, 0.0f, 0.05f);
    m_totalTime += clampedDelta;

    // 게임 로직을 먼저 갱신한 뒤 같은 상태를 렌더링합니다.
    Update(clampedDelta);
    if (m_scene != SceneMode::Level1 && m_cursorCaptured)
    {
        SetLevelCursorCapture(false);
    }
    Render();
}

void AssignmentGame::OnResize(UINT width, UINT height)
{
    // 최소화 상태에서는 0 크기가 들어올 수 있으므로 렌더 버퍼를 만들지 않습니다.
    if (width == 0 || height == 0 || !m_device || !m_swapChain)
    {
        return;
    }

    // 크기 변경 전에 GPU가 기존 버퍼 사용을 끝내도록 기다립니다.
    m_width = width;
    m_height = height;
    FlushCommandQueue();
    CreateWindowSizeDependentResources();
}

void AssignmentGame::OnKeyDown(WPARAM key)
{
    // 키 코드가 배열 범위 안이면 현재 눌림 상태를 기록합니다.
    if (key < m_keyDown.size())
    {
        m_keyDown[key] = true;
    }

    // Level-1에서 Esc를 누르면 PDF 요구사항대로 메뉴 화면으로 복귀합니다.
    if (key == VK_ESCAPE && m_scene == SceneMode::Level1)
    {
        SetLevelCursorCapture(false);
        m_scene = SceneMode::Menu;
    }
}

void AssignmentGame::OnKeyUp(WPARAM key)
{
    // 키 코드가 배열 범위 안이면 눌림 상태를 해제합니다.
    if (key < m_keyDown.size())
    {
        m_keyDown[key] = false;
    }
}

void AssignmentGame::OnMouseMove(int x, int y)
{
    // 화면 좌표는 시작 이름과 메뉴 항목의 선택 판정에 사용합니다.
    m_mouseX = x;
    m_mouseY = y;

    // Level-1에서는 마우스 이동량으로 헬리콥터의 yaw와 pitch를 조작합니다.
    if (m_scene == SceneMode::Level1)
    {
        if (m_hasLastMousePosition)
        {
            const int deltaX = x - m_lastMouseX;
            const int deltaY = y - m_lastMouseY;
            m_helicopterYaw += static_cast<float>(deltaX) * 0.0045f;
            m_helicopterPitch = std::clamp(m_helicopterPitch - static_cast<float>(deltaY) * 0.0035f, -0.55f, 0.45f);
        }

        m_lastMouseX = x;
        m_lastMouseY = y;
        m_hasLastMousePosition = true;

        // 커서를 계속 중앙으로 되돌려 실제 커서가 화면 밖이나 가장자리로 나가지 않게 합니다.
        RECT clientRect{};
        GetClientRect(m_hwnd, &clientRect);
        const int centerX = (clientRect.right - clientRect.left) / 2;
        const int centerY = (clientRect.bottom - clientRect.top) / 2;
        if (x != centerX || y != centerY)
        {
            POINT centerPoint{ centerX, centerY };
            ClientToScreen(m_hwnd, &centerPoint);
            SetCursorPos(centerPoint.x, centerPoint.y);
            m_lastMouseX = centerX;
            m_lastMouseY = centerY;
        }
    }
    else
    {
        m_hasLastMousePosition = false;
    }
}

void AssignmentGame::OnMouseDown(int x, int y)
{
    // Level-1에서는 왼쪽 클릭으로 현재 조준점 방향에 탄환을 발사합니다.
    if (m_scene == SceneMode::Level1)
    {
        FireBulletAtAim();
        return;
    }

    // 시작 화면에서는 회전하는 이름을 클릭하면 폭발 전환을 시작합니다.
    if (m_scene == SceneMode::Start)
    {
        if (!m_nameExploding && HitStartName(x, y))
        {
            m_nameExploding = true;
            m_nameExplosionTime = 0.0f;
        }
        return;
    }

    // 메뉴 화면에서는 Start 또는 Level-1 선택으로 Level-1 씬을 시작합니다.
    if (m_scene == SceneMode::Menu)
    {
        const int entry = HitMenuEntry(x, y);
        if (entry < 0)
        {
            return;
        }

        const std::wstring& label = m_menuEntries[entry].label;
        if (label == L"START" || label == L"LEVEL-1")
        {
            ResetLevel();
            m_scene = SceneMode::Level1;
            SetLevelCursorCapture(true);
        }
        else if (label == L"END")
        {
            PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
        }
    }
}

void AssignmentGame::SetLevelCursorCapture(bool enabled)
{
    // Level-1에서는 마우스 조종을 위해 커서를 숨기고 창 안에 고정합니다.
    if (enabled)
    {
        if (!m_hwnd)
        {
            return;
        }

        RECT clientRect{};
        GetClientRect(m_hwnd, &clientRect);

        POINT topLeft{ clientRect.left, clientRect.top };
        POINT bottomRight{ clientRect.right, clientRect.bottom };
        ClientToScreen(m_hwnd, &topLeft);
        ClientToScreen(m_hwnd, &bottomRight);

        RECT clipRect{ topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
        ClipCursor(&clipRect);

        const int centerX = (clientRect.right - clientRect.left) / 2;
        const int centerY = (clientRect.bottom - clientRect.top) / 2;
        POINT centerPoint{ centerX, centerY };
        ClientToScreen(m_hwnd, &centerPoint);
        SetCursorPos(centerPoint.x, centerPoint.y);

        m_lastMouseX = centerX;
        m_lastMouseY = centerY;
        m_hasLastMousePosition = true;

        if (!m_cursorCaptured)
        {
            while (ShowCursor(FALSE) >= 0)
            {
            }
            m_cursorCaptured = true;
        }
        return;
    }

    // 메뉴나 종료 상태에서는 커서 고정과 숨김을 해제합니다.
    ClipCursor(nullptr);
    if (m_cursorCaptured)
    {
        while (ShowCursor(TRUE) < 0)
        {
        }
    }
    m_cursorCaptured = false;
    m_hasLastMousePosition = false;
}

void AssignmentGame::CreateDeviceResources()
{
    UINT factoryFlags = 0;

#if defined(_DEBUG)
    // Debug 빌드에서는 D3D12 디버그 레이어를 켜서 리소스 오류를 빨리 찾습니다.
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
        factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    // DXGI 팩터리는 어댑터 선택과 스왑 체인 생성의 출발점입니다.
    ThrowIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory)));

    // 하드웨어 어댑터를 우선 사용하고 실패하면 WARP 소프트웨어 어댑터로 대체합니다.
    ComPtr<IDXGIAdapter1> selectedAdapter;
    for (UINT adapterIndex = 0; m_factory->EnumAdapters1(adapterIndex, &selectedAdapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 adapterDesc{};
        selectedAdapter->GetDesc1(&adapterDesc);
        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(selectedAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device))))
        {
            break;
        }
    }

    // 하드웨어 장치 생성에 실패하면 WARP 장치를 생성합니다.
    if (!m_device)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
    }

    // 직접 명령 큐는 그래픽 렌더링 명령을 GPU에 보냅니다.
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // 스왑 체인은 두 개의 백 버퍼를 번갈아 화면에 표시합니다.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(m_factory->CreateSwapChainForHwnd(m_commandQueue.Get(), m_hwnd, &swapChainDesc, nullptr, nullptr, &swapChain));
    ThrowIfFailed(m_factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // RTV/DSV 힙은 렌더 타깃과 깊이 버퍼 디스크립터를 보관합니다.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
    m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    // 명령 할당자와 명령 목록은 매 프레임 렌더 명령 기록에 사용합니다.
    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
    ThrowIfFailed(m_commandList->Close());

    // 모든 객체의 상수 데이터를 하나의 업로드 버퍼에 순차 배치합니다.
    const UINT constantBufferSize = AlignConstantBufferSize(sizeof(ObjectConstants)) * MaxDrawItems;
    const D3D12_HEAP_PROPERTIES uploadHeap = HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    const D3D12_RESOURCE_DESC constantBufferDesc = BufferResourceDesc(constantBufferSize);
    ThrowIfFailed(m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_constantBuffer)));
    ThrowIfFailed(m_constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedConstantBuffer)));

    // Fence는 프레임 끝에서 GPU 완료를 기다리는 데 사용합니다.
    ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
    m_fenceValue = 1;
    m_fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!m_fenceEvent)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
}

void AssignmentGame::CreateWindowSizeDependentResources()
{
    // 기존 백 버퍼와 깊이 버퍼 참조를 해제해야 ResizeBuffers가 성공합니다.
    for (auto& renderTarget : m_renderTargets)
    {
        renderTarget.Reset();
    }
    m_depthStencil.Reset();

    // 스왑 체인을 현재 창 크기에 맞춰 다시 만듭니다.
    ThrowIfFailed(m_swapChain->ResizeBuffers(FrameCount, m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // 백 버퍼마다 렌더 타깃 뷰를 생성합니다.
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT bufferIndex = 0; bufferIndex < FrameCount; ++bufferIndex)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(bufferIndex, IID_PPV_ARGS(&m_renderTargets[bufferIndex])));
        m_device->CreateRenderTargetView(m_renderTargets[bufferIndex].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_rtvDescriptorSize;
    }

    // 깊이 버퍼는 모든 3D 객체의 앞뒤 관계를 올바르게 표시합니다.
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthClearValue.DepthStencil.Depth = 1.0f;
    depthClearValue.DepthStencil.Stencil = 0;

    D3D12_RESOURCE_DESC depthDesc{};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Alignment = 0;
    depthDesc.Width = m_width;
    depthDesc.Height = m_height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    const D3D12_HEAP_PROPERTIES defaultHeap = HeapProperties(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&m_depthStencil)));
    m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    // 뷰포트와 시저 사각형은 백 버퍼 전체를 덮도록 설정합니다.
    m_viewport = { 0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f };
    m_scissorRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
}

void AssignmentGame::CreatePipelineState()
{
    // 루트 시그니처는 객체별 상수 버퍼 하나만 셰이더에 전달합니다.
    D3D12_ROOT_PARAMETER rootParameter{};
    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter.Descriptor.ShaderRegister = 0;
    rootParameter.Descriptor.RegisterSpace = 0;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &rootParameter;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob));
    ThrowIfFailed(m_device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

    // 매우 단순한 색상 셰이더로 모든 큐브와 지형을 그립니다.
    const char* shaderSource = R"(
cbuffer ObjectConstants : register(b0)
{
    float4x4 gWorldViewProjection;
    float4 gColor;
};

struct VertexIn
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VertexOut VSMain(VertexIn input)
{
    VertexOut output;
    output.position = mul(float4(input.position, 1.0f), gWorldViewProjection);
    output.color = input.color * gColor;
    return output;
}

float4 PSMain(VertexOut input) : SV_TARGET
{
    return input.color;
}
)";

    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    // Debug 빌드에서는 셰이더 디버깅 정보를 남깁니다.
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ThrowIfFailed(D3DCompile(shaderSource, std::strlen(shaderSource), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob));
    ThrowIfFailed(D3DCompile(shaderSource, std::strlen(shaderSource), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob));

    // 정점 입력 레이아웃은 위치와 색상 두 요소로 구성됩니다.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 래스터라이저는 지형 스트립의 양면을 모두 볼 수 있게 컬링을 끕니다.
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // 블렌딩은 사용하지 않고 색상을 그대로 씁니다.
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // 깊이 테스트를 켜서 3D 객체가 올바른 순서로 보이게 합니다.
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    // 그래픽 파이프라인 상태 객체는 셰이더와 렌더 상태를 하나로 묶습니다.
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void AssignmentGame::Render()
{
    // 현재 씬의 DrawItem 목록을 만든 뒤 씬에 맞는 카메라를 선택합니다.
    BuildDrawItems();

    const XMMATRIX view =
        (m_scene == SceneMode::Level1)
        ? LevelViewMatrix()
        : XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -8.5f, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    const XMMATRIX viewProjection = view * ProjectionMatrix();

    // 명령 목록을 기록하고 GPU에 제출합니다.
    PopulateCommandList(viewProjection);
    ID3D12CommandList* commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    // 수직 동기화 1로 Present하여 과제 실행 중 화면 찢김을 줄입니다.
    ThrowIfFailed(m_swapChain->Present(1, 0));
    WaitForGpu();
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void AssignmentGame::PopulateCommandList(const XMMATRIX& viewProjection)
{
    // 이전 프레임이 끝난 뒤 같은 할당자를 재사용합니다.
    ThrowIfFailed(m_commandAllocator->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

    // 현재 객체들의 상수 버퍼를 먼저 채웁니다.
    UploadObjectConstants(viewProjection);

    // 현재 백 버퍼를 렌더 타깃 상태로 전환합니다.
    const D3D12_RESOURCE_BARRIER toRenderTarget = TransitionBarrier(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &toRenderTarget);

    // 렌더링 상태와 출력 버퍼를 설정합니다.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += static_cast<SIZE_T>(m_frameIndex) * m_rtvDescriptorSize;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 씬별 배경색은 시작/메뉴와 Level-1을 구분하기 위해 다르게 둡니다.
    const float clearColorStart[4] = { 0.03f, 0.05f, 0.09f, 1.0f };
    const float clearColorLevel[4] = { 0.38f, 0.55f, 0.78f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, (m_scene == SceneMode::Level1) ? clearColorLevel : clearColorStart, 0, nullptr);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 모든 렌더 항목을 메시 종류에 맞게 그립니다.
    const UINT constantBufferStride = AlignConstantBufferSize(sizeof(ObjectConstants));
    for (std::size_t itemIndex = 0; itemIndex < m_drawItems.size(); ++itemIndex)
    {
        const DrawItem& item = m_drawItems[itemIndex];
        const MeshResource& mesh = m_meshes[static_cast<std::size_t>(item.mesh)];
        m_commandList->IASetPrimitiveTopology(mesh.topology);
        m_commandList->IASetVertexBuffers(0, 1, &mesh.vertexBufferView);
        m_commandList->IASetIndexBuffer(&mesh.indexBufferView);
        m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + itemIndex * constantBufferStride);
        m_commandList->DrawIndexedInstanced(mesh.indexCount, 1, 0, 0, 0);
    }

    // 백 버퍼를 다시 Present 상태로 돌려 화면에 표시할 준비를 합니다.
    const D3D12_RESOURCE_BARRIER toPresent = TransitionBarrier(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &toPresent);
    ThrowIfFailed(m_commandList->Close());
}

void AssignmentGame::UploadObjectConstants(const XMMATRIX& viewProjection)
{
    // DrawItem 수가 상수 버퍼 한도를 넘으면 앞쪽 항목만 안전하게 업로드합니다.
    if (m_drawItems.size() > MaxDrawItems)
    {
        m_drawItems.resize(MaxDrawItems);
    }

    const UINT constantBufferStride = AlignConstantBufferSize(sizeof(ObjectConstants));
    for (std::size_t itemIndex = 0; itemIndex < m_drawItems.size(); ++itemIndex)
    {
        const DrawItem& item = m_drawItems[itemIndex];
        const XMMATRIX world = XMLoadFloat4x4(&item.world);
        const XMMATRIX worldViewProjection = XMMatrixTranspose(world * viewProjection);

        ObjectConstants constants{};
        XMStoreFloat4x4(&constants.worldViewProjection, worldViewProjection);
        constants.color = item.color;

        std::memcpy(m_mappedConstantBuffer + itemIndex * constantBufferStride, &constants, sizeof(constants));
    }
}

void AssignmentGame::WaitForGpu()
{
    // GPU에 Fence 값을 신호하고 완료될 때까지 기다립니다.
    const UINT64 fenceToWaitFor = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor));
    ++m_fenceValue;

    if (m_fence->GetCompletedValue() < fenceToWaitFor)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void AssignmentGame::FlushCommandQueue()
{
    // 리소스 재생성 전 모든 GPU 작업이 끝났는지 보장합니다.
    WaitForGpu();
}

D3D12_HEAP_PROPERTIES AssignmentGame::HeapProperties(D3D12_HEAP_TYPE type)
{
    // D3D12 헬퍼 헤더 없이도 힙 속성을 만들기 위한 작은 유틸리티입니다.
    D3D12_HEAP_PROPERTIES properties{};
    properties.Type = type;
    properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    properties.CreationNodeMask = 1;
    properties.VisibleNodeMask = 1;
    return properties;
}

D3D12_RESOURCE_DESC AssignmentGame::BufferResourceDesc(UINT64 byteSize)
{
    // 버퍼 리소스 설명은 정점/인덱스/상수 업로드 버퍼에 공통으로 사용합니다.
    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = byteSize;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    return desc;
}

D3D12_RESOURCE_BARRIER AssignmentGame::TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    // 리소스 상태 전환 배리어를 명확하게 생성합니다.
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = beforeState;
    barrier.Transition.StateAfter = afterState;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    return barrier;
}

void AssignmentGame::ThrowIfFailed(HRESULT hr)
{
    // 실패 HRESULT는 WinMain에서 메시지 박스로 보여 줄 예외로 바꿉니다.
    if (FAILED(hr))
    {
        throw std::runtime_error("Direct3D 12 호출이 실패했습니다.");
    }
}
