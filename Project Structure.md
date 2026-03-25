# FractalEngine — Estrutura do Projeto

```
FractalEngine/
│
├── CMakeLists.txt                  ← Raiz: configura lib, executável, third_party
├── LICENSE
├── README.md
├── vcpkg.json                      ← Dependências via vcpkg (alternativa ao third_party manual)
├── .clang-format
├── .clang-tidy
├── .gitignore
│
│
├── assets/                         ← Recursos em runtime (não compilados)
│   ├── animations/
│   ├── audio/
│   │   ├── music/
│   │   └── sfx/
│   ├── fonts/
│   ├── models/
│   ├── prefabs/
│   ├── scenes/
│   ├── scripts/
│   ├── shaders/
│   │   ├── glsl/
│   │   │   ├── compute/
│   │   │   ├── fragment/
│   │   │   ├── geometry/
│   │   │   └── vertex/
│   │   ├── hlsl/
│   │   └── spirv/                  ← Shaders pré-compilados (Vulkan)
│   └── textures/
│       ├── environment/
│       └── ui/
│
│
├── docs/
│   ├── api/
│   │   └── core.md
│   ├── architecture/
│   │   ├── ecs.md
│   │   ├── overview.md
│   │   └── renderer.md
│   └── guides/
│       ├── building.md
│       ├── contributing.md
│       └── style_guide.md
│
│
├── include/fractal_engine/         ← Headers públicos da engine (interface externa)
│   ├── audio/
│   │   ├── AudioClip.h
│   │   ├── AudioEngine.h
│   │   ├── AudioListener.h
│   │   └── AudioSource.h
│   ├── core/
│   │   ├── Application.hpp
│   │   ├── Engine.hpp
│   │   ├── EventSystem.hpp
│   │   ├── Time.hpp
│   │   └── Window.hpp
│   ├── graphics/
│   │   ├── Camera.h
│   │   ├── Framebuffer.h
│   │   ├── Material.h
│   │   ├── Shader.h
│   │   ├── ShaderProgram.h
│   │   ├── Texture.h
│   │   └── TextureLoader.h
│   ├── input/
│   │   ├── Gamepad.h
│   │   ├── InputManager.hpp
│   │   ├── InputMapper.h
│   │   ├── Keyboard.h
│   │   └── Mouse.h
│   ├── math/
│   │   ├── MathUtils.h
│   │   ├── Noise.h
│   │   └── Random.h
│   ├── physics/
│   │   ├── AABB.h
│   │   ├── BVH.h
│   │   ├── Collider.h
│   │   ├── CollisionDetection.h
│   │   ├── Constraint.h
│   │   ├── PhysicsEngine.h
│   │   └── Rigidbody.h
│   ├── platform/
│   │   └── IPlatform.h
│   ├── renderer/
│   │   ├── backends/
│   │   │   ├── OpenGLRenderer.h
│   │   │   └── VulkanRenderer.h    ← só presente se FE_ENABLE_VULKAN=ON
│   │   ├── IRenderer.h
│   │   ├── Mesh.h
│   │   ├── MeshBuilder.h
│   │   ├── RenderGraph.h
│   │   └── RenderPass.h
│   ├── scene/
│   │   ├── ecs/
│   │   │   ├── ComponentPool.h
│   │   │   ├── Entity.h
│   │   │   ├── Registry.h
│   │   │   └── System.h
│   │   ├── Scene.h
│   │   ├── SceneManager.h
│   │   └── SceneSerializer.h
│   ├── scripting/
│   │   ├── LuaBinding.h
│   │   ├── ScriptContext.h
│   │   └── ScriptEngine.h
│   ├── ui/
│   │   ├── Button.h
│   │   ├── Panel.h
│   │   ├── Text.h
│   │   ├── UIManager.h
│   │   └── Widget.h
│   ├── utils/
│   │   ├── FileSystem.h
│   │   ├── Logger.h
│   │   ├── MemoryPool.h
│   │   ├── StringUtils.h
│   │   └── Timer.h
│   ├── world/
│   │   ├── Chunk.h
│   │   ├── ChunkMesh.h
│   │   ├── ChunkStreamer.h
│   │   ├── HeightMap.h
│   │   ├── TerrainGenerator.h
│   │   ├── World.h
│   │   └── WorldManager.h
│   └── FractalEngine.h             ← Header único de inclusão (umbrella header)
│
│
├── src/                            ← Implementações (.cpp)
│   ├── CMakeLists.txt              ← Orquestra todos os módulos
│   ├── main.cpp
│   │
│   ├── audio/
│   │   ├── AudioClip.cpp
│   │   ├── AudioEngine.cpp
│   │   ├── AudioListener.cpp
│   │   ├── AudioSource.cpp
│   │   └── CMakeLists.txt
│   ├── core/
│   │   ├── Application.cpp
│   │   ├── Engine.cpp
│   │   ├── EventSystem.cpp
│   │   ├── Time.cpp
│   │   ├── Window.cpp
│   │   └── CMakeLists.txt
│   ├── graphics/
│   │   ├── Camera.cpp
│   │   ├── Framebuffer.cpp
│   │   ├── Material.cpp
│   │   ├── Shader.cpp
│   │   ├── ShaderProgram.cpp
│   │   ├── Texture.cpp
│   │   ├── TextureLoader.cpp
│   │   └── CMakeLists.txt
│   ├── input/
│   │   ├── Gamepad.cpp
│   │   ├── InputManager.cpp
│   │   ├── InputMapper.cpp
│   │   ├── Keyboard.cpp
│   │   ├── Mouse.cpp
│   │   └── CMakeLists.txt
│   ├── math/
│   │   ├── MathUtils.cpp
│   │   ├── Noise.cpp
│   │   ├── Random.cpp
│   │   ├── Transform.cpp
│   │   └── CMakeLists.txt
│   ├── physics/
│   │   ├── collision/
│   │   │   ├── AABB.cpp
│   │   │   ├── BVH.cpp
│   │   │   └── CollisionDetection.cpp
│   │   ├── dynamics/
│   │   │   ├── Collider.cpp
│   │   │   ├── Constraint.cpp
│   │   │   └── Rigidbody.cpp
│   │   ├── PhysicsEngine.cpp
│   │   └── CMakeLists.txt
│   ├── platform/
│   │   ├── linux/
│   │   │   └── LinuxPlatform.cpp
│   │   ├── macos/
│   │   │   └── MacPlatform.cpp
│   │   ├── win32/
│   │   │   └── Win32Platform.cpp
│   │   ├── Platform.cpp
│   │   └── CMakeLists.txt
│   ├── renderer/
│   │   ├── backends/
│   │   │   ├── opengl/
│   │   │   │   ├── OpenGLBuffer.cpp
│   │   │   │   ├── OpenGLRenderer.cpp
│   │   │   │   ├── OpenGLShader.cpp
│   │   │   │   └── OpenGLTexture.cpp
│   │   │   └── vulkan/             ← só compilado se FE_ENABLE_VULKAN=ON
│   │   ├── Mesh.cpp
│   │   ├── MeshBuilder.cpp
│   │   ├── RenderGraph.cpp
│   │   ├── RenderPass.cpp
│   │   └── CMakeLists.txt
│   ├── scene/
│   │   ├── ecs/
│   │   │   ├── ComponentPool.cpp
│   │   │   ├── Entity.cpp
│   │   │   ├── Registry.cpp
│   │   │   └── System.cpp
│   │   ├── Scene.cpp
│   │   ├── SceneManager.cpp
│   │   ├── SceneSerializer.cpp
│   │   └── CMakeLists.txt
│   ├── scripting/                  ← só compilado se FE_ENABLE_LUA=ON
│   │   ├── LuaBinding.cpp
│   │   ├── ScriptContext.cpp
│   │   ├── ScriptEngine.cpp
│   │   └── CMakeLists.txt
│   ├── ui/
│   │   ├── Button.cpp
│   │   ├── Panel.cpp
│   │   ├── Text.cpp
│   │   ├── UIManager.cpp
│   │   ├── Widget.cpp
│   │   └── CMakeLists.txt
│   ├── utils/
│   │   ├── FileSystem.cpp
│   │   ├── Logger.cpp
│   │   ├── MemoryPool.cpp
│   │   ├── StringUtils.cpp
│   │   ├── Timer.cpp
│   │   └── CMakeLists.txt
│   └── world/
│       ├── Chunk.cpp
│       ├── ChunkMesh.cpp
│       ├── ChunkStreamer.cpp
│       ├── HeightMap.cpp
│       ├── TerrainGenerator.cpp
│       ├── World.cpp
│       ├── WorldManager.cpp
│       └── CMakeLists.txt
│
│
├── tests/
│   ├── CMakeLists.txt
│   ├── benchmarks/
│   │   └── BenchmarkRenderer.cpp
│   ├── integration/
│   │   └── TestSceneLoading.cpp
│   └── unit/
│       ├── core/
│       │   ├── TestEngine.cpp
│       │   ├── TestEventSystem.cpp
│       │   └── TestWindow.cpp
│       ├── math/
│       │   ├── TestMathUtils.cpp
│       │   ├── TestNoise.cpp
│       │   └── TestTransform.cpp
│       └── physics/
│           ├── TestCollision.cpp
│           └── TestRigidbody.cpp
│
│
├── third_party/                    ← Dependências externas (git submodules ou manual)
│   ├── assimp/
│   ├── entt/
│   ├── glad/
│   │   ├── include/
│   │   │   ├── glad/glad.h
│   │   │   └── KHR/khrplatform.h
│   │   └── src/glad.c
│   ├── glfw/
│   ├── glm/
│   ├── imgui/
│   ├── KHR/
│   ├── lua/
│   ├── nlohmann/
│   ├── SDL3/
│   └── stb/
│
│
└── tools/
    ├── CMakeLists.txt
    ├── asset_pipeline/
    │   ├── AssetImporter.cpp
    │   └── CMakeLists.txt
    ├── editor/
    │   ├── Editor.cpp
    │   └── CMakeLists.txt
    └── shader_compiler/
        ├── ShaderCompiler.cpp
        └── CMakeLists.txt
```

---

## Como compilar (MinGW)

```bash
mkdir build && cd build

# Debug
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
mingw32-make -j8

# Release
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make -j8

# Com Vulkan habilitado
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DFE_ENABLE_VULKAN=ON
mingw32-make -j8

# Sem testes e sem tools
cmake .. -G "MinGW Makefiles" -DFE_BUILD_TESTS=OFF -DFE_BUILD_TOOLS=OFF
mingw32-make -j8
```

## Saídas do build

```
build/
├── bin/
│   ├── FractalEngine.exe       ← executável principal
│   ├── FractalEditor.exe       ← editor (se FE_BUILD_EDITOR=ON)
│   ├── AssetImporter.exe
│   └── ShaderCompiler.exe
└── lib/
    └── libFractalEngineLib.a   ← engine como biblioteca estática
```