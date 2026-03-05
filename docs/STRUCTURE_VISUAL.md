# 🗺️ Mapa Visual da Estrutura

## Antes (Atual)

```
fractal-engine/
├── src/
│   ├── core/
│   ├── math/
│   ├── objects/          ❌ Genérico demais
│   ├── physics/
│   ├── plataform/        ❌ Typo, conteúdo disperso
│   ├── renderer/         ⚠️ Misturado com graphics
│   ├── scene/
│   ├── ui/
│   ├── utils/
│   ├── shaders/          ❌ Deveria estar em assets/
│   ├── assets/
│   ├── Main.cpp          ❌ Vago, deveria ser main.cpp
│   ├── glad.c             ⚠️ Misturado com sources
│   └── Something.txt      ❌ Arquivo solto
│
├── include/
│   ├── assimp/           ⚠️ Libs externas aqui
│   ├── glad/
│   ├── GLFW/
│   ├── glm/
│   ├── KHR/
│   ├── lua/
│   ├── SDL3/
│   └── stb_image/        ⚠️ Sem namespace do projeto
│
└── Assets/               ❌ Pasta vazia/subutilizada
```

**Problemas:**
- ❌ Headers espalhados sem organização clara
- ❌ Libs externas misturadas com headers do projeto
- ❌ Sem namespace global
- ❌ Estrutura não escala bem
- ❌ Difícil adicionar novos módulos
- ⚠️ Alguns arquivos em lugar errado

---

## Depois (Proposto)

```
fractal-engine/
│
├── 📁 src/                            [IMPLEMENTAÇÃO]
│   ├── main.cpp                       ← Entry point único
│   │
│   ├── 📂 core/
│   │   ├── CMakeLists.txt
│   │   ├── Engine.cpp                 ← Loop principal
│   │   ├── Window.cpp
│   │   └── Application.cpp
│   │
│   ├── 📂 graphics/                   [NOVA ORGANIZAÇÃO]
│   │   ├── CMakeLists.txt
│   │   ├── Shader.cpp                 ← Mover de core
│   │   ├── Texture.cpp
│   │   ├── TextureLoader.cpp
│   │   ├── Camera.cpp
│   │   └── Material.cpp
│   │
│   ├── 📂 renderer/                   [SEPARADO]
│   │   ├── CMakeLists.txt
│   │   ├── OpenGLRenderer.cpp         ← Renderização específica
│   │   ├── RenderPass.cpp
│   │   └── Mesh.cpp
│   │
│   ├── 📂 scene/
│   │   ├── CMakeLists.txt
│   │   ├── Scene.cpp
│   │   ├── Entity.cpp                 ← ECS pattern
│   │   └── Transform.cpp
│   │
│   ├── 📂 physics/
│   │   ├── CMakeLists.txt
│   │   ├── PhysicsEngine.cpp
│   │   ├── Rigidbody.cpp
│   │   └── Collider.cpp
│   │
│   ├── 📂 input/                      [NOVO MÓDULO]
│   │   ├── CMakeLists.txt
│   │   ├── InputManager.cpp
│   │   ├── Player.cpp
│   │   └── Input.cpp
│   │
│   ├── 📂 world/                      [NOVO MÓDULO]
│   │   ├── CMakeLists.txt
│   │   ├── Chunk.cpp
│   │   └── World.cpp
│   │
│   ├── 📂 ui/
│   │   ├── CMakeLists.txt
│   │   └── UIManager.cpp
│   │
│   ├── 📂 math/
│   │   ├── CMakeLists.txt
│   │   └── MathUtils.cpp
│   │
│   ├── 📂 utils/
│   │   ├── CMakeLists.txt
│   │   ├── Logger.cpp
│   │   ├── Timer.cpp
│   │   └── FileSystem.cpp
│   │
│   └── 📂 third_party/                [LIBS INTERNAS]
│       ├── CMakeLists.txt
│       └── glad.c
│
├── 📁 include/                        [HEADERS PÚBLICOS]
│   └── 📂 fractal_engine/             [NAMESPACE]
│       ├── FractalEngine.h            ← Agregador mestre
│       │
│       ├── 📂 core/
│       │   ├── Engine.h
│       │   ├── Window.h
│       │   └── Application.h
│       │
│       ├── 📂 graphics/
│       │   ├── Shader.h
│       │   ├── Texture.h
│       │   ├── Camera.h
│       │   └── Material.h
│       │
│       ├── 📂 renderer/
│       │   ├── OpenGLRenderer.h
│       │   ├── RenderPass.h
│       │   └── Mesh.h
│       │
│       ├── 📂 scene/
│       │   ├── Scene.h
│       │   ├── Entity.h
│       │   ├── Component.h
│       │   └── Transform.h
│       │
│       ├── 📂 physics/
│       │   ├── PhysicsEngine.h
│       │   ├── Rigidbody.h
│       │   └── Collider.h
│       │
│       ├── 📂 input/
│       │   ├── InputManager.h
│       │   └── Input.h
│       │
│       ├── 📂 world/
│       │   ├── Chunk.h
│       │   └── World.h
│       │
│       ├── 📂 ui/
│       │   └── UIManager.h
│       │
│       ├── 📂 math/
│       │   └── MathUtils.h
│       │
│       └── 📂 utils/
│           ├── Logger.h
│           ├── Timer.h
│           └── FileSystem.h
│
├── 📁 include/third_party/            [LIBS EXTERNAS]
│   ├── 📂 assimp/
│   ├── 📂 glad/
│   ├── 📂 GLFW/
│   ├── 📂 glm/
│   ├── 📂 KHR/
│   ├── 📂 lua/
│   ├── 📂 SDL3/
│   └── 📂 stb_image/
│
├── 📁 assets/                         [RECURSOS]
│   ├── 📂 shaders/                    ← MOVER daqui de src/
│   │   ├── 📂 vertex/
│   │   └── 📂 fragment/
│   ├── 📂 textures/
│   ├── 📂 models/
│   ├── 📂 scenes/
│   └── 📂 sounds/
│
├── 📁 tests/                          [TESTES - FUTURO]
│   ├── CMakeLists.txt
│   ├── 📂 unit/
│   └── 📂 integration/
│
├── 📁 docs/                           [DOCUMENTAÇÃO]
│   ├── README.md                      ← Você está aqui!
│   ├── STRUCTURE.md
│   ├── ARCHITECTURE.md
│   ├── STYLE_GUIDE.md
│   ├── MIGRATION_PLAN.md
│   ├── CONTRIBUTING.md
│   ├── BUILD.md
│   └── 📂 images/
│
├── 📁 build/                          [OUTPUT - NÃO COMMITAR]
├── 📁 lib/                            [LIBS - NÃO COMMITAR]
│
└── 📄 CMakeLists.txt                  [RAIZ - MODULARIZADO]
```

**Benefícios:**
- ✅ Headers espelham src/ (fácil localizar)
- ✅ Namespace único `fractal_engine::` (sem conflitos)
- ✅ Separação clara entre público (headers) e privado (implementation)
- ✅ Módulos independentes (adição fácil)
- ✅ CMake modular (cada módulo na sua)
- ✅ Escala bem com projeto crescendo
- ✅ Manutenção distribuída
- ✅ Claro quem depende de quem

---

## Migração Passo-a-Passo (Resumido)

### Fase 1: Estrutura Base
```bash
# Criar pastas
mkdir src/{core,graphics,renderer,scene,physics,input,world,ui,math,utils,third_party}
mkdir include/fractal_engine/{core,graphics,renderer,scene,physics,input,world,ui,math,utils}

# Libs externas
mv include/{assimp,glad,GLFW,glm,KHR,lua,SDL3,stb_image} include/third_party/
```

### Fase 2: Mover Sources
```bash
# Core
mv src/Main.cpp src/main.cpp

# Graphics
cp src/core/Shader.cpp src/graphics/
cp src/core/Texture.cpp src/graphics/
# ... etc

# Helpers (input/world/etc)
mv src/Input.cpp src/input/
mv src/Player.cpp src/input/
mv src/Chunk.cpp src/world/
# ... etc
```

### Fase 3: Criar Headers
```bash
# Para cada .cpp, criar .h correspondente em include/
touch include/fractal_engine/graphics/Shader.h
touch include/fractal_engine/graphics/Texture.h
# ... etc
```

### Fase 4: Refatorar Includes
```cpp
// Antes
#include "Shader.h"
#include <glm/glm.hpp>

// Depois
#include "fractal_engine/graphics/Shader.h"
#include <glm/glm.hpp>
```

### Fase 5: Adicionar Namespaces
```cpp
// Antes
class Shader { };

// Depois
namespace fractal_engine::graphics {
    class Shader { };
}
```

### Fase 6: Atualizar CMake
```cmake
# src/CMakeLists.txt (novo, agregador)
add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(renderer)
# ... etc

# src/graphics/CMakeLists.txt (novo, descreve módulo)
target_sources(${PROJECT_NAME}_lib PRIVATE
    Shader.cpp
    Texture.cpp
    # ... etc
)
```

---

## Resumo de Mudanças

| Aspecto | Antes | Depois |
|---------|-------|--------|
| **Namespace** | Nenhum | `fractal_engine::*` |
| **Headers** | Espalhados | Em `include/fractal_engine/` |
| **Libs Ext.** | Em `include/` | Em `include/third_party/` |
| **Módulos** | 10 (confuso) | 10 (bem organizados) |
| **CMake** | Um único | Modular |
| **Entry** | `Main.cpp` | `main.cpp` |
| **Escalabilidade** | ⭐⭐ | ⭐⭐⭐⭐ |
| **Manutenibilidade** | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |

---

## 📍 Você está aqui

Leia a documentação nesta ordem:

1. **[Estrutura do Projeto](../ESTRUCTURA.md)** ← Você recebeu isto
2. **[Este Arquivo - Mapa Visual]** ← Você está aqui (para visualizar)
3. **[Arquitetura](ARCHITECTURE.md)** ← Próximo (para entender dependências)
4. **[Style Guide](STYLE_GUIDE.md)** ← Depois (para escrever código)
5. **[Plano de Migração](MIGRATION_PLAN.md)** ← Para implementar
6. **[Contribuição](CONTRIBUTING.md)** ← Para contribuir

---

Pronto para começar? 🚀
