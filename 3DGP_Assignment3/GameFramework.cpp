#include "GameFramework.h"

#include <algorithm>


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

    // PDF 메뉴 항목 순서를 유지하되, 항목 사이 세로 간격을 넓게 둡니다.
    m_menuEntries =
    {
        { L"TUTORIAL", 1.65f },
        { L"LEVEL-1", 0.90f },
        { L"LEVEL-2", 0.15f },
        { L"LEVEL-3", -0.60f },
        { L"START", -1.35f },
        { L"END", -2.10f }
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
            m_nameExplosionYaw = m_totalTime * 1.7f;
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

void AssignmentGame::OnRightMouseDown(int, int)
{
    // Level-1에서만 우클릭으로 현재 락온 대상을 고정하거나 고정을 해제합니다.
    if (m_scene != SceneMode::Level1)
    {
        return;
    }

    if (m_lockPinned)
    {
        m_lockPinned = false;
        m_lockedTargetIndex = -1;
        UpdateAimRay();
        return;
    }

    UpdateAimRay();
    if (IsTargetIndexValid(m_lockedTargetIndex))
    {
        m_lockPinned = true;
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
