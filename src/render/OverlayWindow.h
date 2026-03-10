#pragma once

#include <windows.h>
#include <string>

namespace EdgeGlow {

/**
 * @class OverlayWindow
 * @brief Manages the transparent, always-on-top, click-through overlay window.
 * 
 * This window covers the entire screen and serves as the canvas for rendering
 * the edge glow effect. It's configured to be:
 * - Borderless (no title bar, no frame)
 * - Transparent background
 * - Click-through (WS_EX_TRANSPARENT)
 * - Always on top (WS_EX_TOPMOST)
 * - Full screen coverage
 */
class OverlayWindow {
public:
    OverlayWindow();
    ~OverlayWindow();

    // Prevent copying
    OverlayWindow(const OverlayWindow&) = delete;
    OverlayWindow& operator=(const OverlayWindow&) = delete;

    /**
     * @brief Initialize and create the overlay window.
     * @return true if successful, false otherwise.
     */
    bool Initialize();

    /**
     * @brief Get the window handle.
     * @return HWND handle to the overlay window.
     */
    HWND GetHandle() const { return m_hwnd; }

    /**
     * @brief Get the window width.
     * @return Width in pixels.
     */
    int GetWidth() const { return m_width; }

    /**
     * @brief Get the window height.
     * @return Height in pixels.
     */
    int GetHeight() const { return m_height; }

    /**
     * @brief Show the overlay window.
     */
    void Show();

    /**
     * @brief Hide the overlay window.
     */
    void Hide();

    /**
     * @brief Check if window is visible.
     */
    bool IsVisible() const { return m_visible; }

private:
    /**
     * @brief Window procedure callback.
     */
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handle window messages.
     */
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Register the window class.
     */
    bool RegisterWindowClass();

    /**
     * @brief Create the actual window.
     */
    bool CreateOverlayWindow();

private:
    HWND m_hwnd;              // Window handle
    HINSTANCE m_hInstance;    // Application instance
    int m_width;              // Screen width
    int m_height;             // Screen height
    bool m_visible;           // Visibility state

    static constexpr wchar_t CLASS_NAME[] = L"EdgeGlowOverlay";
};

} // namespace EdgeGlow
