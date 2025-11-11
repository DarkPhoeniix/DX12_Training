![Banner](Saved/Screenshots/Banner.png)

<div align="center">
  <a href="https://github.com/DarkPhoeniix/EquinoxEngine/releases"><img alt=platforms src="https://img.shields.io/badge/platforms-Windows-red?style=flat"/></a>
  <a href="https://github.com/DarkPhoeniix/EquinoxEngine/releases"><img alt=releases src="https://img.shields.io/github/v/release/DarkPhoeniix/EquinoxEngine?color=yellow"/></a>
  <a><img alt=checks src="https://img.shields.io/github/check-runs/DarkPhoeniix/EquinoxEngine/develop"/></a>
  <a href="https://github.com/DarkPhoeniix/EquinoxEngine/blob/develop/LICENSE"><img alt=lisence src="https://img.shields.io/github/license/DarkPhoeniix/EquinoxEngine?color=blue"/></a>
  <a><img alt=size src="https://img.shields.io/github/repo-size/DarkPhoeniix/EquinoxEngine?color=purple"/></a>
</div>

# Equinox Engine

An experimental real-time renderer built with C++ and DirectX 12 for exploring modern graphics features and techniques.

## ✨ Features

- Deferred rendering pipeline
- Asynchronous command lists building
- Render Graph (Frame graph) for pass scheduling with automated resource state transitions
- Bindless resource management
- Physically-based rendering (PBR)
- Image-based lighting support (IBL)
- Shadow mapping (spot/point lights) with GPU-driven shadow caster culling
- HDR rendering with tone-mapping
- Scene tree with entity-component system (ECS)
- Screen space ambient occlusion (SSAO)

## 📂 Project Structure

```bash
Source/
├── Editor/             # Runtime scene editor and UI tools
├── GPUCrashTracker/    # Aftermath SDK wrapper to create crash dumps
├── Logger/             # Small logging lib with spdlog
├── Renderer/           # Core rendering logic and systems
├── RenderGraph/        # Frame graph implementation for render pass scheduling
├── RHI/                # Abstractions over DirectX 12 API
└── Utility/            # Helper functions (math, timers, etc)
Saved/                  # Test scenes and demo screenshots
CMakeLists.txt          # Root CMake build configuration
CMakePresets.json       # CMake presets for configuring the build
vcpkg.json              # vcpkg dependencies manifest
```

## ⚙️ Build Instructions

### Prerequisites

- Visual Studio 2022
    * Desktop development with C++
    * Game development with C++
    * Cmake tools for Windows
    * vcpkg integration
- Windows 10 SDK
- DirectX 12 compatible GPU
- Shader Model 6.6+

### Building

##### Visual Studio

1. Clone the repository:
```
```
2. Open the CMakeLists.txt in Visual Studio with ```CMake...```
3. Configure the project with the desired preset (Debug/Profile/Release)
4. Build the project.
5. Run *Equinox Engine.exe*
6. *Optionally*: set a custom scene path as a command line parameter

##### CMake + vcpkg

1. Clone the repository:
```
git clone https://github.com/DarkPhoeniix/EquinoxEngine.git
```
2. Create a build directory and navigate into it:
```
mkdir build
cd build
```
3. Configure the project with CMake, specifying the vcpkg toolchain file:
```
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg_root]/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
```
4. Build the project:
```
cmake --build . --config Release
```
5. Run the executable from the build directory:
```
./EquinoxEngine/Equinox Engine.exe
```
6. *Optionally*: set a custom scene path as a command line parameter

## 🧪 Future Enhancements

- [ ] Frustum culling (possible GPU-side)
- [ ] Cascaded shadow maps
- [ ] DoF, Bokeh effect, SSR
- [ ] Vulkan backend
- [ ] Ray tracing (DXR) + denoising
