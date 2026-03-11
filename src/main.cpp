#include "render/OverlayWindow.h"
#include "render/GlowRenderer.h"
#include <windows.h>
#include <chrono>
#include <thread>

using namespace EdgeGlow;

// Global flag for keyboard hook
bool g_running = true;

// Low-level keyboard hook to detect ESC
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
    {
        KBDLLHOOKSTRUCT *pKeyboard = (KBDLLHOOKSTRUCT *)lParam;
        if (pKeyboard->vkCode == VK_ESCAPE)
        {
            g_running = false; // Signal main loop to exit
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

/**
 * @brief Main application entry point.
 *
 * Phase 1 Functionality:
 * - Creates a transparent overlay window
 * - Initializes Direct2D rendering
 * - Renders a static cyan glow on the top edge
 * - Runs at ~60 FPS
 */
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd)
{
    // Unreferenced parameters
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    // Initialize COM (required for Direct2D)
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to initialize COM", L"Error", MB_ICONERROR);
        return -1;
    }

    // Create overlay window
    OverlayWindow window;
    if (!window.Initialize())
    {
        MessageBox(nullptr, L"Failed to create overlay window", L"Error", MB_ICONERROR);
        CoUninitialize();
        return -1;
    }

    // Create renderer
    GlowRenderer renderer;
    if (!renderer.Initialize(window.GetHandle()))
    {
        MessageBox(nullptr, L"Failed to initialize Direct2D renderer", L"Error", MB_ICONERROR);
        CoUninitialize();
        return -1;
    }

    // Show the window
    window.Show();

    // Install keyboard hook to detect ESC (since window is click-through)
    HHOOK hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, hInstance, 0);
    if (!hKeyboardHook)
    {
        MessageBox(nullptr, L"Failed to install keyboard hook", L"Warning", MB_ICONWARNING);
    }

    // Phase 1 test message
    MessageBox(
        nullptr,
        L"EdgeGlow Phase 1 Test\n\n"
        L"You should see a cyan glow at the top of your screen.\n"
        L"The rest of the screen should be TRANSPARENT (you can see your desktop).\n\n"
        L"Press OK to start rendering.\n"
        L"Press ESC anywhere to exit.",
        L"EdgeGlow - Phase 1",
        MB_ICONINFORMATION);

    // Main render loop
    MSG msg = {};

    // Target 60 FPS
    constexpr int TARGET_FPS = 60;
    constexpr auto FRAME_TIME = std::chrono::milliseconds(1000 / TARGET_FPS);

    while (g_running)
    {
        auto frameStart = std::chrono::steady_clock::now();

        // Process Windows messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                g_running = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!g_running)
            break;

        // Render frame
        renderer.Render();

        // Frame rate limiting
        auto frameEnd = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);

        if (elapsed < FRAME_TIME)
        {
            std::this_thread::sleep_for(FRAME_TIME - elapsed);
        }
    }

    // Cleanup
    if (hKeyboardHook)
    {
        UnhookWindowsHookEx(hKeyboardHook);
    }
    renderer.Cleanup();
    CoUninitialize();

    return 0;
}
