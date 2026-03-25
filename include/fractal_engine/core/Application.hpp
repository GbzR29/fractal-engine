#pragma once
#include <memory>

class Window;
class InputManager;
class EditorViewport;
class GameViewport;
class Editor;

class Application
{
public:
    Application();
    ~Application();

    bool Init();
    void Run();
    void Shutdown();

    static Application& Get() { return *s_Instance; }
    Window&       GetWindow()       { return *m_Window;       }
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

    bool  m_Running   = false;
    float m_LastFrame = 0.0f;
};