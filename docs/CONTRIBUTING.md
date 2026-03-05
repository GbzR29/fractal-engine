# Guia de Contribuição - Fractal Engine

Obrigado por querer contribuir! Este documento explica como manter a qualidade e organização do projeto.

## 📋 Antes de Começar

1. **Leia a Documentação**
   - [ARCHITECTURE.md](ARCHITECTURE.md) - Entenda a estrutura
   - [STYLE_GUIDE.md](STYLE_GUIDE.md) - Siga as convenções
   - [MIGRATION_PLAN.md](MIGRATION_PLAN.md) - Compreenda o layout

2. **Setup Local**
   ```bash
   git clone https://github.com/seu-user/fractal-engine.git
   cd fractal-engine
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

3. **Crie uma Branch**
   ```bash
   git checkout -b feature/minha-feature
   # ou
   git checkout -b fix/meu-bug
   # ou
   git checkout -b refactor/modulo-xyz
   ```

---

## 🎯 Padrões de Branch

```
feature/     - Nova funcionalidade
fix/         - Correção de bug
refactor/    - Refatoração (sem mudança de behavior)
docs/        - Apenas documentação
perf/        - Otimizações
test/        - Adição de testes
ci/          - CI/CD changes
chore/       - Manutenção geral
```

Exemplos:
- `feature/component-system`
- `fix/memory-leak-shader`
- `refactor/renderer-module`
- `docs/api-documentation`
- `perf/mesh-batching`

---

## 💻 Padrões de Código

### Style Guide
Siga [STYLE_GUIDE.md](STYLE_GUIDE.md) quanto a:
- Nomes de variáveis/functions/classes
- Formatação e indentação
- const correctness
- Smart pointers (não raw pointers)

### Exemplo Correto

```cpp
// include/fractal_engine/graphics/Mesh.h
#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace fractal_engine::graphics {

class Mesh {
public:
    Mesh(const std::string& name);
    ~Mesh() = default;
    
    void addVertex(const glm::vec3& position);
    void addIndex(uint32_t index);
    
    size_t getVertexCount() const { return m_vertices.size(); }
    bool isEmpty() const { return m_vertices.empty(); }
    
private:
    std::vector<glm::vec3> m_vertices;
    std::vector<uint32_t> m_indices;
    std::string m_name;
};

} // namespace fractal_engine::graphics
```

```cpp
// src/graphics/Mesh.cpp
#include "fractal_engine/graphics/Mesh.h"
#include "fractal_engine/utils/Logger.h"

namespace fractal_engine::graphics {

Mesh::Mesh(const std::string& name) 
    : m_name(name) {
    logger::info("Created mesh: {}", name);
}

void Mesh::addVertex(const glm::vec3& position) {
    m_vertices.push_back(position);
}

void Mesh::addIndex(uint32_t index) {
    if (index >= m_vertices.size()) {
        logger::warn("Index out of bounds: {} >= {}", index, m_vertices.size());
        return;
    }
    m_indices.push_back(index);
}

} // namespace fractal_engine::graphics
```

---

## 📦 Estrutura de Commits

### Commits Atômicos
Cada commit deve ser uma **mudança lógica única e completa**:

**Bom:**
```
✅ Add Mesh class with vertex/index management
✅ Fix memory leak in TextureLoader
✅ Rename Entity::mPosition to Entity::m_position
✅ Update CMakeLists.txt for graphics module
```

**Ruim:**
```
❌ Random fixes and improvements
❌ Fixed stuff
❌ WIP: massive refactoring (200+ files)
❌ Fixed compilation error (sem detalhe)
```

### Formato de Commit Message

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Exemplos:**

```
feat(scene): add component system to entities

Implement a flexible component-based architecture:
- Add Component base class for inheritance
- Implement getComponent<T>() template
- Add components to Entity class
- Include examples in documentation

Closes #42
```

```
fix(renderer): prevent shader compile error on missing include

The ShaderCompiler was not checking if all includes existed
before attempting compilation. Added validation check.

Fixes #123
```

```
refactor(graphics): reorganize shader-related code

Move Shader and Material classes to graphics module:
- Move Shader.cpp from core/ to graphics/
- Create proper header in include/fractal_engine/graphics/
- Update all imports
- Update CMakeLists.txt

BREAKING CHANGE: Include path changed from
  #include "Shader.h"
to
  #include "fractal_engine/graphics/Shader.h"
```

---

## 🧪 Testes

Antes de fazer commit:

```bash
# Compilar
cd build
cmake --build .

# Testar (quando implementado)
ctest
```

### Adicionando Testes

Se você adiciona uma feature significativa, considere adicionar testes:

```cpp
// tests/unit/test_mesh.cpp
#include <gtest/gtest.h>
#include "fractal_engine/graphics/Mesh.h"

using namespace fractal_engine::graphics;

class MeshTest : public ::testing::Test {
protected:
    Mesh mesh{"test"};
};

TEST_F(MeshTest, AddVertexIncreasesCount) {
    EXPECT_EQ(mesh.getVertexCount(), 0);
    mesh.addVertex({0, 0, 0});
    EXPECT_EQ(mesh.getVertexCount(), 1);
}

TEST_F(MeshTest, IsEmptyBeforeAddingVertices) {
    EXPECT_TRUE(mesh.isEmpty());
}
```

---

## 📝 Pull Request

### Checklist

- [ ] Branch é atualizada com `main`
- [ ] Código segue STYLE_GUIDE.md
- [ ] Não há warnings de compilação
- [ ] Commits são atômicos e bem descritos
- [ ] Headers estão em `include/fractal_engine/`
- [ ] Implementation em `src/`
- [ ] Namespaces usados corretamente
- [ ] CMakeLists.txt atualizado (se necessário)
- [ ] Testes passam (se aplicável)
- [ ] Documentação atualizada (se necessário)

### Descrição do PR

```markdown
## Descrição
Breve descrição do que foi implementado.

## Relacionado
Fixes #123
Relates to #456

## Tipo de Mudança
- [ ] Bug fix
- [ ] Nueva funcionalidad
- [ ] Breaking change
- [ ] Refactoring
- [ ] Documentação

## Como Testar
Passos para testar a mudança:
1. Clone a branch
2. Compile com `cmake --build build`
3. Execute `./build/bin/FractalEngine.exe`
4. Verifique que X funciona corretamente

## Screenshots (se aplicável)
[Adicionar screenshots/GIFs se relevante]

## Notas Adicionais
Contexto extra que reviewer precisa saber.
```

---

## 🚀 Adicionando um Novo Módulo

Se você quer adicionar um módulo completamente novo:

### 1. Criar Estrutura
```bash
# Criar pastas
mkdir src/meu_modulo
mkdir include/fractal_engine/meu_modulo
```

### 2. Header Público
```cpp
// include/fractal_engine/meu_modulo/ModoModule.h
#pragma once

namespace fractal_engine::meu_modulo {
    // API pública
}
```

### 3. Implementação
```cpp
// src/meu_modulo/Module.cpp
#include "fractal_engine/meu_modulo/Module.h"

namespace fractal_engine::meu_modulo {
    // implementação
}
```

### 4. CMakeLists.txt
```cmake
# src/meu_modulo/CMakeLists.txt
set(MEU_MODULO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Module.cpp
)

target_sources(${PROJECT_NAME}_lib PRIVATE ${MEU_MODULO_SOURCES})
```

### 5. Adicionar ao CMake Raiz
```cmake
# CMakeLists.txt (raiz)
add_subdirectory(src/meu_modulo)
```

### 6. Atualizar Agregador
```cpp
// include/fractal_engine/FractalEngine.h
#include "fractal_engine/meu_modulo/Module.h"
```

---

## 🐛 Reportando Bugs

Use GitHub Issues com:

```markdown
### Descrição
O que esperava vs. o que aconteceu.

### Reprodução
Passos para reproduzir:
1. ...
2. ...
3. ...

### Ambiente
- OS: Windows 11
- Compiler: MSVC 19.x
- CMake: 3.20+

### Logs/Errors
```
[cole stack trace ou logs]
```

### Possível Solução
[Se souber da solução, sugira]
```

---

## 📚 Recursos Úteis

- [ARCHITECTURE.md](ARCHITECTURE.md) - Padrões de design
- [STYLE_GUIDE.md](STYLE_GUIDE.md) - Convenções de código
- [MIGRATION_PLAN.md](MIGRATION_PLAN.md) - Estrutura de pastas
- [BUILD.md](BUILD.md) - Instruções de compilação (TODO)

---

## ❓ Dúvidas?

Abra uma issue com label `question` ou me envie uma mensagem!

Obrigado por contribuir para o Fractal Engine! 🚀
