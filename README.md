# Fractal Engine

A from-scratch C++ game engine powered by OpenGL, designed for real-time graphics, clean architecture, and full low-level control over the rendering pipeline.

Fractal Engine focuses on understanding and building every core system manually instead of relying on heavy frameworks or black-box abstractions.  
The goal is performance, clarity, and learning how game engines truly work under the hood.

---

## ✨ Philosophy

Fractal Engine follows a few core principles:

• Build everything from scratch  
• Minimal abstractions, maximum control  
• Low-level control first  
• Clean and modular architecture  
• Real-time performance  

This project exists both as a production-ready foundation and as a learning journey into graphics programming and engine design.

---

## 🚀 Features (current and planned)

### Rendering
- OpenGL based renderer
- Shader system
- Texture loading (stb_image)
- Model loading (Assimp)
- Sprite rendering
- Camera system
- Real-time transformations

### Core
- Entity system
- Scene management
- Modular architecture
- Asset pipeline
- Input handling
- Time system

### Planned
- ECS architecture
- 2D/3D physics
- Animation system
- Lighting & PBR
- Post-processing
- Audio
- Scripting support
- Editor tools

---

## 🛠 Tech Stack

- C++20
- OpenGL
- GLFW
- stb_image
- Assimp
- CMake

---

## 📦 Build

```bash
git clone https://github.com/GbzR29/fractal-engine
cd fractal-engine
mkdir build
cd build
cmake ..
cmake --build .
