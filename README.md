# Grey-Scott Simulation

Real-time reaction-diffusion simulation using OpenCL and OpenGL.

<table>
  <tr>
    <td width="33%">
      <img src="assets/1.png" alt="Pattern 1" width="100%">
    </td>
    <td width="33%">
      <img src="assets/2.png" alt="Pattern 2" width="100%">
    </td>
    <td width="33%">
      <img src="assets/3.png" alt="Pattern 3" width="100%">
    </td>
  </tr>
  <tr>
    <td width="33%">
      <img src="assets/4.png" alt="Pattern 4" width="100%">
    </td>
    <td width="33%">
      <img src="assets/5.png" alt="Pattern 5" width="100%">
    </td>
    <td width="33%">
      <img src="assets/6.png" alt="Pattern 6" width="100%">
    </td>
  </tr>
  <tr>
    <td width="33%">
      <img src="assets/7.png" alt="Pattern 7" width="100%">
    </td>
    <td width="33%">
      <img src="assets/8.png" alt="Pattern 8" width="100%">
    </td>
    <td width="33%">
      <img src="assets/9.png" alt="Pattern 9" width="100%">
    </td>
  </tr>
</table>

## Build

### Prerequisites

- CMake 3.15+
- C++17 compiler (GCC, Clang, or MSVC)
- Ninja (recommended) or Make

### Install Dependencies

**Linux (Ubuntu/Debian):**
```bash
sudo apt install cmake ninja-build g++ libsdl2-dev libglew-dev libgl1-mesa-dev \
    opencl-headers ocl-icd-opencl-dev
```

**Linux (Arch):**
```bash
sudo pacman -S cmake ninja gcc sdl2 glew mesa opencl-headers ocl-icd
```

**macOS:**
```bash
brew install cmake ninja sdl2 glew
```

**Windows (vcpkg - Recommended):**
```powershell
# One-time vcpkg setup
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg install sdl2 glew opencl --triplet=x64-windows

# Build (from project directory)
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release

# Run
.\build\Release\GreyScottSim.exe
```

**Windows (MSYS2/MinGW alternative):**
```bash
# Install dependencies in MSYS2 MinGW64 shell
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-SDL2 mingw-w64-x86_64-glew mingw-w64-x86_64-opencl-headers

# Build
cmake --preset default
cmake --build build -j
./build/GreyScottSim.exe
```

> **Note:** OpenCL requires GPU drivers. NVIDIA, AMD, and Intel GPU drivers include OpenCL runtime.

### Build & Run (Linux/macOS)

```bash
cmake --preset default
cmake --build build -j
./build/GreyScottSim
```

## Platform Notes

| Platform | GPU (OpenCL) | OpenGL Version | Notes |
|----------|--------------|----------------|-------|
| Linux    | Full support | 4.6            | Best experience |
| Windows  | Full support | 4.6            | Requires OpenCL drivers |
| macOS    | CPU-only     | 4.1            | OpenCL deprecated by Apple |

## Controls

| Key | Action |
|-----|--------|
| **Space** | Pause/Resume |
| **R** | Reset simulation |
| **C** | Toggle CPU/GPU implementation |
| **ESC** | Quit |
| **Up/Down** | Adjust F (feed rate) |
| **Left/Right** | Adjust k (kill rate) |
| **F1-F5** | Pattern presets (spots, stripes, waves, chaos, holes) |

## Code Structure

```
grey-scott-sim/
├── CMakePresets.json                       # Build presets
├── CMakeLists.txt                          # Build configuration
├── include/                                # Header files
├── src/                                    # Source files
│   ├── main.cpp                            # Program entry point
│   ├── core/
│   │   └── Application.cpp                 # Event loop, input handling, ImGui overlay
│   ├── compute/
│   │   ├── ComputeManager.cpp              # OpenCL context and queue setup
│   │   └── Simulation.cpp                  # GPU Grey-Scott implementation
│   ├── cpu/
│   │   └── SimulationCPU.cpp               # CPU Grey-Scott reference implementation
│   ├── graphics/
│   │   └── Renderer.cpp                    # OpenGL texture rendering, shaders
│   ├── imgui_impl_opengl3.cpp              # ImGui OpenGL backend implementation
│   └── imgui_impl_sdl2.cpp                 # ImGui SDL2 backend implementation
│
├── kernels/
└── build/
```

## Project Info

Academic project for GPU Programming course. Demonstrates parallel computing concepts, performance analysis, and real-time graphics.
