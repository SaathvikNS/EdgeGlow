#pragma once

#include <windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dcomp.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dcomp.lib")

namespace EdgeGlow {

/**
 * @class GlowRenderer
 * @brief Handles all Direct2D rendering for the edge glow effect.
 * 
 * This class manages:
 * - Direct2D factory and render target initialization
 * - Drawing edge glow rectangles with gradients
 * - Frame rendering and presentation
 * 
 * Phase 1: Renders a static glow on the top edge for testing.
 */
class GlowRenderer {
public:
    GlowRenderer();
    ~GlowRenderer();

    // Prevent copying
    GlowRenderer(const GlowRenderer&) = delete;
    GlowRenderer& operator=(const GlowRenderer&) = delete;

    /**
     * @brief Initialize Direct2D resources.
     * @param hwnd Window handle to render to.
     * @return true if successful, false otherwise.
     */
    bool Initialize(HWND hwnd);

    /**
     * @brief Render a frame.
     * @note Phase 1: Renders static test glow on top edge.
     */
    void Render();

    /**
     * @brief Clean up Direct2D resources.
     */
    void Cleanup();

private:
    /**
     * @brief Create device-independent resources (factory, etc.).
     */
    bool CreateDeviceIndependentResources();

    /**
     * @brief Create device-dependent resources (render target, brushes).
     */
    bool CreateDeviceResources();

    /**
     * @brief Discard device-dependent resources.
     */
    void DiscardDeviceResources();

    /**
     * @brief Draw the edge glow effect.
     * @note Phase 1: Only draws top edge as a test.
     */
    void DrawEdgeGlow();

    /**
     * @brief Create a gradient brush for the glow effect.
     * @param startColor Starting color (inner edge).
     * @param endColor Ending color (outer edge, transparent).
     * @param isHorizontal True for horizontal gradient, false for vertical.
     * @return Gradient brush (caller must release).
     */
    ID2D1LinearGradientBrush* CreateGradientBrush(
        D2D1_COLOR_F startColor,
        D2D1_COLOR_F endColor,
        bool isHorizontal
    );

private:
    HWND m_hwnd;                                    // Window handle
    ID2D1Factory* m_d2dFactory;                     // Direct2D factory
    ID2D1HwndRenderTarget* m_renderTarget;          // Render target
    
    // Brushes
    ID2D1SolidColorBrush* m_solidBrush;             // For testing
    ID2D1LinearGradientBrush* m_gradientBrush;      // For glow effect

    // Window dimensions
    int m_width;
    int m_height;

    // Test parameters (Phase 1 only)
    static constexpr float TEST_GLOW_THICKNESS = 20.0f;
    static constexpr D2D1_COLOR_F TEST_GLOW_COLOR = { 0.0f, 0.8f, 1.0f, 1.0f }; // Cyan
};

} // namespace EdgeGlow
