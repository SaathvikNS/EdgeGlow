#include "GlowRenderer.h"
#include <stdexcept>

namespace EdgeGlow
{

    GlowRenderer::GlowRenderer()
        : m_hwnd(nullptr), m_d2dFactory(nullptr), m_renderTarget(nullptr), m_solidBrush(nullptr), m_gradientBrush(nullptr), m_width(0), m_height(0)
    {
    }

    GlowRenderer::~GlowRenderer()
    {
        Cleanup();
    }

    bool GlowRenderer::Initialize(HWND hwnd)
    {
        m_hwnd = hwnd;

        // Get window dimensions
        RECT rc;
        GetClientRect(hwnd, &rc);
        m_width = rc.right - rc.left;
        m_height = rc.bottom - rc.top;

        // Create device-independent resources
        if (!CreateDeviceIndependentResources())
        {
            return false;
        }

        // Create device-dependent resources
        if (!CreateDeviceResources())
        {
            return false;
        }

        return true;
    }

    bool GlowRenderer::CreateDeviceIndependentResources()
    {
        // Create Direct2D factory
        HRESULT hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            &m_d2dFactory);

        return SUCCEEDED(hr);
    }

    bool GlowRenderer::CreateDeviceResources()
    {
        if (m_renderTarget)
        {
            return true; // Already created
        }

        // ------------------------------------------------------------------
        // Allocate a 32‑bit premultiplied bitmap and create a DC for it. We
        // will render into this bitmap and then upload it to the layered
        // window using UpdateLayeredWindow(ULW_ALPHA).
        // ------------------------------------------------------------------

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = m_width;
        bmi.bmiHeader.biHeight = -m_height; // top‑down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        HDC screenDC = GetDC(nullptr);
        m_memDC = CreateCompatibleDC(screenDC);
        m_hBitmap = CreateDIBSection(screenDC, &bmi, DIB_RGB_COLORS, nullptr, nullptr, 0);
        ReleaseDC(nullptr, screenDC);

        if (!m_memDC || !m_hBitmap)
        {
            return false;
        }

        SelectObject(m_memDC, m_hBitmap);

        // Create the Direct2D render target that draws into the DC
        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                D2D1_ALPHA_MODE_PREMULTIPLIED // we need per-pixel alpha
                ));

        HRESULT hr = m_d2dFactory->CreateDCRenderTarget(&props, &m_renderTarget);
        if (FAILED(hr))
        {
            return false;
        }

        // Create a solid brush for testing
        hr = m_renderTarget->CreateSolidColorBrush(
            TEST_GLOW_COLOR,
            &m_solidBrush);

        if (FAILED(hr))
        {
            return false;
        }

        // Create gradient brush for glow effect. outerColor is transparent
        // when using alpha mode; it would be magenta if we were still
        // using a colour key.
        D2D1_COLOR_F innerColor = TEST_GLOW_COLOR;
        D2D1_COLOR_F outerColor = TEST_GLOW_COLOR_FADED;

        m_gradientBrush = CreateGradientBrush(innerColor, outerColor, false);

        return m_gradientBrush != nullptr;
    }

    void GlowRenderer::DiscardDeviceResources()
    {
        if (m_solidBrush)
        {
            m_solidBrush->Release();
            m_solidBrush = nullptr;
        }

        if (m_gradientBrush)
        {
            m_gradientBrush->Release();
            m_gradientBrush = nullptr;
        }

        if (m_renderTarget)
        {
            m_renderTarget->Release();
            m_renderTarget = nullptr;
        }

        if (m_memDC)
        {
            DeleteDC(m_memDC);
            m_memDC = nullptr;
        }
        if (m_hBitmap)
        {
            DeleteObject(m_hBitmap);
            m_hBitmap = nullptr;
        }
    }

    void GlowRenderer::Cleanup()
    {
        DiscardDeviceResources();

        if (m_d2dFactory)
        {
            m_d2dFactory->Release();
            m_d2dFactory = nullptr;
        }
    }

    void GlowRenderer::Render()
    {
        if (!m_renderTarget)
        {
            if (!CreateDeviceResources())
            {
                return;
            }
        }

        // Before drawing we must bind the DC so the render target knows
        // where to send the pixels.  A DCRenderTarget does nothing until
        // BindDC has been called with a valid HDC/size.
        // Bind the DC; the second parameter is a RECT giving the area
        // that the render target should treat as its drawing surface.
        RECT rc = {0, 0, m_width, m_height};
        m_renderTarget->BindDC(m_memDC, &rc);

        // Draw to the offscreen bitmap
        m_renderTarget->BeginDraw();
        m_renderTarget->Clear(D2D1::ColorF(0, 0, 0, 0)); // fully transparent
        DrawEdgeGlow();
        HRESULT hr = m_renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceResources();
            return;
        }

        // Push the bitmap to the layered window using per-pixel alpha
        BLENDFUNCTION blend = {};
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;

        SIZE size = {m_width, m_height};
        POINT ptSrc = {0, 0};
        POINT ptDest = {0, 0};

        HDC screenDC = GetDC(nullptr);
        BOOL ok = UpdateLayeredWindow(
            m_hwnd,
            screenDC,
            &ptDest,
            &size,
            m_memDC,
            &ptSrc,
            0,
            &blend,
            ULW_ALPHA);
        if (!ok)
        {
            DWORD err = GetLastError();
            // For debugging we pop a message box once so the user knows
            // the update failed.  In release builds you can remove this.
            wchar_t buf[256];
            swprintf_s(buf, L"UpdateLayeredWindow failed: %u", err);
            MessageBox(nullptr, buf, L"EdgeGlow", MB_ICONERROR);
        }
        ReleaseDC(nullptr, screenDC);
    }

    void GlowRenderer::DrawEdgeGlow()
    {
        if (!m_gradientBrush)
        {
            return;
        }

        // Phase 1: Draw only the TOP edge as a test
        D2D1_RECT_F topEdge = D2D1::RectF(
            0.0f,                        // Left
            0.0f,                        // Top
            static_cast<float>(m_width), // Right
            TEST_GLOW_THICKNESS          // Bottom
        );

        // fill with gradient brush
        m_renderTarget->FillRectangle(topEdge, m_gradientBrush);

        // Future phases will add:
        // - Bottom edge
        // - Left edge
        // - Right edge
        // All with dynamic colors and thicknesses
    }

    ID2D1LinearGradientBrush *GlowRenderer::CreateGradientBrush(
        D2D1_COLOR_F startColor,
        D2D1_COLOR_F endColor,
        bool isHorizontal)
    {
        if (!m_renderTarget)
        {
            return nullptr;
        }

        // Create gradient stops
        ID2D1GradientStopCollection *gradientStops = nullptr;
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
            &gradientStops);

        if (FAILED(hr))
        {
            return nullptr;
        }

        // Create linear gradient brush
        ID2D1LinearGradientBrush *brush = nullptr;

        // For top edge: gradient goes from top (0,0) to bottom (0, thickness)
        D2D1_POINT_2F start = D2D1::Point2F(0.0f, 0.0f);
        D2D1_POINT_2F end = D2D1::Point2F(
            isHorizontal ? static_cast<float>(m_width) : 0.0f,
            isHorizontal ? 0.0f : TEST_GLOW_THICKNESS);

        hr = m_renderTarget->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(start, end),
            gradientStops,
            &brush);

        gradientStops->Release();

        return SUCCEEDED(hr) ? brush : nullptr;
    }

} // namespace EdgeGlow
