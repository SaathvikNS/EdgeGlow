#include "GlowRenderer.h"
#include <stdexcept>

namespace EdgeGlow {

GlowRenderer::GlowRenderer()
    : m_hwnd(nullptr)
    , m_d2dFactory(nullptr)
    , m_renderTarget(nullptr)
    , m_solidBrush(nullptr)
    , m_gradientBrush(nullptr)
    , m_width(0)
    , m_height(0)
{
}

GlowRenderer::~GlowRenderer() {
    Cleanup();
}

bool GlowRenderer::Initialize(HWND hwnd) {
    m_hwnd = hwnd;

    // Get window dimensions
    RECT rc;
    GetClientRect(hwnd, &rc);
    m_width = rc.right - rc.left;
    m_height = rc.bottom - rc.top;

    // Create device-independent resources
    if (!CreateDeviceIndependentResources()) {
        return false;
    }

    // Create device-dependent resources
    if (!CreateDeviceResources()) {
        return false;
    }

    return true;
}

bool GlowRenderer::CreateDeviceIndependentResources() {
    // Create Direct2D factory
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &m_d2dFactory
    );

    return SUCCEEDED(hr);
}

bool GlowRenderer::CreateDeviceResources() {
    if (m_renderTarget) {
        return true; // Already created
    }

    // Create render target properties with proper alpha mode for transparency
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            D2D1_ALPHA_MODE_PREMULTIPLIED  // Critical for transparency
        )
    );

    // Create HWND render target with proper presentation options
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(
        m_hwnd,
        D2D1::SizeU(m_width, m_height),
        D2D1_PRESENT_OPTIONS_IMMEDIATELY  // Important: immediate presentation
    );

    HRESULT hr = m_d2dFactory->CreateHwndRenderTarget(
        props,
        hwndProps,
        &m_renderTarget
    );

    if (FAILED(hr)) {
        return false;
    }

    // Create a solid brush for testing
    hr = m_renderTarget->CreateSolidColorBrush(
        TEST_GLOW_COLOR,
        &m_solidBrush
    );

    if (FAILED(hr)) {
        return false;
    }

    // Create gradient brush for glow effect
    D2D1_COLOR_F innerColor = TEST_GLOW_COLOR;
    D2D1_COLOR_F outerColor = TEST_GLOW_COLOR;
    outerColor.a = 0.0f; // Transparent outer edge

    m_gradientBrush = CreateGradientBrush(innerColor, outerColor, false);

    return m_gradientBrush != nullptr;
}

void GlowRenderer::DiscardDeviceResources() {
    if (m_solidBrush) {
        m_solidBrush->Release();
        m_solidBrush = nullptr;
    }

    if (m_gradientBrush) {
        m_gradientBrush->Release();
        m_gradientBrush = nullptr;
    }

    if (m_renderTarget) {
        m_renderTarget->Release();
        m_renderTarget = nullptr;
    }
}

void GlowRenderer::Cleanup() {
    DiscardDeviceResources();

    if (m_d2dFactory) {
        m_d2dFactory->Release();
        m_d2dFactory = nullptr;
    }
}

void GlowRenderer::Render() {
    if (!m_renderTarget) {
        if (!CreateDeviceResources()) {
            return;
        }
    }

    // Begin drawing
    m_renderTarget->BeginDraw();

    // Clear to transparent
    m_renderTarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));

    // Draw the edge glow
    DrawEdgeGlow();

    // End drawing
    HRESULT hr = m_renderTarget->EndDraw();

    // Handle device loss
    if (hr == D2DERR_RECREATE_TARGET) {
        DiscardDeviceResources();
    }
}

void GlowRenderer::DrawEdgeGlow() {
    if (!m_gradientBrush) {
        return;
    }

    // Phase 1: Draw only the TOP edge as a test
    D2D1_RECT_F topEdge = D2D1::RectF(
        0.0f,                           // Left
        0.0f,                           // Top
        static_cast<float>(m_width),    // Right
        TEST_GLOW_THICKNESS             // Bottom
    );

    m_renderTarget->FillRectangle(topEdge, m_gradientBrush);

    // Future phases will add:
    // - Bottom edge
    // - Left edge
    // - Right edge
    // All with dynamic colors and thicknesses
}

ID2D1LinearGradientBrush* GlowRenderer::CreateGradientBrush(
    D2D1_COLOR_F startColor,
    D2D1_COLOR_F endColor,
    bool isHorizontal
) {
    if (!m_renderTarget) {
        return nullptr;
    }

    // Create gradient stops
    ID2D1GradientStopCollection* gradientStops = nullptr;
    D2D1_GRADIENT_STOP stops[2];

    stops[0].position = 0.0f;
    stops[0].color = startColor;

    stops[1].position = 1.0f;
    stops[1].color = endColor;

    HRESULT hr = m_renderTarget->CreateGradientStopCollection(
        stops,
        2,
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &gradientStops
    );

    if (FAILED(hr)) {
        return nullptr;
    }

    // Create linear gradient brush
    ID2D1LinearGradientBrush* brush = nullptr;

    // For top edge: gradient goes from top (0,0) to bottom (0, thickness)
    D2D1_POINT_2F start = D2D1::Point2F(0.0f, 0.0f);
    D2D1_POINT_2F end = D2D1::Point2F(
        isHorizontal ? static_cast<float>(m_width) : 0.0f,
        isHorizontal ? 0.0f : TEST_GLOW_THICKNESS
    );

    hr = m_renderTarget->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(start, end),
        gradientStops,
        &brush
    );

    gradientStops->Release();

    return SUCCEEDED(hr) ? brush : nullptr;
}

} // namespace EdgeGlow
