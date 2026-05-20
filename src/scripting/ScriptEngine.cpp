#include "LuaScriptEngine.hpp"
#include "SceneEntity.hpp"
#include "AssetLoader.hpp"

#ifdef FE_ENABLE_LUA
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers internos
// ─────────────────────────────────────────────────────────────────────────────
#ifdef FE_ENABLE_LUA
static lua_State* L(void* p) { return static_cast<lua_State*>(p); }

// print() que redireciona para std::cout (pode ser ligado ao console do editor depois)
static int lua_print(lua_State* state)
{
    int n = lua_gettop(state);
    for (int i = 1; i <= n; i++) {
        if (i > 1) std::cout << "\t";
        if (lua_isstring(state, i))
            std::cout << lua_tostring(state, i);
        else
            std::cout << "[" << lua_typename(state, lua_type(state, i)) << "]";
    }
    std::cout << "\n";
    return 0;
}

// Chave do registry para o ambiente de uma entidade (usando o ID como chave)
static void entity_env_key(lua_State* state, uint32_t id)
{
    lua_pushliteral(state, "FE_entity_env_");
    lua_pushinteger(state, (lua_Integer)id);
    lua_concat(state, 2);
}
#endif

// ─────────────────────────────────────────────────────────────────────────────
LuaScriptEngine::LuaScriptEngine()  = default;
LuaScriptEngine::~LuaScriptEngine() { Shutdown(); }

bool LuaScriptEngine::Init()
{
#ifdef FE_ENABLE_LUA
    if (m_L) return true;

    lua_State* state = luaL_newstate();
    if (!state) { m_LastError = "luaL_newstate falhou"; return false; }
    luaL_openlibs(state);

    // Substitui o print padrão
    lua_pushcfunction(state, lua_print);
    lua_setglobal(state, "print");

    m_L = state;
    std::cout << "[Lua] Script engine iniciado (Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR ")\n";
    return true;
#else
    m_LastError = "Lua não compilado (FE_ENABLE_LUA=OFF)";
    return false;
#endif
}

void LuaScriptEngine::Shutdown()
{
#ifdef FE_ENABLE_LUA
    if (m_L) {
        lua_close(L(m_L));
        m_L = nullptr;
    }
    m_Running = false;
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  Carregar script de uma entidade
// ─────────────────────────────────────────────────────────────────────────────
bool LuaScriptEngine::ReloadScript(SceneEntity& e)
{
    if (!e.HasScript()) return false;
#ifdef FE_ENABLE_LUA
    if (!m_L) { m_LastError = "Engine não inicializado"; return false; }

    auto& sc   = *e.Script;
    sc.loaded  = false;
    sc.lastError.clear();

    // Resolve caminho absoluto
    std::filesystem::path absPath = AssetLoader::assetsRoot() / sc.scriptPath;
    if (!std::filesystem::exists(absPath)) {
        sc.lastError = "Arquivo não encontrado: " + absPath.string();
        m_LastError  = sc.lastError;
        return false;
    }

    lua_State* state = L(m_L);

    // Cria ambiente isolado para esta entidade
    // env herda de _G via __index para ter acesso às stdlib
    lua_newtable(state);                          // env = {}
    lua_newtable(state);                          // meta = {}
    lua_pushvalue(state, LUA_REGISTRYINDEX);      // empilha registry
    lua_getfield(state, -1, "_LOADED");           // não útil, pop
    lua_pop(state, 1);
    lua_getglobal(state, "_G");                   // _G
    lua_setfield(state, -2, "__index");           // meta.__index = _G
    lua_setmetatable(state, -2);                  // setmetatable(env, meta)

    // Guarda env no registry keyed por ID
    entity_env_key(state, e.ID);                  // empilha a chave string
    lua_pushvalue(state, -2);                     // copia env
    lua_settable(state, LUA_REGISTRYINDEX);       // registry[key] = env
    lua_pop(state, 1);                            // pop env

    // Carrega o arquivo
    int status = luaL_loadfile(state, absPath.string().c_str());
    if (status != LUA_OK) {
        sc.lastError = lua_tostring(state, -1);
        m_LastError  = sc.lastError;
        lua_pop(state, 1);
        return false;
    }

    // Configura o ambiente do chunk
    entity_env_key(state, e.ID);
    lua_gettable(state, LUA_REGISTRYINDEX);       // pega env do registry
    lua_setupvalue(state, -2, 1);                 // chunk._ENV = env

    // Executa o chunk (define OnStart/OnUpdate/etc. dentro de env)
    status = lua_pcall(state, 0, 0, 0);
    if (status != LUA_OK) {
        sc.lastError = lua_tostring(state, -1);
        m_LastError  = sc.lastError;
        lua_pop(state, 1);
        return false;
    }

    sc.loaded = true;
    std::cout << "[Lua] Script carregado: " << sc.scriptPath << " (entity " << e.ID << ")\n";
    return true;
#else
    e.Script->lastError = "Lua não disponível";
    return false;
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  Sincronização C++ ↔ Lua
// ─────────────────────────────────────────────────────────────────────────────
#ifdef FE_ENABLE_LUA
void LuaScriptEngine::SyncToLua(SceneEntity& e)
{
    lua_State* state = L(m_L);

    // Pega env do registry
    entity_env_key(state, e.ID);
    lua_gettable(state, LUA_REGISTRYINDEX);
    if (!lua_istable(state, -1)) { lua_pop(state, 1); return; }
    int envIdx = lua_gettop(state);

    // Monta tabela 'self'
    lua_newtable(state);                           // self = {}
    int selfIdx = lua_gettop(state);

    // self.name
    lua_pushstring(state, e.Tag.Name.c_str());
    lua_setfield(state, selfIdx, "name");

    // self.active
    lua_pushboolean(state, e.Active ? 1 : 0);
    lua_setfield(state, selfIdx, "active");

    // Helper: push vec3 sub-table
    auto pushVec3 = [&](glm::vec3& v) {
        lua_newtable(state);
        lua_pushnumber(state, v.x); lua_setfield(state, -2, "x");
        lua_pushnumber(state, v.y); lua_setfield(state, -2, "y");
        lua_pushnumber(state, v.z); lua_setfield(state, -2, "z");
    };

    pushVec3(e.Transform.Position); lua_setfield(state, selfIdx, "position");
    pushVec3(e.Transform.Rotation); lua_setfield(state, selfIdx, "rotation");
    pushVec3(e.Transform.Scale);    lua_setfield(state, selfIdx, "scale");

    lua_setfield(state, envIdx, "self");  // env.self = self
    lua_pop(state, 1);                   // pop env
}

void LuaScriptEngine::SyncFromLua(SceneEntity& e)
{
    lua_State* state = L(m_L);

    entity_env_key(state, e.ID);
    lua_gettable(state, LUA_REGISTRYINDEX);
    if (!lua_istable(state, -1)) { lua_pop(state, 1); return; }

    lua_getfield(state, -1, "self");    // env.self
    if (!lua_istable(state, -1)) { lua_pop(state, 2); return; }
    int selfIdx = lua_gettop(state);

    // active
    lua_getfield(state, selfIdx, "active");
    if (lua_isboolean(state, -1)) e.Active = (lua_toboolean(state, -1) != 0);
    lua_pop(state, 1);

    // Helper: read vec3 sub-table
    auto readVec3 = [&](const char* field, glm::vec3& out) {
        lua_getfield(state, selfIdx, field);
        if (lua_istable(state, -1)) {
            lua_getfield(state, -1, "x"); out.x = (float)lua_tonumber(state, -1); lua_pop(state, 1);
            lua_getfield(state, -1, "y"); out.y = (float)lua_tonumber(state, -1); lua_pop(state, 1);
            lua_getfield(state, -1, "z"); out.z = (float)lua_tonumber(state, -1); lua_pop(state, 1);
        }
        lua_pop(state, 1);
    };

    readVec3("position", e.Transform.Position);
    readVec3("rotation", e.Transform.Rotation);
    readVec3("scale",    e.Transform.Scale);

    lua_pop(state, 2); // pop self + env
}

bool LuaScriptEngine::CallEntityFunc(SceneEntity& e, const char* funcName, float arg)
{
    lua_State* state = L(m_L);

    entity_env_key(state, e.ID);
    lua_gettable(state, LUA_REGISTRYINDEX);
    if (!lua_istable(state, -1)) { lua_pop(state, 1); return true; }

    lua_getfield(state, -1, funcName);
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2); // pop func + env
        return true;       // função não existe, tudo bem
    }

    int nArgs = 0;
    if (arg != 0.0f || std::string(funcName) == "OnUpdate") {
        lua_pushnumber(state, (lua_Number)arg);
        nArgs = 1;
    }

    int status = lua_pcall(state, nArgs, 0, 0);
    lua_pop(state, 1); // pop env

    if (status != LUA_OK) {
        std::string err = lua_tostring(state, -1);
        lua_pop(state, 1);
        if (e.HasScript()) e.Script->lastError = err;
        m_LastError = err;
        std::cerr << "[Lua] Erro em " << funcName << " (entity " << e.ID << "): " << err << "\n";
        return false;
    }
    return true;
}
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  Lifecycle
// ─────────────────────────────────────────────────────────────────────────────
void LuaScriptEngine::OnStart(std::vector<std::unique_ptr<SceneEntity>>& entities)
{
#ifdef FE_ENABLE_LUA
    if (!m_L) return;
    m_Running = true;
    for (auto& e : entities) {
        if (!e->HasScript()) continue;
        if (!e->Script->loaded)
            ReloadScript(*e);
        if (!e->Script->loaded) continue;
        SyncToLua(*e);
        CallEntityFunc(*e, "OnStart");
        SyncFromLua(*e);
    }
#endif
}

void LuaScriptEngine::OnUpdate(std::vector<std::unique_ptr<SceneEntity>>& entities, float dt)
{
#ifdef FE_ENABLE_LUA
    if (!m_L || !m_Running) return;
    for (auto& e : entities) {
        if (!e->HasScript() || !e->Script->loaded) continue;
        SyncToLua(*e);
        CallEntityFunc(*e, "OnUpdate", dt);
        SyncFromLua(*e);
    }
#endif
}

void LuaScriptEngine::OnStop(std::vector<std::unique_ptr<SceneEntity>>& entities)
{
#ifdef FE_ENABLE_LUA
    if (!m_L) return;
    m_Running = false;
    for (auto& e : entities) {
        if (!e->HasScript() || !e->Script->loaded) continue;
        SyncToLua(*e);
        CallEntityFunc(*e, "OnDestroy");
    }
#endif
}

void LuaScriptEngine::PushEntityEnv(SceneEntity& e)
{
    (void)e;
}
