# EdgeGlow - Build Instructions (Phase 1)

## Prerequisites

- **Visual Studio 2026 Insiders** (or 2022) with C++ desktop development workload
- **CMake** 3.20 or later (in PATH)
- **Windows 11** (or Windows 10 1809+)
- **Windows SDK** (included with Visual Studio)

## Quick Build (Command Line)

### Option 1: Using CMake + MSVC

```bash
# Navigate to project root
cd D:\new\Projects\ambientlighting\edgeglow

# Create build directory
mkdir build
cd build

# Configure with CMake (use x64 architecture)
cmake .. -A x64

# Build the project
cmake --build . --config Release

# Run the executable
.\bin\Release\EdgeGlow.exe
```

### Option 2: Using Developer Command Prompt

```bash
# Open "Developer Command Prompt for VS 2026"
cd D:\new\Projects\ambientlighting\edgeglow
mkdir build
cd build

# Configure and build
cmake .. -G "Visual Studio 17 2026" -A x64
cmake --build . --config Release

# Run
bin\Release\EdgeGlow.exe
```

## Build with Visual Studio IDE

1. Open **Visual Studio 2026 Insiders**
2. Select **"Open a local folder"**
3. Navigate to `D:\new\Projects\ambientlighting\edgeglow`
4. Visual Studio will auto-detect CMakeLists.txt
5. Select **Release x64** configuration
6. Press **F5** or click **"Build" → "Build All"**
7. Run from `out\build\x64-Release\bin\EdgeGlow.exe`

## Testing Phase 1

### Expected Behavior

When you run `EdgeGlow.exe`:

1. ✅ A message box appears explaining Phase 1
2. ✅ Press OK
3. ✅ You should see a **cyan glow** at the **top edge** of your screen (20px thick)
4. ✅ The glow has a gradient effect (solid at screen edge, fading downward)
5. ✅ The overlay is **click-through** (you can interact with windows beneath it)
6. ✅ Press **ESC** to exit

### Troubleshooting

**Problem**: "Cannot find d2d1.lib"
- **Solution**: Ensure Windows SDK is installed via Visual Studio Installer

**Problem**: Window appears but no glow visible
- **Solution**: Check GPU drivers are up to date

**Problem**: Application crashes on startup
- **Solution**: Run from Developer Command Prompt to see error messages

**Problem**: Black screen instead of transparent
- **Solution**: Verify `WS_EX_LAYERED` flag is set (check OverlayWindow.cpp)

## Clean Build

```bash
# Remove build directory
cd D:\new\Projects\ambientlighting\edgeglow
rmdir /s /q build

# Rebuild from scratch
mkdir build
cd build
cmake .. -A x64
cmake --build . --config Release
```

## Performance Verification

While EdgeGlow is running:

1. Open **Task Manager** (Ctrl+Shift+Esc)
2. Go to **"Details"** tab
3. Find **EdgeGlow.exe**
4. Check CPU usage: Should be **< 2%**
5. Check Memory: Should be **< 30 MB**

## Next Steps

Once Phase 1 is verified working:
- **Phase 2**: Audio capture (WASAPI)
- **Phase 3**: FFT analysis
- **Phase 4**: Reactive rendering

---

**Phase 1 Goal**: Confirm transparent overlay window works and Direct2D renders correctly.

If you see the cyan glow at the top of your screen → **Phase 1 Complete! ✅**
