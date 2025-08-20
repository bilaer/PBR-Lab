# PBR Lab

**PBR Lab** is an experimental/learning project focused on testing **Physically Based Rendering (PBR)** with **Image-Based Lighting (IBL)**.  
All environment maps are generated using my custom **IBL Gen** tool, which produces the **Irradiance Map**, **Prefilter Map**, and **BRDF LUT** on the GPU.


## ✨ Features

- **Model Loading**
  - Load `.ply` files (via Assimp)
  - Load `.glb` / `.gltf` files (via TinyGLTF)
- **Basic Geometry**
  - Built-in sphere and plane primitives
- **Texture Binding**
  - Manually bind textures to material slots
  - Or directly read PBR textures from model files
- **IBL Integration**
  - Import precomputed maps from **IBL Gen**:
    - Irradiance Map
    - Prefilter Map
    - BRDF LUT

---

## 🛠️ Dependencies

- **Rendering & Assets**
  - `stb_image`: load 2D textures
  - `tinygltf`: load `.glb` / `.gltf` models
  - `assimp`: load `.ply` models
  - `gli`: read `.ktx` environment maps and LUTs
- **Debug GUI**
  - `ImGui`: in-engine debugging interface
- **Offline Tool**
  - **IBL Gen** (custom): My custom GPU-based tool for generating environment maps and BRDF LUTs (to be open-sourced soon).

---

## 🚀 Roadmap

- Extend BRDF models:
  - Anisotropic BRDF
  - Clearcoat BRDF
- Add **Spherical Harmonics (SH)** support for real-time irradiance map approximation
- Provide more built-in geometries and model format support
- Enhance debugging and visualization tools
- Might grow into a complete graphics engine in the future.
---

## 📷 Screenshots

(Add your images here, e.g.:)

| Sphere (IBL) | Plane (IBL) |
|--------------|-------------|
| ![](images/sphere.png) | ![](images/plane.png) |

---

## 🔧 Build & Run

> **TODO:** Instructions for building and running will be added later.

---

## 📄 License

MIT License
