#include "render/OverlayWindow.h"
#include "render/GlowRenderer.h"
#include "audio/AudioCapture.h"
#include <windows.h>
#include <chrono>
#include <thread>
#include <iostream>

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
 * Phase 2 Functionality:
 * - Creates a transparent overlay window
 * - Initializes Direct2D rendering
 * - Captures system audio via WASAPI loopback
 * - Calculates RMS energy and prints to console
 * - Renders a static cyan glow on the top edge
 * - Runs at ~60 FPS
 */
int main(int argc, char *argv[])
{
    // Unreferenced parameters
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    // Initialize COM (required for Direct2D and WASAPI)
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize COM" << std::endl;
        return -1;
    }

    // Create overlay window
    OverlayWindow window;
    if (!window.Initialize())
    {
        std::cerr << "Failed to create overlay window" << std::endl;
        CoUninitialize();
        return -1;
    }

    // Create renderer
    GlowRenderer renderer;
    if (!renderer.Initialize(window.GetHandle()))
    {
        std::cerr << "Failed to initialize Direct2D renderer" << std::endl;
        CoUninitialize();
        return -1;
    }

    // Create audio capture (Phase 2)
    AudioCapture audioCapture;
    if (!audioCapture.Initialize())
    {
        std::cerr << "Failed to initialize audio capture" << std::endl;
        CoUninitialize();
        return -1;
    }

    // Start audio capture
    if (!audioCapture.Start())
    {
        std::cerr << "Failed to start audio capture" << std::endl;
        CoUninitialize();
        return -1;
    }

    // Show the window
    window.Show();

    // For console app, use a simple timeout or manual exit
    std::cout << "EdgeGlow Phase 2 Test" << std::endl;
    std::cout << "=====================" << std::endl;
    std::cout << "You should see:" << std::endl;
    std::cout << "1. Cyan glow at the top of your screen" << std::endl;
    std::cout << "2. Console output showing RMS energy values" << std::endl;
    std::cout << std::endl;
    std::cout << "Play some music to see RMS values change!" << std::endl;
    std::cout << "Running for 30 seconds... Press Ctrl+C to exit early" << std::endl;
    std::cout << std::endl;

    // Main render loop - run for 30 seconds for testing
    MSG msg = {};
    auto startTime = std::chrono::steady_clock::now();
    const auto testDuration = std::chrono::seconds(30);

    // Target 60 FPS
    constexpr int TARGET_FPS = 60;
    constexpr auto FRAME_TIME = std::chrono::milliseconds(1000 / TARGET_FPS);

    while (std::chrono::steady_clock::now() - startTime < testDuration)
    {
        auto frameStart = std::chrono::steady_clock::now();

        // Process Windows messages (though we don't expect many in console app)
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Get RMS energy from audio capture (Phase 2)
        float rmsEnergy = audioCapture.GetRMSEnergy();

        // Print RMS to console every 10 frames (~6 times per second)
        static int frameCount = 0;
        if (frameCount++ % 10 == 0)
        {
            std::cout << "\rRMS Energy: " << rmsEnergy << " | ";

            // Visual bar (crude volume meter)
            int barLength = static_cast<int>(rmsEnergy * 50.0f);
            std::cout << "[";
            for (int i = 0; i < 50; i++)
            {
                std::cout << (i < barLength ? "=" : " ");
            }
            std::cout << "]" << std::flush; // Use flush to ensure output appears

            // Add newline every second (60 frames)
            if (frameCount % 60 == 0)
            {
                std::cout << std::endl;
            }
        }

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

    std::cout << std::endl
              << "Test completed! EdgeGlow Phase 2 working correctly." << std::endl;

    // Cleanup
    audioCapture.Stop();
    renderer.Cleanup();
    CoUninitialize();

    return 0;
}