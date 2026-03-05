# Plano de Migração - Estrutura Organizada

## 📋 Fases de Implementação

### Fase 1: Preparação (0-2 dias)
- [ ] Revisar estrutura proposta
- [ ] Fazer backup completo do projeto
- [ ] Criar branch `refactor/reorganize-structure`
- [ ] Documentar estado atual (imports, dependências)

### Fase 2: Criar Estrutura Base (1-2 dias)
- [ ] Criar diretórios:
  ```
  src/
  ├── core/
  ├── graphics/
  ├── renderer/
  ├── scene/
  ├── physics/
  ├── input/
  ├── world/
  ├── ui/
  ├── math/
  ├── utils/
  └── third_party/
  
  include/fractal_engine/
  ├── core/
  ├── graphics/
  ├── renderer/
  ├── scene/
  ├── physics/
  ├── input/
  ├── world/
  ├── ui/
  ├── math/
  └── utils/
  
  docs/
  ├── ARCHITECTURE.md ✓
  ├── STYLE_GUIDE.md ✓
  ├── BUILD.md
  └── CONTRIBUTING.md
  ```

- [ ] Mover includes atuais para `include/fractal_engine/`:
  ```
  glad/ -> include/third_party/glad/
  GLFW/ -> include/third_party/GLFW/
  glm/ -> include/third_party/glm/
  assimp/ -> include/third_party/assimp/
  SDL3/ -> include/third_party/SDL3/
  KHR/ -> include/third_party/KHR/
  lua/ -> include/third_party/lua/
  stb_image/ -> include/third_party/stb_image/
  ```

### Fase 3: Mover Sources Gradualmente (3-5 dias)

**Passo 1: Core**
- [ ] Mover `Main.cpp` → `src/main.cpp`
- [ ] Mover `glad.c` → `src/third_party/glad.c`
- [ ] Criar headers base em `include/fractal_engine/core/`

**Passo 2: Graphics (baixa dependência)**
- [ ] Mover `Shader.cpp` → `src/graphics/`
- [ ] Mover `Texture.cpp`, `TextureLoader.cpp` → `src/graphics/`
- [ ] Mover `Camera.cpp` → `src/graphics/`
- [ ] Criar headers correspondentes

**Passo 3: Renderer**
- [ ] Criar `src/renderer/` com renderização específica
- [ ] Separar lógica de render do core

**Passo 4: Scene & Entities**
- [ ] Mover arquivos de entidades → `src/scene/`
- [ ] Implementar pattern de componentes

**Passo 5: Physics**
- [ ] Organizar física em `src/physics/`

**Passo 6: Input**
- [ ] Mover `Input.cpp`, `Player.cpp` → `src/input/`

**Passo 7: World (Chunks)**
- [ ] Mover `Chunk.cpp` → `src/world/`

**Passo 8: UI & Utils**
- [ ] Mover UI → `src/ui/`
- [ ] Mover utilitários → `src/utils/`
- [ ] Criar Logger, Timer, FileSystem

### Fase 4: Refatorar CMakeLists.txt (2-3 dias)
- [ ] Criar CMakeLists.txt modular
- [ ] Adicionar CMakeLists.txt para cada módulo
- [ ] Testar compilação incremental
- [ ] Remover warnings

### Fase 5: Adicionar Namespaces (2-3 dias)
- [ ] Adicionar namespace `fractal_engine` globalmente
- [ ] Adicionar sub-namespaces (graphics, renderer, scene, etc)
- [ ] Atualizar includes e usages
- [ ] Testar compilação

### Fase 6: Refatorar Includes (1-2 dias)
- [ ] Usar include guards ou `#pragma once`
- [ ] Seguir padrão de includes (stdlib, external, engine)
- [ ] Remover includes desnecessários
- [ ] Forward declarations onde possível

### Fase 7: Documentação & Testes (1-2 dias)
- [ ] Criar BUILD.md (instruções de build)
- [ ] Criar CONTRIBUTING.md
- [ ] Testar compilação e Link
- [ ] Documentar estrutura final

### Fase 8: Integração Contínua (opcional, 1-2 dias)
- [ ] Adicionar GitHub Actions para build automático
- [ ] Adicionar linter/formatter (clang-format)
- [ ] Setup de testes (GoogleTest)

---

## 🔧 Estrutura de Pasta Detalhada

```
fractal-engine/
│
├── 📄 CMakeLists.txt                 # Raiz
├── 📄 README.md
├── 📄 LICENSE
├── 📄 .gitignore
│
├── 📂 src/                           # Implementação
│   ├── CMakeLists.txt                # Agrega todos os módulos
│   ├── main.cpp                      # Entry point
│   │
│   ├── 📂 core/
│   │   ├── CMakeLists.txt            # Lista sources deste módulo
│   │   ├── Engine.cpp
│   │   ├── Window.cpp
│   │   └── Application.cpp
│   │
│   ├── 📂 graphics/
│   │   ├── CMakeLists.txt
│   │   ├── Shader.cpp
│   │   ├── Texture.cpp
│   │   ├── TextureLoader.cpp
│   │   ├── Camera.cpp
│   │   └── Material.cpp
│   │
│   ├── 📂 renderer/
│   │   ├── CMakeLists.txt
│   │   ├── OpenGLRenderer.cpp
│   │   ├── RenderPass.cpp
│   │   └── Mesh.cpp
│   │
│   ├── 📂 scene/
│   │   ├── CMakeLists.txt
│   │   ├── Scene.cpp
│   │   ├── Entity.cpp
│   │   └── Transform.cpp
│   │
│   ├── 📂 physics/
│   │   ├── CMakeLists.txt
│   │   ├── PhysicsEngine.cpp
│   │   ├── Rigidbody.cpp
│   │   └── Collider.cpp
│   │
│   ├── 📂 input/
│   │   ├── CMakeLists.txt
│   │   ├── InputManager.cpp
│   │   ├── Player.cpp
│   │   └── Input.cpp
│   │
│   ├── 📂 world/
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
│   └── 📂 third_party/
│       ├── CMakeLists.txt
│       └── glad.c
│
├── 📂 include/                       # Headers públicos
│   └── 📂 fractal_engine/            # Namespace base
│       ├── FractalEngine.h            # Agregador (inclui tudo)
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
│       │   ├── MathUtils.h
│       │   └── Vector.h
│       │
│       └── 📂 utils/
│           ├── Logger.h
│           ├── Timer.h
│           └── FileSystem.h
│
├── 📂 include/third_party/           # Libs externas (já existentes)
│   ├── assimp/
│   ├── glad/
│   ├── GLFW/
│   ├── glm/
│   ├── KHR/
│   ├── lua/
│   ├── SDL3/
│   └── stb_image/
│
├── 📂 assets/                        # Recursos
│   ├── 📂 shaders/
│   │   ├── 📂 vertex/
│   │   └── 📂 fragment/
│   ├── 📂 textures/
│   ├── 📂 models/
│   ├── 📂 scenes/
│   └── 📂 sounds/
│
├── 📂 tests/                         # Testes (futuro)
│   ├── CMakeLists.txt
│   ├── 📂 unit/
│   └── 📂 integration/
│
├── 📂 docs/                          # Documentação
│   ├── README.md
│   ├── STRUCTURE.md          # esta com você!
│   ├── ARCHITECTURE.md        # esta com você!
│   ├── STYLE_GUIDE.md         # esta com você!
│   ├── BUILD.md               # TODO
│   ├── CONTRIBUTING.md        # TODO
│   └── 📂 images/             # Diagramas, etc
│
├── 📂 build/                         # Build output (gerado)
├── 📂 lib/                           # Bibliotecas compiladas
│
└── 📄 .gitignore
```

---

## 🎯 Checklist de Migraçao Passo-a-Passo

Ao fazer cada movimento:

1. **Antes de mover arquivo**
   - [ ] Encontre TODAS as referências a ele
   - [ ] Documente dependências
   - [ ] Prepare novo caminho

2. **Ao mover arquivo**
   - [ ] Copie para novo local
   - [ ] Atualize path absoluto de includes
   - [ ] Compile e teste
   - [ ] Delete original apenas se passar

3. **Depois de mover arquivo**
   - [ ] Atualize CMakeLists.txt
   - [ ] Atualize includes em ficheiros que incluem ele
   - [ ] Teste compilação completa
   - [ ] Commite com mensagem descritiva

---

## 📝 Exemplo de Commit Messages

```
refactor(structure): organize src/graphics module

- Move Shader.cpp, Texture.cpp to src/graphics/
- Create header files in include/fractal_engine/graphics/
- Update CMakeLists.txt with new paths
- Add namespace fractal_engine::graphics

Related: #1 (refactor project structure)
```

---

## ⚠️ Coisas a Evitar

❌ Mover muitos arquivos de uma vez (dificulta debug)
❌ Não testar compilação após cada passo
❌ Deixar includes absolutos em lugar de relativos
❌ Misturar namespaces e não-namespaces
❌ Fazer commits grandes (dificulta revert)
❌ Esquecer de atualizar CMakeLists.txt

---

## ✅ Próximas Etapas Recomendadas

Após a migração:

1. **Integração Contínua**
   ```
   - GitHub Actions para build automático
   - Linting com clang-format
   - Static analysis com clang-tidy
   ```

2. **Testes Automatizados**
   ```
   - GoogleTest para unit tests
   - Teste cada módulo isoladamente
   - Teste integração entre módulos
   ```

3. **Documentação de API**
   ```
   - Doxygen para gerar docs dos headers
   - Exemplos de uso para cada módulo
   - Architecture decision records (ADR)
   ```

4. **Performance Profiling**
   ```
   - Adicionar instrumentação (timing)
   - Uso de profiler (perf, VTune)
   - Benchmarks de rendering
   ```

5. **Escalabilidade Futura**
   ```
   - Suporte a plugins (dinâmico)
   - Serialização de dados (JSON, binary)
   - Asset pipeline (compilação off-line)
   ```
