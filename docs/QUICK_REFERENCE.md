# 🎯 Quick Reference - Estrutura & Convenções

## 📁 Estrutura em Uma Olhada

```
fractal-engine/
├── src/                    → Implementação (.cpp)
│   ├── main.cpp
│   ├── core/               → Loop principal
│   ├── graphics/           → Shaders, texturas, material
│   ├── renderer/           → Renderização OpenGL
│   ├── scene/              → Entidades, componentes
│   ├── physics/            → Física
│   ├── input/              → Input, controle
│   ├── world/              → Chunks
│   ├── ui/                 → Interface
│   ├── math/               → Math utils
│   ├── utils/              → Logger, Timer, FileSystem
│   └── third_party/        → glad.c (interno)
│
├── include/                → Headers públicos (.h)
│   ├── fractal_engine/     [NAMESPACE]
│   │   ├── core/
│   │   ├── graphics/
│   │   ├── renderer/
│   │   ├── scene/
│   │   ├── physics/
│   │   ├── input/
│   │   ├── world/
│   │   ├── ui/
│   │   ├── math/
│   │   ├── utils/
│   │   └── FractalEngine.h [Agregador]
│   │
│   └── third_party/        [Libs externas]
│       ├── GLFW/
│       ├── glm/
│       ├── assimp/
│       ├── SDL3/
│       └── ...
│
├── assets/                 → Recursos
│   ├── shaders/
│   ├── textures/
│   ├── models/
│   └── sounds/
│
├── tests/                  → Testes (futuro)
├── docs/                   → Documentação
├── build/                  → Output (gerado)
├── lib/                    → Libs compiladas
└── CMakeLists.txt          → Build
```

---

## 🏷️ Convenções de Nomenclatura

```
┏━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━┓
┃ Elemento             ┃ Padrão        ┃ Exemplo              ┃
┡━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━┩
│ Namespace            │ snake_case    │ fractal_engine       │
│ Classe / Struct      │ PascalCase    │ PlayerController     │
│ Função / Método      │ camelCase     │ updateTransform()    │
│ Variável Local       │ camelCase     │ borderSize           │
│ Membro (privado)     │ m_camelCase   │ m_position           │
│ Constante            │ UPPER_SNAKE   │ MAX_VERTICES         │
│ Enum Class           │ PascalCase    │ RenderMode           │
│ Enum Value           │ PascalCase    │ RenderMode::Opaque   │
│ Arquivo .h / .cpp    │ PascalCase    │ Shader.h / Shader.cpp│
└────────────────────┴──────────────────┴──────────────────────┘
```

---

## 💻 Exemplo de Estrutura (Módulo Completo)

### Header (`include/fractal_engine/graphics/Texture.h`)
```cpp
#pragma once

#include <string>
#include <glm/glm.hpp>

namespace fractal_engine::graphics {

class Texture {
public:
    explicit Texture(const std::string& path);
    ~Texture() = default;
    
    void bind() const;
    void unbind() const;
    
    uint32_t getId() const { return m_id; }
    glm::vec2 getSize() const { return m_size; }
    
private:
    uint32_t m_id = 0;
    glm::vec2 m_size = {};
    std::string m_path;
};

} // namespace fractal_engine::graphics
```

### Implementação (`src/graphics/Texture.cpp`)
```cpp
#include "fractal_engine/graphics/Texture.h"
#include "fractal_engine/utils/Logger.h"
#include <stb_image/stb_image.h>

namespace fractal_engine::graphics {

Texture::Texture(const std::string& path) : m_path(path) {
    // Carregar textura
    logger::info("Loaded texture: {}", path);
}

void Texture::bind() const {
    // OpenGL bind
}

void Texture::unbind() const {
    // OpenGL unbind
}

} // namespace fractal_engine::graphics
```

### CMakeLists.txt (`src/graphics/CMakeLists.txt`)
```cmake
set(GRAPHICS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Camera.cpp
)

target_sources(${PROJECT_NAME}_lib PRIVATE ${GRAPHICS_SOURCES})
```

---

## 📚 Includes Corretos

```cpp
// ✅ CORRETO: Segue ordem (stdlib, external, engine)
#include <vector>           // Standard library
#include <memory>
#include <glm/glm.hpp>      // External
#include <glfw/glfw3.h>
#include "fractal_engine/graphics/Shader.h"  // Engine (com namespace)

// ❌ ERRADO
#include "Shader.h"                          // Vago
#include <vector>
#include <glm/glm.hpp>
#include "fractal_engine/graphics/Shader.h"  // Fora de ordem
```

---

## 🔗 Dependências Entre Módulos

```
                    ┌─────────────────┐
                    │  main() / Core  │
                    └────────┬────────┘
                             │
        ┌────────┬───────────┼────────┬─────────┐
        │        │           │        │         │
    ┌───▼──┐ ┌──▼───┐ ┌─────▼──┐ ┌─▼────┐ ┌───▼──┐
    │Scene │ │Input │ │Physics │ │ UI   │ │World │
    └───┬──┘ └──────┘ └────────┘ └──────┘ └──────┘
        │
    ┌───▼──────────┐
    │  Renderer    │
    └───┬──────────┘
        │
    ┌───▼────────┐
    │ Graphics   │
    └───┬────────┘
        │
    ┌───▼───┐  ┌─────────┐
    │ Math  │  │ Utils   │
    └───────┘  └────┬────┘
                    │
            ┌───────▼────────┐
            │ Third-Party    │
            │ (GLFW,GLM,...) │
            └────────────────┘

Regra: Sem dependências circulares!
```

---

## ✅ Checklist Rápido (Antes de Commitar)

- [ ] **Nomenclatura**: Sigo as convenções acima?
- [ ] **Namespaces**: Usei `fractal_engine::*`?
- [ ] **Headers**: Estão em `include/fractal_engine/modulo/`?
- [ ] **Imports**: Ordem certa (stdlib, external, engine)?
- [ ] **Smart Pointers**: Usam `unique_ptr` ou `shared_ptr`?
- [ ] **const correctness**: Métodos const marcados?
- [ ] **Compilação**: Compila sem warnings?
- [ ] **CMakeLists**: Atualizado se .cpp novo?
- [ ] **Commits**: Pequenos, atômicos, bem descritos?
- [ ] **Testes**: Executam se houver?

---

## 🚨 Erros Comuns

```cpp
// ❌ ERRADO: Sem namespace
class Shader { };

// ✅ CORRETO
namespace fractal_engine::graphics {
    class Shader { };
}

// ❌ ERRADO: Raw pointer
Texture* texture = new Texture();
delete texture;

// ✅ CORRETO: Smart pointer
auto texture = std::make_unique<Texture>();

// ❌ ERRADO: Include vago
#include "Shader.h"

// ✅ CORRETO: Include claro
#include "fractal_engine/graphics/Shader.h"

// ❌ ERRADO: Variável sem inicialização
int m_count;

// ✅ CORRETO: Inicializada
int m_count = 0;

// ❌ ERRADO: Membros públicos
class Entity {
public:
    glm::vec3 position;  // Direto
};

// ✅ CORRETO: Getters/Setters ou públicos com m_ prefix
class Entity {
private:
    glm::vec3 m_position;
    
public:
    const glm::vec3& getPosition() const { return m_position; }
};
```

---

## 🎬 Setup Inicial de Arquivo Novo

### 1. Header (`include/fractal_engine/meu_modulo/MyClass.h`)
```cpp
#pragma once

namespace fractal_engine::meu_modulo {

class MyClass {
public:
    explicit MyClass(const std::string& name);
    ~MyClass() = default;
    
    // Métodos públicos
    
private:
    // Membros privados com m_ prefix
    std::string m_name;
};

} // namespace fractal_engine::meu_modulo
```

### 2. Implementação (`src/meu_modulo/MyClass.cpp`)
```cpp
#include "fractal_engine/meu_modulo/MyClass.h"

namespace fractal_engine::meu_modulo {

MyClass::MyClass(const std::string& name) : m_name(name) {
    // Construtor
}

} // namespace fractal_engine::meu_modulo
```

### 3. CMakeLists.txt (`src/meu_modulo/CMakeLists.txt`)
```cmake
set(MEU_MODULO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/MyClass.cpp
)

target_sources(${PROJECT_NAME}_lib PRIVATE ${MEU_MODULO_SOURCES})
```

### 4. Raiz CMakeLists.txt
```cmake
add_subdirectory(src/meu_modulo)
```

### 5. Agregador Principal
```cpp
// include/fractal_engine/FractalEngine.h
#include "fractal_engine/meu_modulo/MyClass.h"
```

---

## 📖 Leitura Recomendada

1. **Iniciante/Junior**: STYLE_GUIDE.md
2. **Mid-level**: ARCHITECTURE.md
3. **Senior/Arquiteto**: ARCHITECTURE.md + CONTRIBUTING.md
4. **Nova Feature**: ARCHITECTURE.md (verificar dependências)
5. **Refactoring**: MIGRATION_PLAN.md

---

## 🔗 Links Importantes

```
docs/
├── README.md                    ← Índice
├── STRUCTURE_VISUAL.md          ← Antes/Depois visual
├── ARCHITECTURE.md              ← Design patterns
├── STYLE_GUIDE.md               ← Convenções
├── MIGRATION_PLAN.md            ← Passo-a-passo
└── CONTRIBUTING.md              ← Como contribuir

Raiz:
├── ESTRUCTURA.md                ← Overview
└── EXECUTIVE_SUMMARY.md         ← Para PMs/Leads
```

---

## 💬 Perguntas Frequentes

**P: Onde colocar arquivo novo X?**  
R: Se é classe de gráficos → `src/graphics/` + `include/fractal_engine/graphics/`

**P: Como incluir arquivo X?**  
R: `#include "fractal_engine/modulo/Arquivo.h"`

**P: Qual namespace usar?**  
R: `namespace fractal_engine::modulo { }`

**P: Preciso adicionar nova dependência?**  
R: Coloque em `include/third_party/` e atualize CMakeLists.txt

**P: Meu código não compila!**  
R: 1) Verifique includes. 2) Verifique namespaces. 3) Verifique CMakeLists.txt

---

**Printável**: ✅ Sim (use scaling 80% em PDF)  
**Tamanho**: ~2 páginas  
**Última atualização**: Março 2026
