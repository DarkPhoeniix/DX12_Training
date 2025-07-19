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
- Render Graph (Frame graph) for pass scheduling
- Physically-based rendering (PBR)
- Image-based lighting support (IBL)
- GPU-side shadow mapping (spot and point lights)
- HDR rendering with tone-mapping
- Scene tree with entity-component system (ECS)
- Screen space ambient occlusion (SSAO)

## 📂 Project Structure

```bash
Source/
├── DX12Lib/            # Abstractions over DirectX 12 API
├── Editor/             # Runtime scene editor and UI tools
├── GPUCrashTracker/    # Aftermath SDK wrapper to create crash dumps
├── Logger/             # Small logging lib with spdlog
├── RenderGraph/        # Frame graph implementation for render pass scheduling
└── Renderer/           # Core rendering logic and systems
Saved/                  # Test scenes and demo screenshots
EquinoxEngine.sln       # Visual Studio solution file for building the project
```

## ⚙️ Build Instructions

### Prerequisites

- Visual Studio 2022
- Windows 10 SDK
- Installed vcpkg manager for dependencies

### Building

```
git clone https://github.com/DarkPhoeniix/EquinoxEngine.git
```
- Run ```DownloadAftermathSDK.bat```
- Open EquinoxEngine.sln
- Set Editor as the startup project
- Build x64 Debug/Release configuration
- Run *Equinox Engine.exe*
- *Optionally*: set a custom scene path as a command line parameter

## 🧪 Future Enhancements

- [ ] Frustum culling (possible GPU-side)
- [ ] Render pass for (semi-)transparent meshes
- [ ] Ray tracing (DXR) + denoising
- [ ] Bindless resources
- [ ] Full scene serialization/deserialization
- [ ] Cascaded shadow maps
- [ ] Bloom, DoF, Bokeh effect, SSR
- [ ] One sunny day, I believe, there will be a place for CMake
