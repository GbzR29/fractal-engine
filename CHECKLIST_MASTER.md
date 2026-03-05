# 📋 Checklist Master - Reorganização Fractal Engine

## 🎯 Objetivo
Reorganizar projeto para escalabilidade com estrutura bem-organizada e fácil manutenção.

**Duração Estimada**: 2-3 semanas  
**Pessoa Responsável**: [Nome]  
**Data Início**: [Data]  
**Data Fim**: [Data]

---

## 📑 Fase 1: Preparação (1-2 dias)

- [ ] Revisar todos documentos de estrutura
  - [ ] ESTRUCTURA.md
  - [ ] ARCHITECTURE.md
  - [ ] MIGRATION_PLAN.md

- [ ] Team reunido para alinhamento
  - [ ] Todos entendem a estrutura nova?
  - [ ] Decisões críticas aprovadas?
  - [ ] Timeline aceita?

- [ ] Backup do projeto
  - [ ] Git backup feito
  - [ ] Cópia local completa

- [ ] Setup inicial do Git
  - [ ] [ ] `git checkout -b refactor/reorganize-structure`
  - [ ] [ ] Push branch vazia (ou com docs)
  - [ ] Commitar documentação

- [ ] Preparar ambiente
  - [ ] [ ] VS Code atualizado
  - [ ] [ ] CMake atualizado
  - [ ] [ ] Compilação teste feita (deve passar)

**Responsáveis**: Lead, Setup person  
**Status**: ⏳ TODO | ⏸️ IN PROGRESS | ✅ DONE

---

## 📑 Fase 2: Criar Estrutura Base (1-2 dias)

### Criar Diretórios src/
- [ ] `mkdir src/core`
- [ ] `mkdir src/graphics`
- [ ] `mkdir src/renderer`
- [ ] `mkdir src/scene`
- [ ] `mkdir src/physics`
- [ ] `mkdir src/input`
- [ ] `mkdir src/world`
- [ ] `mkdir src/ui`
- [ ] `mkdir src/math`
- [ ] `mkdir src/utils`
- [ ] `mkdir src/third_party`

### Criar Diretórios include/
- [ ] `mkdir include/fractal_engine`
- [ ] `mkdir include/fractal_engine/{core,graphics,renderer,scene,physics,input,world,ui,math,utils}`

### Mover Libs Externas
- [ ] `mv include/assimp include/third_party/`
- [ ] `mv include/glad include/third_party/`
- [ ] `mv include/GLFW include/third_party/`
- [ ] `mv include/glm include/third_party/`
- [ ] `mv include/KHR include/third_party/`
- [ ] `mv include/lua include/third_party/`
- [ ] `mv include/SDL3 include/third_party/`
- [ ] `mv include/stb_image include/third_party/`

### Testes
- [ ] Compilação ainda passa?
- [ ] Ajustar paths se necessário
- [ ] Commit: "chore(structure): create directory structure"

**Responsáveis**: Junior dev com supervision  
**Status**: ⏳ TODO | ⏸️ IN PROGRESS | ✅ DONE

---

## 📑 Fase 3a: Mover Sources - Core (0-1 dia)

### Reorganização
- [ ] `mv src/Main.cpp src/main.cpp`
- [ ] Atualizar referências em CMakeLists.txt

### Mover Arquivos Core
- [ ] Identificar core files actuales
  - [ ] Engine.cpp (se existe)
  - [ ] Window.cpp (se existe)
  - [ ] Application.cpp (se existe)
  
- [ ] Criar `src/core/Engine.cpp` (template se não existe)
- [ ] Mover arquivos identificados a Core

### Headers
- [ ] Criar `include/fractal_engine/core/Engine.h`
- [ ] Criar headers para cada .cpp em src/core/

### CMakeLists.txt
- [ ] Criar `src/core/CMakeLists.txt`
- [ ] Atualizar `src/CMakeLists.txt` para incluir core

### Testes & Commits
- [ ] Compilação passa?
- [ ] Commit: "refactor(core): move core files to dedicated module"

**Responsáveis**: Dev 1  
**Status**: ⏳ TODO | ⏸️ IN PROGRESS | ✅ DONE

---

## 📑 Fase 3b: Mover Sources - Graphics (1 dia)

### Identificar Arquivos
- [ ] Mover `Shader.cpp` → `src/graphics/`
- [ ] Mover `Texture.cpp` → `src/graphics/`
- [ ] Mover `TextureLoader.cpp` → `src/graphics/`
- [ ] Mover `Camera.cpp` → `src/graphics/`
- [ ] Mover `Material.cpp` → `src/graphics/` (se existe)

### Headers
- [ ] `include/fractal_engine/graphics/Shader.h`
- [ ] `include/fractal_engine/graphics/Texture.h`
- [ ] `include/fractal_engine/graphics/TextureLoader.h`
- [ ] `include/fractal_engine/graphics/Camera.h`
- [ ] `include/fractal_engine/graphics/Material.h` (se necessário)

### CMakeLists.txt
- [ ] Criar `src/graphics/CMakeLists.txt`

### Testes & Commits
- [ ] Compilação passa?
- [ ] Sem warnings?
- [ ] Commit: "refactor(graphics): reorganize graphics module"

**Responsáveis**: Dev 1 ou Dev 2  
**Status**: ⏳ TODO | ⏸️ IN PROGRESS | ✅ DONE

---

## 📑 Fase 3c: Mover Sources - Renderer (0-1 dia)

### Identificar Arquivos
- [ ] Criar `src/renderer/OpenGLRenderer.cpp` (ou remombrear existentes)
- [ ] Identificar arquivos de render actuales

### Headers
- [ ] Criar headers correspondientes en `include/fractal_engine/renderer/`

### CMakeLists.txt
- [ ] Crear `src/renderer/CMakeLists.txt`

### Tests & Commits
- [ ] Compilar?
- [ ] Commit: "refactor(renderer): separate renderer module"

**Responsáveis**: Dev 2  
**Status**: ⏳ TODO | ⏸️ IN PROGRESS | ✅ DONE

---

## 📑 Fase 3d: Mover Sources - Resto (2 dias)

### Scene
- [ ] Mover scene files → `src/scene/`
- [ ] Crear headers en `include/fractal_engine/scene/`
- [ ] Crear `src/scene/CMakeLists.txt`
- [ ] [ ] Commit: "refactor(scene): organize scene module"

### Physics
- [ ] Mover physics → `src/physics/`
- [ ] Headers en `include/fractal_engine/physics/`
- [ ] [ ] Commit: "refactor(physics): organize physics module"

### Input
- [ ] Mover `Input.cpp` → `src/input/Input.cpp`
- [ ] Mover `Player.cpp` → `src/input/Player.cpp`
- [ ] Crear Input Manager si es necesario
- [ ] Headers
- [ ] [ ] Commit: "refactor(input): reorganize input module"

### World (Chunks)
- [ ] Mover `Chunk.cpp` → `src/world/`
- [ ] Crear World.cpp si necesario
- [ ] Headers
- [ ] [ ] Commit: "refactor(world): organize chunk system"

### UI
- [ ] Mover UI files → `src/ui/`
- [ ] Headers
- [ ] [ ] Commit: "refactor(ui): organize ui module"

### Math
- [ ] Mover math utilidades → `src/math/`
- [ ] Headers
- [ ] [ ] Commit: "refactor(math): organize math utilities"

### Utils
- [ ] Crear if not exists: Logger.cpp, Timer.cpp, FileSystem.cpp
- [ ] Headers en `include/fractal_engine/utils/`
- [ ] [ ] Commit: "refactor(utils): organize utilities"

### Third Party
- [ ] Mover `glad.c` → `src/third_party/`
- [ ] Crear `src/third_party/CMakeLists.txt`
- [ ] [ ] Commit: "refactor(third_party): organize glads and internals"

**Responsáveis**: Dev 1, Dev 2, Dev 3 (paralelo)  
**Status**: ⏳ TODO | ⏸️ IN PROGRESS | ✅ DONE

---

## 📑 Fase 4: Refatorar CMakeLists.txt (2 días)

### Crear CMakeLists Base
- [ ] Reescribir raíz `CMakeLists.txt`
  - [ ] Versión/descripción correcta
  - [ ] C++23
  - [ ] Flags warnings
  - [ ] add_library() principal
  - [ ] add_subdirectory() para cada módulo
  - [ ] target_include_directories()
  - [ ] target_link_libraries()

### Crear CMakeLists Módulo Agregador
- [ ] Crear `src/CMakeLists.txt`
  - [ ] add_subdirectory() para cada módulo
  - [ ] Mensaje de status

### CMakeLists por Módulo
- [ ] `src/core/CMakeLists.txt`
- [ ] `src/graphics/CMakeLists.txt`
- [ ] `src/renderer/CMakeLists.txt`
- [ ] `src/scene/CMakeLists.txt`
- [ ] `src/physics/CMakeLists.txt`
- [ ] `src/input/CMakeLists.txt`
- [ ] `src/world/CMakeLists.txt`
- [ ] `src/ui/CMakeLists.txt`
- [ ] `src/math/CMakeLists.txt`
- [ ] `src/utils/CMakeLists.txt`
- [ ] `src/third_party/CMakeLists.txt`

### Testes
- [ ] cmake .. funciona
- [ ] cmake --build . funciona
- [ ] Ejercitable genera sin errores
- [ ] Ningún warning de linking
- [ ] Commit: "chore(cmake): modularize cmake structure"

**Responsáveis**: CMake expert  
**Status**: ⏳ TODO | ⏸️ IN PROGRESS | ✅ DONE

---

## 📑 Fase 5: Refactor Includes (2 días)

### Paso a Paso
- [ ] Archivo por archivo, actualizar includes
  - [ ] Cambiar `#include "Shader.h"` → `#include "fractal_engine/graphics/Shader.h"`
  - [ ] Asegurar orden: stdlib, external, engine
  - [ ] Remove duplicates

### Headers
- [ ] Crear agregador `include/fractal_engine/FractalEngine.h`
  - [ ] Incluye todos los headers principales
  - [ ] Orden lógico

### Testes Compilación
- [ ] Después de cambios por modulo: compilar
- [ ] Commits pequeños: "refactor(includes): update module paths"

**Responsáveis**: Equipo (1 dev por módulo)  
**Status**: ⏳ TODO | ⏸️ IN PROGRESS | ✅ DONE

---

## 📑 Fase 6: Añadir Namespaces (2-3 días)

### Paso a Paso
- [ ] Archivo por archivo, envolveré en namespace
- [ ] `namespace fractal_engine::modulo { }`
- [ ] Actualizar usos locales (sin namespace)
- [ ] Commits: "refactor(namespace): add module namespaces"

### Orden Recomendado
1. [ ] third_party (no namespace)
2. [ ] math, utils (base)
3. [ ] graphics, renderer
4. [ ] scene, physics, input, world, ui
5. [ ] core (último)

### Testes
- [ ] Compilación sin errores após cada modulo
- [ ] Ningún conflicto de nombres

**Responsáveis**: Equipo distribuido  
**Status**: ⏳ TODO | ⏸️ IN PROGRESS | ✅ DONE

---

## 📑 Fase 7: Documentación & Testes (1-2 días)

### Documentación
- [ ] BUILD.md creado (instrucciones de build)
- [ ] README.md actualizado (root)
- [ ] Ejemplos de uso actualizados
- [ ] Commit: "docs: add building and usage documentation"

### Testes Finales
- [ ] Clean build (rm -rf build):
  - [ ] cmake ..
  - [ ] cmake --build .
  - [ ] ./build/bin/FractalEngine.exe funciona
  - [ ] Sin warnings
  - [ ] Sin errores

- [ ] Funcionalidad preservada:
  - [ ] Aplicación inicia
  - [ ] Ventana abre
  - [ ] Rendering funciona
  - [ ] Input funciona
  - [ ] etc.

### Merge & Deploy
- [ ] Pull request creado
- [ ] Review aprobado
- [ ] Merge a main
- [ ] Tag release

**Responsáveis**: QA + Lead  
**Status**: ⏳ TODO | ⏸️ IN PROGRESS | ✅ DONE

---

## 📑 Fase 8: Integración Continua (Opcional, 1-2 días)

- [ ] GitHub Actions configurado
- [ ] clang-format configurado
- [ ] clang-tidy configurado (opcional)
- [ ] Tests automáticos (opcional, futuro)
- [ ] Commit: "ci: add github actions and linting"

**Responsáveis**: DevOps / Senior Dev  
**Status**: ⏳ TODO | ⏸️ IN PROGRESS | ✅ DONE

---

## 🎵 Checklist Final

### Después de TODO:
- [ ] Compilación limpia (sin warnings)
- [ ] Executable funciona
- [ ] Estructura matches documentación
- [ ] Todos los namespaces estan
- [ ] CMake modular funciona
- [ ] Documentación actualizada
- [ ] Commits son atómicos y bien descritos
- [ ] PR mergeado a main
- [ ] Team capacitado en nueva estructura

### Validación:
- [ ] Nuevo dev puede navegar estructura
- [ ] Código sigue style guide
- [ ] Dependencias están claras
- [ ] Proyecto es escalable

---

## 📊 Tracking de Progreso

```
Fase 1: [████████████████████] 100% ✅
Fase 2: [████████████████░░░░] 80%  ⏳
Fase 3: [████░░░░░░░░░░░░░░░] 20%  ⏳
Fase 4: [░░░░░░░░░░░░░░░░░░░░] 0%   TODO
Fase 5: [░░░░░░░░░░░░░░░░░░░░] 0%   TODO
Fase 6: [░░░░░░░░░░░░░░░░░░░░] 0%   TODO
Fase 7: [░░░░░░░░░░░░░░░░░░░░] 0%   TODO
Fase 8: [░░░░░░░░░░░░░░░░░░░░] 0%   TODO
────────────────────────────────────
TOTAL:  [████░░░░░░░░░░░░░░░░] 20%  ⏳
```

---

## 📝 Notas

### Problemas Encontrados
- [ ] [Descripción]
- [ ] [Solución implementada]

### Decisiones Tomadas
- [ ] [Decisión 1]
- [ ] [Decisión 2]

### Aprendizajes
- [ ] [Aprendizaje 1]
- [ ] [Aprendizaje 2]

---

## 📞 Contactos

- **Lead**: [Nombre] - [Email/Phone]
- **CMake Expert**: [Nombre] - [Email]
- **Senior Dev**: [Nombre] - [Email]

---

**Documento Creado**: Março 2026  
**Última Atualización**: [Data]  
**Status Geral**: ⏳ IN PROGRESS

**Imprir?**: ✅ Sim (recomendado, 2-3 páginas)
