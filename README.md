# Grey-Scott Reaction-Diffusion Simulation

A high-performance cellular automata simulation implementing the Grey-Scott model using OpenCL for parallel computation and OpenGL for real-time visualization.

## Project Overview

This project simulates reaction-diffusion systems using the Grey-Scott model, which produces complex emergent patterns from simple chemical reactions. The simulation leverages:

- **OpenCL**: GPU-accelerated parallel computation
- **OpenGL 4.6**: Hardware-accelerated graphics rendering
- **SDL2**: Cross-platform window management
- **C++17**: Modern C++ with clean architecture

## Features (Planned)

- [x] OpenCL and OpenGL initialization
- [x] Device enumeration and selection
- [x] Basic rendering framework
- [ ] Grey-Scott simulation kernel
- [ ] Real-time parameter adjustment
- [ ] Multiple pattern presets
- [ ] Multi-GPU/Multi-device support
- [ ] Performance benchmarking
- [ ] CPU fallback implementation
- [ ] Interactive visualization with color mapping

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

## Development Milestones

### ✅ Milestone 1: Project Setup (Current)
- CMake build system
- SDL2 window and OpenGL context
- OpenCL device enumeration and initialization
- Basic rendering infrastructure

### 🔄 Milestone 2: Core Simulation (Next)
- Grey-Scott reaction-diffusion equations
- OpenCL kernel implementation
- Initial conditions and patterns
- Basic visualization

### 📋 Milestone 3: Optimization
- Shared memory utilization
- Work group optimization
- Memory access patterns
- Performance profiling

### 📋 Milestone 4: Multi-Device Support
- Multi-GPU computation
- Domain decomposition
- Synchronization strategies
- Performance comparison

### 📋 Milestone 5: Advanced Features
- Interactive controls
- Parameter presets
- Color mapping
- Export capabilities

## Controls

- **ESC**: Quit application
- More controls coming in future milestones...

## Performance Goals

- Real-time simulation at 512x512 resolution
- 60+ FPS rendering
- Scalable to 2048x2048 or larger grids
- Multi-device speedup documentation

## Documentation

Development progress, optimization strategies, and performance evaluations will be documented throughout the project to meet academic requirements.

## License

Academic project for parallel computing course.

## Author

Lothiard - November 2025

## Version History

- **v0.1.0** (2025-11-23): Initial setup with OpenCL/OpenGL infrastructure
