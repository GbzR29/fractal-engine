/**
 * @file Application.hpp
 * @brief Top-level application class — owns all engine subsystems and drives the main loop.
 *
 * Usage:
 * @code
 *   Application app;
 *   if (app.Init()) app.Run();
 *   app.Shutdown();
 * @endcode
 */
#pragma once
#include <memory>

class Window;
class InputManager;
class EditorViewport;
class GameViewport;
class Editor;
class SceneRenderer;
class SkyRenderer;
class LuaScriptEngine;
class AudioEngine;

/// @brief Engine entry point — singleton that owns every subsystem.
class Application
{
public:
    Application();
    ~Application();

    /**
     * @brief Creates the window, initialises OpenGL, ImGui, and all subsystems.
     * @return @c true on success; @c false if any critical subsystem fails.
     */
    bool Init();

    /// Enters the main loop; blocks until the window is closed or @c m_Running becomes @c false.
    void Run();

    /// Shuts down all subsystems and releases resources in reverse-init order.
    void Shutdown();

    /// @return The global Application singleton.
    static Application& Get() { return *s_Instance; }

    /// @return Reference to the GLFW window wrapper.
    Window&       GetWindow()       { return *m_Window;       }

    /// @return Reference to the keyboard/mouse input manager.
    InputManager& GetInputManager() { return *m_InputManager; }

private:
    void Update(float deltaTime);
    void RenderScene(float deltaTime);
    void RenderGame(float deltaTime);
    void RenderImGui(float deltaTime);

private:
    static Application* s_Instance;

    std::unique_ptr<Window>         m_Window;
    std::unique_ptr<InputManager>   m_InputManager;
    std::unique_ptr<EditorViewport> m_SceneViewport;
    std::unique_ptr<GameViewport>   m_GameViewport;
    std::unique_ptr<Editor>         m_Editor;
    std::unique_ptr<SceneRenderer>    m_SceneRenderer;
    std::unique_ptr<SkyRenderer>      m_SkyRenderer;
    std::unique_ptr<LuaScriptEngine>  m_ScriptEngine;
    std::unique_ptr<AudioEngine>      m_AudioEngine;

    bool  m_Running   = false;
    float m_LastFrame = 0.0f;
};