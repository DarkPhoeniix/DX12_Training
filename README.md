# DX12 Render Engine

An experimental real-time renderer built with C++ and DirectX 12 for exploring modern graphics features and techniques.

![Demo](Saved/Screenshots/Demo.png)

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
DX12_RenderEngine/
├── DX12Lib/            # Abstractions over DirectX 12 API
├── Editor/             # Runtime scene editor and UI tools
├── GPUCrashTracker/    # Aftermath SDK wrapper to create crash dumps
├── Logger/             # small logging lib with spdlog
├── RenderGraph/        # Frame graph implementation for render pass scheduling
├── Renderer/           # Core rendering logic and systems
├── Saved/              # Test scenes and demo screenshots
└── DX12_Sandbox.sln    # Visual Studio solution file for building the project
```

## ⚙️ Build Instructions

### Prerequisites

- Visual Studio 2022
- Windows 10 SDK
- Installed vcpkg manager for dependencies

### Building

```
git clone https://github.com/DarkPhoeniix/DX12_RenderEngine.git
```
- Open DX12_Sandbox.sln
- Set Editor as the startup project
- Build x64 Debug/Release configuration
- Run *Editor.exe*
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
