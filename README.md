# EdgeGlow

> **Phase 1**: Foundation - Transparent Overlay Window

A low-latency, GPU-accelerated audio-reactive screen edge glow overlay for Windows.

## Current Status: Phase 1 - Foundation

**Implemented:**

- Transparent, borderless overlay window
- Always-on-top rendering
- Click-through functionality
- Direct2D initialization with per-pixel alpha
- Static glow rendering with smooth transparency fade
- Offscreen bitmap rendering with UpdateLayeredWindow

**Next Phase:**

- Phase 2: Audio capture (WASAPI loopback)

---

## What is EdgeGlow?

EdgeGlow creates a beautiful, subtle glow effect around your screen edges that reacts to system audio in real-time. Perfect for music listening, gaming, or just adding ambient lighting to your desktop.

### Planned Features

- **Dual color modes**: Manual color selection or rainbow cycling
- **Adjustable parameters**: Intensity, sensitivity, thickness, smoothness
- **High performance**: < 2% CPU usage, GPU-accelerated
- **System integration**: Auto-start with Windows, system tray control
- **Audio reactive**: Syncs with bass, mids, treble, and beats

---

## Phase 1 Testing

### Build Instructions

See [BUILD.md](BUILD.md) for detailed instructions.

**Quick start:**

```bash
cd D:\new\Projects\ambientlighting\edgeglow
mkdir build && cd build
cmake .. -A x64
cmake --build . --config Release
.\bin\Release\EdgeGlow.exe
```

### What You Should See

When running Phase 1:

1. A message box explaining the test
2. A **cyan glow** at the **top edge** of your screen (20px thick)
3. **Smooth gradient effect** (solid cyan fading to fully transparent)
4. Window is click-through (doesn't block mouse interaction)
5. Desktop is fully visible through the transparent areas

Press **ESC** to exit.

---

## Architecture (Preview)

```
┌──────────────┐
│ System Audio │ ← WASAPI Loopback (Phase 2)
└──────┬───────┘
       │
┌──────▼────────┐
│ FFT Analysis  │ ← KissFFT (Phase 3)
│ Beat Detection│
└──────┬────────┘
       │
┌──────▼─────────┐
│ Visual Mapping │ ← Audio → Color/Intensity (Phase 4)
└──────┬─────────┘
       │
┌──────▼──────────┐
│ Direct2D Render │ ← Currently here (Phase 1) ✅
└─────────────────┘
```

---

## Tech Stack

| Component     | Technology                   |
| ------------- | ---------------------------- |
| Language      | C++17                        |
| Audio Capture | WASAPI                       |
| FFT           | KissFFT                      |
| Rendering     | Direct2D + DirectComposition |
| Windowing     | Win32 API                    |
| Build System  | CMake                        |

---

## Project Structure

```
edgeglow/
├── src/
│   ├── render/
│   │   ├── OverlayWindow.cpp	# Transparent window management
│   │   └── GlowRenderer.cpp	# Direct2D rendering
│   └── main.cpp		# Entry point
├── BUILD.md			# Build instructions
├── CMakeLists.txt
└── README.md
```

---

## Development Roadmap

- [x] **Phase 1**: Foundation (transparent overlay + Direct2D)
- [ ] **Phase 2**: Audio capture (WASAPI loopback)
- [ ] **Phase 3**: Audio analysis (FFT + beat detection)
- [ ] **Phase 4**: Visual mapping (audio → visuals)
- [ ] **Phase 5**: Reactive rendering (dynamic glow)
- [ ] **Phase 6**: Settings UI (modern Windows 11 style)
- [ ] **Phase 7**: System integration (tray, auto-start)
- [ ] **Phase 8**: Documentation & GitHub release

---

## Requirements

- **OS**: Windows 10 (1809+) or Windows 11
- **RAM**: 8 GB minimum, 16 GB recommended
- **GPU**: Any GPU with Direct2D support (integrated graphics OK)
- **Build**: Visual Studio 2022+ with C++ desktop development

---

## License

MIT License (to be added)

---

## Contributing

This project is in active development. Contributions welcome after Phase 8 (GitHub release).

---

**Current Phase**: 1/8 - Foundation
**Next Milestone**: Audio capture working
