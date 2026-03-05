# C++ Style Guide - Fractal Engine

## Convenções de Nomenclatura

### Classes e Structs
```cpp
class PlayerController { /* ... */ };
struct Position { /* ... */ };
namespace fractal_engine { /* ... */ }
```
- PascalCase para classes, structs, tipos
- Use namespace `fractal_engine` para tudo

### Funções e Métodos
```cpp
void updateTransform();
bool isVisible() const;
void setPosition(const glm::vec3& pos);
```
- camelCase para funções e métodos
- Booleans: use prefixo `is`, `has`, `can`, etc.

### Variáveis Locais
```cpp
int vertexCount = 0;
float elapsedTime = 0.0f;
auto* renderer = getRenderer();
```
- camelCase para variáveis locais
- const quando possível

### Membros de Classe
```cpp
class Entity {
private:
    glm::vec3 m_position;
    float m_rotation = 0.0f;
    std::string m_name;
};
```
- Prefixo `m_` para membros privados
- Inicialize com valores padrão quando apropriado

### Constantes
```cpp
constexpr float MAX_SPEED = 100.0f;
const char* SHADER_VERSION = "430";
```
- UPPER_SNAKE_CASE para constantes globais

### Enums
```cpp
enum class RenderMode {
    Opaque,
    Transparent,
    Custom
};
```
- Use `enum class` (não `enum`)
- PascalCase para valores

---

## Organização de Headers

### Ordem de Includes
```cpp
// Standard library
#include <vector>
#include <memory>
#include <string>

// External libraries
#include <glm/glm.hpp>
#include <glfw/glfw3.h>

// Engine headers
#include "fractal_engine/core/Engine.h"
#include "fractal_engine/graphics/Shader.h"
```

### Guard de Include
```cpp
#ifndef FRACTAL_ENGINE_GRAPHICS_SHADER_H
#define FRACTAL_ENGINE_GRAPHICS_SHADER_H

namespace fractal_engine::graphics {
    class Shader {
        // ...
    };
} // namespace fractal_engine::graphics

#endif
```

Ou use:
```cpp
#pragma once
```

---

## Formataçao de Código

### Indentação e Espaçamento
```cpp
// Use 4 espaços ou 1 tab
class MyClass {
public:
    void publicMethod();

private:
    void privateMethod();
    int m_value = 0;
};
```

### Brace Style
```cpp
// Use K&R ou Allman (seja consistente)

// K&R
if (condition) {
    doSomething();
} else {
    doOtherThing();
}

// Para funções, preferencialmente:
void MyClass::myMethod() {
    // ...
}
```

### Linhas Longas
```cpp
// Mantenha as linhas ~100 caracteres
// Quebra linhas longas logicamente

std::vector<Entity*> entities = scene->getEntities(
    EntityType::Dynamic,
    true  // visible only
);
```

---

## Boas Práticas

### Use Smart Pointers
```cpp
// Bom
std::unique_ptr<Shader> shader = std::make_unique<Shader>();
std::shared_ptr<Texture> texture = std::make_shared<Texture>();

// Ruim
Shader* shader = new Shader();
delete shader;
```

### const Correctness
```cpp
// Bom
class GameObject {
public:
    const glm::vec3& getPosition() const;
    void setPosition(const glm::vec3& pos);
};

// Use const ref para inputs grandes
void processData(const std::vector<Vertex>& vertices);
```

### RAII Pattern
```cpp
class ScopedTimer {
public:
    ScopedTimer(const std::string& name) 
        : m_name(name) { 
        m_start = std::chrono::high_resolution_clock::now();
    }
    
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - m_start
        );
        logger::info(m_name + " took " + std::to_string(duration.count()) + "ms");
    }
    
private:
    std::string m_name;
    std::chrono::high_resolution_clock::time_point m_start;
};

// Uso
void updateScene() {
    ScopedTimer timer("Scene Update");
    // ...
} // Timer destruído e log impresso automaticamente
```

### Evite Globals
```cpp
// Ruim
Renderer* g_renderer = nullptr;  // Global state

// Bom
class Engine {
private:
    Renderer m_renderer;
    
public:
    Renderer& getRenderer() { return m_renderer; }
};
```

### Use Range-based For Loops
```cpp
// Bom
for (const auto& entity : entities) {
    entity->update(deltaTime);
}

// Ruim
for (int i = 0; i < entities.size(); ++i) {
    entities[i]->update(deltaTime);
}
```

### Logging e Debugging
```cpp
#include "fractal_engine/utils/Logger.h"

logger::info("Engine initialized");
logger::warn("Shader not found: {}", shaderPath);
logger::error("Failed to load texture: {}", texturePath);

// Debug builds
#ifdef _DEBUG
    logger::debug("Vertex count: {}", vertexCount);
    assert(shader != nullptr);
#endif
```

---

## Tabela de Referência Rápida

| Elemento | Convenção | Exemplo |
|----------|-----------|---------|
| Namespace | snake_case | `fractal_engine` |
| Classe | PascalCase | `PlayerController` |
| Função | camelCase | `updateTransform()` |
| Método const | camelCase | `getPosition() const` |
| Variável local | camelCase | `borderSize` |
| Membro privado | m_camelCase | `m_position` |
| Constante | UPPER_SNAKE_CASE | `MAX_VERTICES` |
| Enum class | PascalCase | `RenderMode` |
| Enum value | PascalCase | `RenderMode::Opaque` |

---

## Checklist Antes de Commitar

- [ ] Nomes seguem convenções
- [ ] Código bem indentado
- [ ] Smart pointers usados corretamente
- [ ] const correctness aplicado
- [ ] Sem globals (exceto constantes)
- [ ] Headers bem-organizados
- [ ] Compila sem warnings
- [ ] Comentários claros (não óbvios)
