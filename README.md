# 🔷 Fractal Engine

A scalable, modular C++20 game engine.

## 🏗️ Structure
- `src/`       — Implementation (.cpp)
- `include/`   — Public headers (.h/.hpp)
- `assets/`    — Shaders, textures, models, audio
- `tests/`     — Unit & integration tests
- `tools/`     — Asset pipeline, shader compiler, editor
- `docs/`      — Architecture & guides

## ⚙️ Building
```bash

cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake -S . -B build
mingw32-make -j8



cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## 📦 Dependencies
Managed via `vcpkg.json`: glfw3, glm, assimp, SDL3, lua, entt, nlohmann-json
