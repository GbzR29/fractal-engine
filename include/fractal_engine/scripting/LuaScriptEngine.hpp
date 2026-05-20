/**
 * @file LuaScriptEngine.hpp
 * @brief Lua scripting engine — one isolated Lua environment per @ref ScriptComponent entity.
 *
 * Each script receives a @c self table bound to the entity's transform and state.
 *
 * ## Script API (available in every entity script)
 *
 * | Symbol               | Type        | Description                                  |
 * |----------------------|-------------|----------------------------------------------|
 * | `self.position.x/y/z`| read/write  | World-space position components.             |
 * | `self.rotation.x/y/z`| read/write  | Euler rotation in degrees.                   |
 * | `self.scale.x/y/z`   | read/write  | Scale components.                            |
 * | `self.name`          | read-only   | Entity display name.                         |
 * | `self.active`        | read/write  | Whether the entity is active.                |
 * | `print(msg)`         | function    | Appends a message to the editor console.     |
 * | `dt`                 | parameter   | Delta time argument passed to `OnUpdate(dt)` |
 *
 * ## Expected functions (all optional)
 * @code
 * function OnStart()  end          -- called once when Play mode begins
 * function OnUpdate(dt)  end       -- called every frame in Play mode
 * function OnDestroy()  end        -- called when Play mode ends
 * @endcode
 */
#pragma once
#include <string>
#include <vector>
#include <memory>

struct SceneEntity;

/// @brief Lua-based scripting engine integrated with the editor's Play mode lifecycle.
class LuaScriptEngine
{
public:
    LuaScriptEngine();
    ~LuaScriptEngine();

    /**
     * @brief Creates the shared Lua state and registers built-in functions.
     * @return @c true on success.
     */
    bool Init();

    /// Closes the Lua state and releases all script environments.
    void Shutdown();

    /**
     * @brief Loads and runs @c OnStart() for every entity with a @ref ScriptComponent.
     * @param entities  Active scene entity list.
     */
    void OnStart(std::vector<std::unique_ptr<SceneEntity>>& entities);

    /**
     * @brief Syncs entity state to Lua, calls @c OnUpdate(dt), then syncs back.
     * @param entities  Active scene entity list.
     * @param dt        Delta time in seconds.
     */
    void OnUpdate(std::vector<std::unique_ptr<SceneEntity>>& entities, float dt);

    /**
     * @brief Calls @c OnDestroy() for every scripted entity and clears runtime state.
     * @param entities  Active scene entity list.
     */
    void OnStop(std::vector<std::unique_ptr<SceneEntity>>& entities);

    /**
     * @brief Reloads the script for a single entity (e.g. after editing the script file).
     * @param e  Entity whose @ref ScriptComponent::scriptPath should be compiled.
     * @return @c true if the script compiled without errors.
     */
    bool ReloadScript(SceneEntity& e);

    /// @return The last script error message, or an empty string if none.
    const std::string& GetLastError() const { return m_LastError; }

    /// @return @c true while Play mode is active (between @ref OnStart and @ref OnStop).
    bool IsRunning() const { return m_Running; }

private:
    void SyncToLua   (SceneEntity& e);
    void SyncFromLua (SceneEntity& e);
    bool CallEntityFunc(SceneEntity& e, const char* funcName, float arg = 0.0f);
    void PushEntityEnv(SceneEntity& e);

    void* m_L = nullptr; ///< @c lua_State* stored as @c void* to keep lua.h out of downstream headers.
    bool  m_Running  = false;
    std::string m_LastError;
};
