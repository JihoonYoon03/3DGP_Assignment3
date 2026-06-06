// 3DGP_Assignment3.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "3DGP_Assignment3.h"
#include "GameFramework.h"

// 마우스 좌표 추출 매크로를 사용합니다.
#include <windowsx.h>

// 표준 시간/메모리/예외 처리는 게임 루프와 오류 보고에 사용합니다.
#include <chrono>
#include <memory>
#include <stdexcept>

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HWND g_hWnd = nullptr;                          // D3D12가 렌더링할 주 창입니다.
std::unique_ptr<AssignmentGame> g_game;          // 과제 3 게임 객체입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY3DGPASSIGNMENT3, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // D3D12 렌더러와 게임 상태를 생성합니다.
    try
    {
        RECT clientRect{};
        GetClientRect(g_hWnd, &clientRect);
        g_game = std::make_unique<AssignmentGame>();
        g_game->Initialize(g_hWnd, static_cast<UINT>(clientRect.right - clientRect.left), static_cast<UINT>(clientRect.bottom - clientRect.top));
    }
    catch (const std::exception& e)
    {
        MessageBoxA(g_hWnd, e.what(), "초기화 오류", MB_ICONERROR | MB_OK);
        return FALSE;
    }

    MSG msg{};
    auto previousTime = std::chrono::steady_clock::now();

    // PeekMessage 기반 루프를 사용해 메시지가 없을 때도 매 프레임 렌더링합니다.
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else if (g_game)
        {
            const auto currentTime = std::chrono::steady_clock::now();
            const float deltaSeconds = std::chrono::duration<float>(currentTime - previousTime).count();
            previousTime = currentTime;
            g_game->Tick(deltaSeconds);
        }
    }

    // 창이 닫힌 뒤 GPU 리소스를 정리합니다.
    g_game.reset();

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY3DGPASSIGNMENT3));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = nullptr;
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   // 과제 실행 화면은 16:9 기본 해상도로 생성합니다.
   RECT windowRect{ 0, 0, 1280, 720 };
   AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        // 리소스 메뉴를 사용하지 않지만 기본 명령 처리는 남겨 둡니다.
        return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_SIZE:
        // 창 크기 변경을 D3D12 백 버퍼 재생성으로 전달합니다.
        if (g_game)
        {
            g_game->OnResize(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_KEYDOWN:
        // 반복 입력도 이동에는 유효하므로 게임 객체로 전달합니다.
        if (g_game)
        {
            g_game->OnKeyDown(wParam);
        }
        break;
    case WM_KEYUP:
        // 키를 떼면 이동 상태를 해제합니다.
        if (g_game)
        {
            g_game->OnKeyUp(wParam);
        }
        break;
    case WM_MOUSEMOVE:
        // 마우스 위치는 메뉴 hover와 선택에 사용합니다.
        if (g_game)
        {
            g_game->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;
    case WM_LBUTTONDOWN:
        // 왼쪽 클릭은 시작 이름 또는 메뉴 항목 선택입니다.
        SetCapture(hWnd);
        if (g_game)
        {
            g_game->OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;
    case WM_LBUTTONUP:
        // 클릭 드래그가 끝나면 마우스 캡처를 해제합니다.
        ReleaseCapture();
        break;
    case WM_RBUTTONDOWN:
        // 오른쪽 클릭은 Level-1에서 현재 자동 락온 대상을 고정하거나 해제합니다.
        if (g_game)
        {
            g_game->OnRightMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            // 실제 화면 출력은 D3D12 렌더 루프에서 수행합니다.
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
