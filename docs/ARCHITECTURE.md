# Guia de Arquitetura - Fractal Engine

## Diagrama de Dependências (Camadas)

```
┌─────────────────────────────────────────────────────┐
│                Application (main.cpp)                │
├─────────────────────────────────────────────────────┤
│                    Core Engine                       │
│          (Inicialização, Loop Principal)             │
├─────────────────────────────────────────────────────┤
│         Input    │ Scene  │  UI   │  World          │
│      (Controla)  │(Organiza)(Renderiza)(Chunk)      │
├─────────────────────────────────────────────────────┤
│    Physics        │        Renderer        │         │
│  (Simulação)      │  (OpenGL, RenderPass) │         │
├─────────────────────────────────────────────────────┤
│              Graphics (Shaders, Texturas)            │
├─────────────────────────────────────────────────────┤
│          Math (Vectors, Matrices, Utils)             │
├─────────────────────────────────────────────────────┤
│    Utils (Logger, Timer, FileSystem, etc)            │
├─────────────────────────────────────────────────────┤
│    Third-Party (GLFW, OpenGL, GLM, SDL, Lua)        │
└─────────────────────────────────────────────────────┘
```

### Regra de Ouro: **Acoplamento Baixo, Coesão Alta**

---

## Relações Entre Módulos

### 1. **Core**
- **Dependências**: utils, math, third_party
- **Dependentes**: todos os outros
- **Responsabilidade**: Inicialização, loop principal, lifecycle

```cpp
class Engine {
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<PhysicsEngine> m_physics;
    std::unique_ptr<InputManager> m_input;
    
    void run();  // Main loop
};
```

---

### 2. **Graphics**
- **Dependências**: math, utils, third_party (OpenGL, GLM)
- **Dependentes**: renderer, world
- **Responsabilidade**: Abstrações low-level de gráficos

```cpp
namespace graphics {
    class Shader { /* ... */ };
    class Texture { /* ... */ };
    class Material { /* ... */ };
    class Camera { /* ... */ };
}
```

**Não depende de**: scene, physics, input, world

---

### 3. **Renderer**
- **Dependências**: graphics, math, utils, core
- **Dependentes**: core, scene
- **Responsabilidade**: Renderização, passes, batch rendering

```cpp
namespace renderer {
    class OpenGLRenderer {
        void render(const Scene& scene);  // Renderiza cena inteira
        void renderEntity(const Entity& entity);
    };
}
```

**Não depende de**: physics, input (apenas lê dados)

---

### 4. **Scene**
- **Dependências**: math, utils, core
- **Dependentes**: core, physics, input, world, renderer
- **Responsabilidade**: Organização de entidades, transformações

```cpp
namespace scene {
    class Entity {
        Transform m_transform;
        std::vector<Component*> m_components;
    };
    
    class Scene {
        std::vector<std::unique_ptr<Entity>> m_entities;
        Entity* createEntity();
    };
}
```

**Nota**: Use padrão de Componentes (ECS-lite)

---

### 5. **Physics**
- **Dependências**: math, utils, scene (componentes de physics)
- **Dependentes**: core, scene
- **Responsabilidade**: Simulação física

```cpp
namespace physics {
    class Rigidbody {
        float m_mass;
        glm::vec3 m_velocity;
        glm::vec3 m_acceleration;
    };
    
    class PhysicsEngine {
        void step(float deltaTime);
    };
}
```

---

### 6. **Input**
- **Dependências**: core, utils, third_party (GLFW)
- **Dependentes**: core, scene (via controllers)
- **Responsabilidade**: Captura e distribuição de input

```cpp
namespace input {
    class InputManager {
        void update();
        bool isKeyPressed(Key k);
        glm::vec2 getMousePosition();
    };
}
```

**Não depende de**: renderer, graphics, physics

---

### 7. **World** (Chunk System)
- **Dependências**: scene, math, utils, renderer
- **Dependentes**: core, scene
- **Responsabilidade**: Geração, gerenciamento de chunks

```cpp
namespace world {
    struct Chunk {
        std::array<Block, 16*16*16> blocks;
        glm::vec3 m_position;
    };
    
    class World {
        std::map<glm::vec3, Chunk> m_chunks;
    };
}
```

---

### 8. **UI**
- **Dependências**: graphics, math, input, utils
- **Dependentes**: core
- **Responsabilidade**: Interface de usuário

```cpp
namespace ui {
    class UIManager {
        void render();
        void handleInput(const input::InputEvent& event);
    };
}
```

---

### 9. **Math** (Utilities)
- **Dependências**: third_party (GLM)
- **Dependentes**: quase todos
- **Responsabilidade**: Funções math, helpers

```cpp
namespace math {
    glm::vec3 normalize(const glm::vec3& v);
    float distance(const glm::vec3& a, const glm::vec3& b);
    bool intersectRayAABB(const Ray& ray, const AABB& box);
}
```

---

### 10. **Utils**
- **Dependências**: third_party
- **Dependentes**: todos
- **Responsabilidade**: Logging, timing, filesystem

```cpp
namespace utils {
    namespace logger {
        void info(const std::string& msg);
        void error(const std::string& msg);
    }
    
    class Timer { /* ... */ };
    class FileSystem { /* ... */ };
}
```

---

## Pattern: Componentes (ECS-Lite)

Recomendo usar um padrão de componentes simples:

```cpp
// Base class para componentes
class Component {
public:
    virtual ~Component() = default;
    virtual void update(float deltaTime) {}
    virtual void onEnable() {}
    virtual void onDisable() {}
};

// Componentes específicos
class TransformComponent : public Component {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

class RigidbodyComponent : public Component {
    physics::Rigidbody m_rigidbody;
};

class SpriteComponent : public Component {
    graphics::Texture m_texture;
};

// Entity contém componentes
class Entity {
    std::vector<std::unique_ptr<Component>> m_components;
    
    template<typename T>
    T* getComponent() {
        for (auto& comp : m_components) {
            if (dynamic_cast<T*>(comp.get())) {
                return dynamic_cast<T*>(comp.get());
            }
        }
        return nullptr;
    }
};
```

**Vantagens**
- Baixo acoplamento
- Flexível e extensível
- Fácil de debugar
- Componíveis

---

## Ciclo de Vida da Engine

```
┌─────────────────────────────────┐
│  Engine::initialize()            │
│  - Load configs                  │
│  - Init graphics                 │
│  - Load scenes                   │
└────────────┬────────────────────┘
             │
             ▼
     ┌────────────────────┐
     │  Engine::run()     │
     │  (Main Loop)       │
     └────────┬───────────┘
              │
    ┌─────────┴─────────┐
    │                   │
    ▼                   ▼
Input::update()    Physics::update()
    │                   │
    ├──────────┬────────┤
    │          ▼        │
    │      Scene::update()
    │          │        │
    └──────────┼────────┘
               │
               ▼
        Renderer::render()
               │
               ▼
        SwapBuffers()
              (loop)
```

---

## Imports/Includes Pattern

### Header Público (include/fractal_engine/)
```cpp
// include/fractal_engine/scene/Entity.h
#pragma once

#include <glm/glm.hpp>
#include "fractal_engine/math/Transform.h"

namespace fractal_engine::scene {
    class Entity {
        // ...
    };
}
```

### Implementação (src/)
```cpp
// src/scene/Entity.cpp
#include "fractal_engine/scene/Entity.h"
#include "fractal_engine/graphics/Texture.h"
#include "fractal_engine/utils/Logger.h"

void Entity::update(float dt) {
    // ...
}
```

### Agregador Principal
```cpp
// include/fractal_engine/FractalEngine.h
#pragma once

#include "fractal_engine/core/Engine.h"
#include "fractal_engine/scene/Scene.h"
#include "fractal_engine/physics/PhysicsEngine.h"
#include "fractal_engine/graphics/Shader.h"
// ... resto dos headers

// Uso no user code:
// #include <fractal_engine/FractalEngine.h>
```

---

## Evitando Dependências Circulares

**Ruim:**
```cpp
// Engine.h inclui Renderer.h
// Renderer.h inclui Engine.h  ❌ Circular!
```

**Bom:** Use Forward Declarations
```cpp
// Engine.h
namespace fractal_engine::renderer { class Renderer; }

class Engine {
    std::unique_ptr<renderer::Renderer> m_renderer;
};
```

---

## Checklist de Arquitetura

- [ ] Cada módulo tem responsabilidade única
- [ ] Dependências fluem em uma direção (acíclicas)
- [ ] Utils é independente
- [ ] Graphics não depende de Physics
- [ ] Scene não depende de Renderer (apenas permite ser renderizada)
- [ ] Input é independente (apenas atualiza estado)
- [ ] Componentes são baixo acoplamento
- [ ] Forward declarations evitam includes desnecessários
