> [!WARNING]  
> This project is WIP.

# Grey-Scott Reaction-Diffusion Simulation

A high-performance cellular automata simulation implementing the Grey-Scott model using OpenCL for parallel computation and OpenGL for real-time visualization.

## Project Overview

This project simulates reaction-diffusion systems using the Grey-Scott model, which produces complex emergent patterns from simple chemical reactions. The simulation leverages:

- **OpenCL**: GPU-accelerated parallel computation
- **OpenGL 4.6**: Hardware-accelerated graphics rendering
- **SDL2**: Cross-platform window management
- **C++17**: Modern C++ with clean architecture

## Project Structure

```
grey-scott-sim/
├── src/
│   ├── main.cpp              # Entry point
│   ├── core/
│   │   └── Application.cpp   # Main application loop
│   ├── compute/
│   │   └── ComputeManager.cpp # OpenCL wrapper
│   └── graphics/
│       └── Renderer.cpp      # OpenGL renderer
├── include/                  # Header files
├── kernels/                  # OpenCL kernel sources
├── shaders/                  # GLSL shader sources
├── assets/                   # Resources
└── CMakeLists.txt           # Build configuration
```

## Requirements

### Arch Linux
```bash
sudo pacman -S cmake gcc sdl2 opencl-headers ocl-icd mesa glew
```

### Ubuntu/Debian
```bash
sudo apt install cmake g++ libsdl2-dev opencl-headers ocl-icd-opencl-dev mesa-common-dev libglew-dev
```

### macOS
```bash
brew install cmake sdl2
# OpenCL and OpenGL are included in macOS
```

## Building

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run
./GreyScottSim
```

## Controls

- **ESC**: Quit application
- More controls coming in future milestones...

## License

Academic project for GPU Programming course.
