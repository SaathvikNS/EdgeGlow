# EdgeGlow

> **Phase 3**: FFT Analysis & Beat Detection

A low-latency, GPU-accelerated audio-reactive screen edge glow overlay for Windows.

## Current Status: Phase 2 Complete - Phase 3 Next

**Completed:**

- ✅ Transparent, borderless overlay window
- ✅ Always-on-top rendering
- ✅ Click-through functionality
- ✅ Direct2D per-pixel alpha rendering
- ✅ WASAPI loopback audio capture
- ✅ Lock-free circular buffer
- ✅ RMS energy calculation
- ✅ Console Output for audio monitoring

**Next Phase:**

- Phase 3: FFT analysis & beat detection

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

## Phase 2 Testing

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

When running Phase 2:

1. **Console output** showing audio format detection
2. A **cyan glow** at the **top edge** of your screen (20px thick)
3. **Real-time RMS energy values** in the console
4. Volume bar that moves with audio intensity
5. Play music/videos to see RMS values change!

**Expected Console Output:**

```
Audio Format:
  Sample Rate: 48000 Hz
  Channels: 2
  Bits Per Sample: 32
  Format Tag: 65534
  Format: Extensible IEEE Float
Audio capture initialized successfully!
Audio capture started!
Capture thread started
RMS Energy: 0.145 | [=======                                           ]
RMS Energy: 0.267 | [=============                                     ]
RMS Energy: 0.423 | [=====================                             ]
```

The application runs for 30 seconds then exits automatically.

---

## Architecture

```
┌──────────────┐
│ System Audio │ ← WASAPI Loopback (Phase 2 ✅)
└──────┬───────┘
       │
┌──────▼──────────┐
│ Circular Buffer │ ← Lock-free SPSC (Phase 2 ✅)
└──────┬──────────┘
       │
┌──────▼────────┐
│  RMS Energy   │ ← Volume Calculation (Phase 2 ✅)
└──────┬────────┘
       │
┌──────▼────────┐
│ FFT Analysis  │ ← KissFFT (Phase 3 - Next)
│ Beat Detection│
└──────┬────────┘
       │
┌──────▼─────────┐
│ Visual Mapping │ ← Audio → Color/Intensity (Phase 4)
└──────┬─────────┘
       │
┌──────▼──────────┐
│ Direct2D Render │ ← Static Glow (Phase 1 ✅)
└─────────────────┘
```

---

## Tech Stack

| Component     | Technology                   |
| ------------- | ---------------------------- |
| Language      | C++17                        |
| Audio Capture | WASAPI Loopback              |
| Audio Buffer  | Lock-free Circular Buffer    |
| FFT           | KissFFT (Phase 3)            |
| Rendering     | Direct2D + DirectComposition |
| Windowing     | Win32 API                    |
| Build System  | CMake                        |

---

## Project Structure

```
edgeglow/
├── src/
│   ├── audio/
│   │   ├── AudioCapture.h      # WASAPI audio capture class
│   │   └── AudioCapture.cpp    # WASAPI implementation
│   ├── render/
│   │   ├── OverlayWindow.h     # Transparent window management
│   │   ├── OverlayWindow.cpp   # Window implementation
│   │   ├── GlowRenderer.h      # Direct2D renderer class
│   │   └── GlowRenderer.cpp    # Direct2D rendering
│   ├── utils/
│   │   └── CircularBuffer.h    # Lock-free ring buffer
│   └── main.cpp                # Application entry point
├── libs/
│   ├── json/                   # JSON library for config
│   └── kissfft/                # FFT library for analysis
├── docs/                       # Documentation
├── BUILD.md                    # Build instructions
├── CMakeLists.txt              # CMake configuration
└── README.md                   # This file
```

---

## Development Roadmap

- [x] **Phase 1**: Foundation (transparent overlay + Direct2D)
- [x] **Phase 2**: Audio capture (WASAPI loopback)
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

MIT Licence - [Click Here](./LICENSE "MIT LIcense")

---

## Contributing

This project is in active development. Contributions welcome after Phase 8 (GitHub release).

---

**Current Phase**: 2/8 - Audio Capture Complete ✅
**Next Milestone**: FFT analysis & beat detection
