#include "Application.hpp"
#include "Window.hpp"
#include "InputManager.hpp"
#include "EditorViewport.hpp"
#include "GameViewport.hpp"
#include "Editor.hpp"
#include "EditorTheme.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

Application* Application::s_Instance = nullptr;

Application::Application() { s_Instance = this; }
Application::~Application() {}

bool Application::Init()
{
    // ── Janela ────────────────────────────────────────────────────────────────
    WindowProps props;
    props.Title  = "FractalEngine";
    props.Width  = 1280;
    props.Height = 720;

    m_Window = std::make_unique<Window>();
    if (!m_Window->Init(props))
    {
        std::cerr << "[Application] Falha ao criar a janela.\n";
        return false;
    }

    // ── Input ─────────────────────────────────────────────────────────────────
    m_InputManager = std::make_unique<InputManager>();
    m_InputManager->Init(m_Window->GetNativeWindow());

    // ── ImGui ─────────────────────────────────────────────────────────────────
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    EditorTheme::Apply();

    ImGui_ImplGlfw_InitForOpenGL(m_Window->GetNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");
    std::cout << "[ImGui] " << IMGUI_VERSION << " inicializado.\n";

    // ── Viewports ─────────────────────────────────────────────────────────────
    m_SceneViewport = std::make_unique<EditorViewport>();
    if (!m_SceneViewport->Init(1280, 720))
    {
        std::cerr << "[Application] Falha ao inicializar SceneViewport.\n";
        return false;
    }

    m_GameViewport = std::make_unique<GameViewport>();
    m_GameViewport->Init(1280, 720);

    // ── Editor ────────────────────────────────────────────────────────────────
    m_Editor = std::make_unique<Editor>();

    m_Running = true;
    return true;
}

void Application::Run()
{
    while (m_Running && !m_Window->ShouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime    = currentFrame - m_LastFrame;
        m_LastFrame        = currentFrame;

        m_InputManager->Poll();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Update(deltaTime);

        // Renderiza as duas cenas nos seus FBOs
        RenderScene(deltaTime);
        RenderGame(deltaTime);

        // Volta para o framebuffer padrão (fundo do editor)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.08f, 0.08f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // UI
        RenderImGui(deltaTime);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_Window->SwapBuffers();
    }
}

void Application::Update(float deltaTime)
{
    (void)deltaTime;
    // Adicione lógica de sistemas aqui
}

void Application::RenderScene(float deltaTime)
{
    // ── Viewport de edição: renderiza cena + grid do editor ───────────────────
    m_SceneViewport->BindFramebuffer();
    {
        glEnable(GL_DEPTH_TEST);

        // ── Coloque seu renderer de cena aqui ─────────────────────────────────
        // Exemplo:
        //   auto& cam = m_SceneViewport->GetCamera();
        //   m_Renderer->Begin(cam.GetViewProjection());
        //   for (auto& entity : m_Scene->GetEntities())
        //       m_Renderer->Submit(entity.GetMesh(), entity.GetTransform());
        //   m_Renderer->End();

        // Grid sempre por último (é transparente, usa blending)
        m_SceneViewport->DrawGrid();
    }
    m_SceneViewport->UnbindFramebuffer();
    (void)deltaTime;
}

void Application::RenderGame(float deltaTime)
{
    // ── Viewport de gameplay: só renderiza quando Playing ─────────────────────
    if (!m_Editor->IsPlaying()) return;

    m_GameViewport->BindFramebuffer();
    {
        glEnable(GL_DEPTH_TEST);

        // ── Coloque o renderer com a câmera do jogo aqui ──────────────────────
        // Exemplo:
        //   m_Renderer->Begin(m_GameCamera->GetViewProjection());
        //   for (auto& entity : m_Scene->GetEntities())
        //       m_Renderer->Submit(entity.GetMesh(), entity.GetTransform());
        //   m_Renderer->End();
    }
    m_GameViewport->UnbindFramebuffer();
    (void)deltaTime;
}

void Application::RenderImGui(float deltaTime)
{
    m_Editor->Render(
        m_SceneViewport.get(),
        m_GameViewport.get(),
        deltaTime
    );
}

void Application::Shutdown()
{
    m_Editor.reset();
    m_GameViewport.reset();
    m_SceneViewport.reset();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_InputManager.reset();
    m_Window.reset();
    glfwTerminate();
}