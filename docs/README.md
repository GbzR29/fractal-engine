# 📚 Documentação - Fractal Engine

Bem-vindo à documentação técnica do Fractal Engine! Aqui você encontra tudo que precisa para entender, contribuir e escalar o projeto.

## 📖 Índice de Documentos

### 🏗️ [Estrutura do Projeto](../ESTRUCTURA.md)
**O que**: Visão geral da organização proposta  
**Quem**: Todos os desenvolvedores  
**Quando**: Primeira coisa a ler  
**Tópicos**:
- Estrutura de pastas (escalável)
- Benefícios de cada organização
- Mudanças de nomenclatura
- Primeiros passos da migração

---

### 🎯 [Arquitetura](ARCHITECTURE.md)
**O que**: Design patterns, camadas, responsabilidades  
**Quem**: Arquitetos, desenvolvedores seniors  
**Quando**: Antes de implementar grandes features  
**Tópicos**:
- Diagrama de dependências
- Relações entre módulos
- Pattern de Componentes (ECS-lite)
- Ciclo de vida da engine
- Evitando dependências circulares

---

### 📝 [Style Guide](STYLE_GUIDE.md)
**O que**: Convenções de código e formatação  
**Quem**: Todos os desenvolvedores  
**Quando**: Ao escrever/revisar código  
**Tópicos**:
- Convenções de nomenclatura
- Organização de headers
- Formatação e indentation
- Boas práticas (smart pointers, const correctness, RAII)
- Checklist antes de commitar

---

### 🚀 [Plano de Migração](MIGRATION_PLAN.md)
**O que**: Passos práticos para reorganizar o projeto  
**Quem**: Team leads, refactoring assignments  
**Quando**: Durante sprint de reorganização  
**Tópicos**:
- 8 fases de implementação
- Checklist passo-a-passo
- Estrutura detalhada
- Exemplo de commits
- Próximas etapas após migração

---

### 🤝 [Guia de Contribuição](CONTRIBUTING.md)
**O que**: Como contribuir mantendo qualidade  
**Quem**: Novos contribuidores, equipe  
**Quando**: Antes de fazer PR  
**Tópicos**:
- Padrões de branch
- Padrões de commits
- Checklist de PR
- Como adicionar novo módulo
- Como reportar bugs

---

## 📊 Estrutura Rápida

```
fractal-engine/
├── src/                    # Implementação (com módulos organizados)
├── include/fractal_engine/ # Headers públicos (espelhando src/)
├── assets/                 # Recursos (shaders, texturas, etc)
├── tests/                  # Testes unitários
├── docs/                   # Esta documentação
├── build/                  # Output de compilação
└── lib/                    # Bibliotecas externas compiladas
```

---

## 🎬 Getting Started Rápido

### 1️⃣ Novo no Projeto?
Leia na ordem:
1. [Estrutura do Projeto](../ESTRUCTURA.md) - Entenda o layout
2. [Arquitetura](ARCHITECTURE.md) - Entenda as dependências
3. [Style Guide](STYLE_GUIDE.md) - Aprenda as convenções

### 2️⃣ Vai Implementar uma Feature?
1. Consulte [Arquitetura](ARCHITECTURE.md) - Qual módulo?
2. Siga [Style Guide](STYLE_GUIDE.md) - Formatação e nomes
3. Leia [Guia de Contribuição](CONTRIBUTING.md) - Como fazer PR

### 3️⃣ Reorganizando o Projeto?
1. Leia [Plano de Migração](MIGRATION_PLAN.md) - Roadmap detalhado
2. Siga o checklist fase-a-fase
3. Teste compilação após cada mudança

### 4️⃣ Code Review?
1. Verifique [Style Guide](STYLE_GUIDE.md) - Formatação OK?
2. Consulte [Arquitetura](ARCHITECTURE.md) - Dependências OK?
3. Leia [Contribuição](CONTRIBUTING.md) - Commits bem feitos?

---

## 🎯 Matriz de Responsabilidades

| Documento | Leitura | Implementação | Review | Arquitetura |
|-----------|---------|---------------|--------|-------------|
| **Estrutura** | ⭐⭐⭐ | ⭐⭐ | ⭐ | ⭐⭐⭐ |
| **Arquitetura** | ⭐⭐ | ⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐ |
| **Style Guide** | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ | ⭐ |
| **Migração** | ⭐⭐ | ⭐⭐⭐ | ⭐⭐ | ⭐⭐ |
| **Contribuição** | ⭐⭐⭐ | ⭐⭐ | ⭐⭐ | ⭐ |

---

## 🔗 Referências Rápidas

### Convenções Rápidas
```cpp
class MyClass { };          // PascalCase
void myFunction() { }       // camelCase
int m_memberVar = 10;       // m_camelCase
const int MAX_VALUE = 100;  // UPPER_SNAKE_CASE

namespace fractal_engine::module { }  // snake_case
```

### Estrutura de Pasta
```
src/            → Implementação C++
include/        → Headers públicos
assets/         → Recursos (shaders, texturas, etc)
docs/           → Documentação
tests/          → Testes unitários
build/          → Output (gerado, não commitar)
lib/            → Libs externas
```

### Padrão de Módulo
```
src/meu_modulo/
  ├── CMakeLists.txt        (lista sources)
  ├── Classe1.cpp
  └── Classe2.cpp

include/fractal_engine/meu_modulo/
  ├── Classe1.h
  └── Classe2.h
```

---

## 📞 Suporte

### Dúvidas Comuns

**P: Por que essa estrutura?**  
R: Escalabilidade, manutenibilidade e padrões da indústria (veja [Arquitetura](ARCHITECTURE.md))

**P: Posso adicionar um novo módulo?**  
R: Sim! Siga [Guia de Contribuição](CONTRIBUTING.md) - seção "Adicionando um Novo Módulo"

**P: Como mergear minha feature?**  
R: Leia [Guia de Contribuição](CONTRIBUTING.md) - seção "Pull Request"

**P: Meu código não compila, e agora?**  
R: Verifique [Style Guide](STYLE_GUIDE.md) e confirme imports/namespaces

---

## 📅 Versão desta Documentação

- **Versão**: 1.0
- **Atualizado em**: Março 2026
- **Status**: Completo

---

## 🚀 Próximos Passos

- [ ] Implementar migração (veja [Plano de Migração](MIGRATION_PLAN.md))
- [ ] Configurar CI/CD (GitHub Actions)
- [ ] Adicionar testes automatizados
- [ ] Documentação com Doxygen
- [ ] Performance profiling

---

**Última nota**: Este é um projeto em evolução. A documentação será atualizada conforme o projeto cresce. Sugestões são bem-vindas! 🎯
