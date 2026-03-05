# Estrutura do Projeto - Fractal Engine

## 📐 Estrutura Recomendada (Escalável e Mantível)

```
fractal-engine/
├── CMakeLists.txt                 # CMake raiz
├── README.md
├── LICENSE
│
├── src/                           # Source files (.cpp)
│   ├── CMakeLists.txt             # CMake do src
│   ├── main.cpp                   # Entry point
│   │
│   ├── core/                      # Core do engine
│   │   ├── CMakeLists.txt
│   │   ├── Engine.cpp
│   │   ├── Window.cpp
│   │   └── Application.cpp
│   │
│   ├── graphics/                  # Tudo gráfico (separado de renderer)
│   │   ├── CMakeLists.txt
│   │   ├── Shader.cpp
│   │   ├── Texture.cpp
│   │   ├── TextureLoader.cpp
│   │   ├── Camera.cpp
│   │   └── Material.cpp
│   │
│   ├── renderer/                  # Renderização específica
│   │   ├── CMakeLists.txt
│   │   ├── OpenGLRenderer.cpp
│   │   ├── RenderPass.cpp
│   │   └── Mesh.cpp
│   │
│   ├── scene/                     # Sistema de cena
│   │   ├── CMakeLists.txt
│   │   ├── Scene.cpp
│   │   ├── Entity.cpp
│   │   └── Transform.cpp
│   │
│   ├── physics/                   # Física
│   │   ├── CMakeLists.txt
│   │   ├── PhysicsEngine.cpp
│   │   ├── Rigidbody.cpp
│   │   └── Collider.cpp
│   │
│   ├── input/                     # Sistema de input
│   │   ├── CMakeLists.txt
│   │   ├── InputManager.cpp
│   │   ├── Player.cpp
│   │   └── Input.cpp
│   │
│   ├── world/                     # World management (chunks, etc)
│   │   ├── CMakeLists.txt
│   │   ├── Chunk.cpp
│   │   └── World.cpp
│   │
│   ├── ui/                        # UI system
│   │   ├── CMakeLists.txt
│   │   └── UIManager.cpp
│   │
│   ├── math/                      # Utilitários math
│   │   ├── CMakeLists.txt
│   │   └── MathUtils.cpp
│   │
│   ├── utils/                     # Utilitários gerais
│   │   ├── CMakeLists.txt
│   │   ├── Logger.cpp
│   │   ├── Timer.cpp
│   │   └── FileSystem.cpp
│   │
│   └── third_party/               # Wrapper para libs externas
│       ├── CMakeLists.txt
│       └── glad.c
│
├── include/                       # Header files (.h, .hpp)
│   ├── fractal_engine/            # Namespace base
│   │   ├── core/
│   │   │   ├── Engine.h
│   │   │   ├── Window.h
│   │   │   └── Application.h
│   │   │
│   │   ├── graphics/
│   │   │   ├── Shader.h
│   │   │   ├── Texture.h
│   │   │   ├── Camera.h
│   │   │   └── Material.h
│   │   │
│   │   ├── renderer/
│   │   │   ├── OpenGLRenderer.h
│   │   │   ├── RenderPass.h
│   │   │   └── Mesh.h
│   │   │
│   │   ├── scene/
│   │   │   ├── Scene.h
│   │   │   ├── Entity.h
│   │   │   └── Transform.h
│   │   │
│   │   ├── physics/
│   │   │   ├── PhysicsEngine.h
│   │   │   ├── Rigidbody.h
│   │   │   └── Collider.h
│   │   │
│   │   ├── input/
│   │   │   ├── InputManager.h
│   │   │   ├── Player.h
│   │   │   └── Input.h
│   │   │
│   │   ├── world/
│   │   │   ├── Chunk.h
│   │   │   └── World.h
│   │   │
│   │   ├── ui/
│   │   │   └── UIManager.h
│   │   │
│   │   ├── math/
│   │   │   └── MathUtils.h
│   │   │
│   │   ├── utils/
│   │   │   ├── Logger.h
│   │   │   ├── Timer.h
│   │   │   └── FileSystem.h
│   │   │
│   │   └── FractalEngine.h        # Header aggregator (inclui tudo)
│   │
│   └── third_party/               # Apenas libs externas
│       ├── glad/
│       ├── glfw/
│       ├── glm/
│       ├── assimp/
│       ├── SDL3/
│       ├── KHR/
│       ├── lua/
│       └── stb_image/
│
├── assets/                        # Recursos do projeto
│   ├── shaders/
│   │   ├── vertex/
│   │   └── fragment/
│   ├── textures/
│   ├── models/
│   ├── scenes/
│   └── sounds/
│
├── tests/                         # Testes unitários (futuramente)
│   ├── CMakeLists.txt
│   ├── unit/
│   └── integration/
│
├── docs/                          # Documentação
│   ├── README.md
│   ├── architecture.md
│   ├── building.md
│   └── style_guide.md
│
├── build/                         # Build output (gerado, não commitar)
├── lib/                           # Bibliotecas compiladas
└── .gitignore
```

---

## 📋 Principais Benefícios desta Estrutura

### 1. **Escalabilidade**
- ✅ Fácil adicionar novos módulos (basta criar pasta + CMakeLists.txt)
- ✅ Suporta crescimento até projetos muito grandes
- ✅ Cada módulo é independente

### 2. **Manutenibilidade**
- ✅ Headers espelham a estrutura de sources
- ✅ Namespace único (`fractal_engine::`) evita conflitos
- ✅ Dependências claras entre módulos
- ✅ CMake modular (cada subsistema tem seu CMakeLists.txt)

### 3. **Organização Clara**
- ✅ `core/` = fundamentals do engine
- ✅ `graphics/` = tudo gráfico-relacionado
- ✅ `renderer/` = renderização específica
- ✅ `world/` = sistemas de mundo (chunks, etc)
- ✅ `scene/` = ECS ou scene graph
- ✅ `third_party/` = bibliotecas externas separadas

### 4. **Boas Práticas**
- ✅ Header-include agregador (`FractalEngine.h`) para facilitar incluyões
- ✅ Separação clara de `src/` (implementação) e `include/` (contratos)
- ✅ Diretório `docs/` para manutenção de documentação
- ✅ `tests/` separado (pronto para crescer)

---

## 🔧 Mudanças de Nomenclatura

| Atual | Novo | Razão |
|-------|------|-------|
| `plataform/` | Removido (ou `platform/`) | Typo + conteúdo pode ir para `core/` ou `input/` |
| `objects/` | Removido (vai para `scene/` via ECS) | Mais genérico e organizado |
| Sem namespace | `fractal_engine::` | Evita conflitos globais |
| Headers espalhados | Espelho de `src/` | Fácil localizar headers |

---

## 📝 Primeiros Passos para Migração

1. **Criar estrutura base** de pastas
2. **Mover headers** para `include/fractal_engine/`
3. **Organizar sources** por módulo
4. **Criar CMakeLists.txt** para cada subsistema
5. **Refatorar includes** (usar paths relativos claros)
6. **Adicionar namespaces** (`fractal_engine::`)
7. **Testar compilação** e ajustar

---

## 🎯 Próximos Passos (Futuros)

- Adicionar testes com GoogleTest ou Catch2
- Documentação técnica de arquitetura
- Guide de style para código
- CI/CD (GitHub Actions)
- Ferramentas de build (vcpkg, conan)
