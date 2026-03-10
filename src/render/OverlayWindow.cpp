#include "OverlayWindow.h"
#include <dwmapi.h>
#include <stdexcept>

#pragma comment(lib, "dwmapi.lib")

namespace EdgeGlow {

OverlayWindow::OverlayWindow()
    : m_hwnd(nullptr)
    , m_hInstance(GetModuleHandle(nullptr))
    , m_width(0)
    , m_height(0)
    , m_visible(false)
{
}

OverlayWindow::~OverlayWindow() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
    }
}

bool OverlayWindow::Initialize() {
    // Get primary monitor dimensions
    m_width = GetSystemMetrics(SM_CXSCREEN);
    m_height = GetSystemMetrics(SM_CYSCREEN);

    // Register window class
    if (!RegisterWindowClass()) {
        return false;
    }

    // Create the window
    if (!CreateOverlayWindow()) {
        return false;
    }

    return true;
}

bool OverlayWindow::RegisterWindowClass() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = OverlayWindow::WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;

    // Register the class
    if (!RegisterClassEx(&wc)) {
        // Check if already registered (not an error)
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }
    }

    return true;
}

bool OverlayWindow::CreateOverlayWindow() {
    // Extended window styles for overlay behavior
    DWORD exStyle = WS_EX_TOPMOST          // Always on top
                  | WS_EX_TRANSPARENT      // Click-through
                  | WS_EX_LAYERED          // Required for transparency + click-through with DWM
                  | WS_EX_NOACTIVATE;      // Don't activate when shown

    // Window style - borderless popup
    DWORD style = WS_POPUP | WS_VISIBLE;

    // Create the window
    m_hwnd = CreateWindowEx(
        exStyle,
        CLASS_NAME,
        L"EdgeGlow Overlay",
        style,
        0, 0,                   // Position: top-left corner
        m_width, m_height,      // Size: full screen
        nullptr,                // No parent
        nullptr,                // No menu
        m_hInstance,
        this                    // Pass 'this' pointer to WM_CREATE
    );

    if (!m_hwnd) {
        return false;
    }

    // Set layered window attributes for proper blending with DWM
    SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);

    // Extend frame into client area for transparency
    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(m_hwnd, &margins);

    return true;
}

void OverlayWindow::Show() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
        m_visible = true;
    }
}

void OverlayWindow::Hide() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_HIDE);
        m_visible = false;
    }
}

LRESULT CALLBACK OverlayWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    OverlayWindow* window = nullptr;

    if (msg == WM_CREATE) {
        // Retrieve the 'this' pointer passed to CreateWindowEx
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = reinterpret_cast<OverlayWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        // Retrieve the stored 'this' pointer
        window = reinterpret_cast<OverlayWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window) {
        return window->HandleMessage(msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT OverlayWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            // Let Direct2D handle painting (we'll implement this in GlowRenderer)
            ValidateRect(m_hwnd, nullptr);
            return 0;
        }

        case WM_ERASEBKGND:
            // Prevent background erasure (Direct2D will handle everything)
            return 1;

        case WM_NCHITTEST:
            // Make the window click-through by returning HTTRANSPARENT
            // This allows mouse events to pass through to windows beneath
            return HTTRANSPARENT;

        case WM_MOUSEACTIVATE:
            // Prevent the window from being activated by mouse clicks
            return MA_NOACTIVATE;

        default:
            return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

} // namespace EdgeGlow
