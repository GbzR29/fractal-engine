#include "Application.hpp"
#include "Window.hpp"
#include "InputManager.hpp"
#include "EditorViewport.hpp"
#include "GameViewport.hpp"
#include "Editor.hpp"
#include "EditorTheme.hpp"
#include "AssetLoader.hpp"
#include "SceneRenderer.hpp"
#include "SkyRenderer.hpp"
#include "LuaScriptEngine.hpp"
#include "AudioEngine.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#ifdef FE_HAS_IMGUIZMO
#include <ImGuizmo.h>
#endif

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

    if (const char* envRoot = std::getenv("FRACTAL_ENGINE_ASSETS"))
        AssetLoader::setAssetsRoot(envRoot);

    // ── Cria estrutura de pastas padrão de assets ─────────────────────────────
    {
        const auto& root = AssetLoader::assetsRoot();
        for (const char* sub : {"models", "textures", "scenes", "audio",
                                 "shaders", "animations", "fonts"}) {
            std::filesystem::create_directories(root / sub);
        }
    }

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

    // ── SceneRenderer + SkyRenderer (shaders PBR) ────────────────────────────
    const std::string shaderDir = (AssetLoader::assetsRoot() / "shaders").string();

    m_SceneRenderer = std::make_unique<SceneRenderer>();
    if (!m_SceneRenderer->Init(shaderDir))
        std::cerr << "[Application] SceneRenderer sem shaders — render desativado.\n";

    m_SkyRenderer = std::make_unique<SkyRenderer>();
    if (!m_SkyRenderer->Init(shaderDir))
        std::cerr << "[Application] SkyRenderer sem shaders — ceu desativado.\n";

    m_ScriptEngine = std::make_unique<LuaScriptEngine>();
    if (!m_ScriptEngine->Init())
        std::cerr << "[Application] ScriptEngine: " << m_ScriptEngine->GetLastError() << "\n";
    m_Editor->SetScriptEngine(m_ScriptEngine.get());

    m_AudioEngine = std::make_unique<AudioEngine>();
    if (!m_AudioEngine->Init())
        std::cerr << "[Application] AudioEngine: " << m_AudioEngine->GetLastError() << "\n";
    m_Editor->SetAudioEngine(m_AudioEngine.get());

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

        flushPendingAssetLoads();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

#ifdef FE_HAS_IMGUIZMO
        ImGuizmo::BeginFrame();
        ImGuizmo::SetOrthographic(false);
#endif

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
    // Scripting — roda scripts durante o Play
    static bool s_WasPlaying = false;
    bool isPlaying = m_Editor->IsPlaying();

    if (isPlaying && !s_WasPlaying)
        m_ScriptEngine->OnStart(m_Editor->GetEntities());
    else if (!isPlaying && s_WasPlaying)
        m_ScriptEngine->OnStop(m_Editor->GetEntities());
    else if (isPlaying && !m_Editor->IsPaused())
        m_ScriptEngine->OnUpdate(m_Editor->GetEntities(), deltaTime);

    s_WasPlaying = isPlaying;

    m_AudioEngine->Update(m_Editor->GetEntities());
}

void Application::RenderScene(float deltaTime)
{
    (void)deltaTime;

    // ── Atualiza luz direcional a partir da cena ──────────────────────────────
    for (const auto& e : m_Editor->GetEntities()) {
        if (e->HasDirectionalLight()) {
            const auto& dl  = *e->DirectionalLight;
            const auto& rot = e->Transform.Rotation;

            // Converte Rotation (Euler em graus) em direção world-space
            // -Y no espaço local = "para baixo" = sol no zênite com rot=(0,0,0)
            glm::mat4 rotMat = glm::mat4(1.0f);
            rotMat = glm::rotate(rotMat, glm::radians(rot.x), {1,0,0});
            rotMat = glm::rotate(rotMat, glm::radians(rot.y), {0,1,0});
            rotMat = glm::rotate(rotMat, glm::radians(rot.z), {0,0,1});

            // uLightDir no shader = direção DA SUPERFÍCIE para a luz (negativa do sentido da luz)
            glm::vec3 lightDown = glm::normalize(glm::vec3(rotMat * glm::vec4(0,-1,0,0)));
            m_SceneRenderer->SetSunDirection(-lightDown);   // aponta para cima = em direção ao sol
            m_SceneRenderer->SetSunColor(dl.Color * dl.Intensity);
            break;
        }
    }

    // ── Viewport de edição: céu → entidades → grid ────────────────────────────
    m_SceneViewport->BindFramebuffer();
    {
        glEnable(GL_DEPTH_TEST);

        auto& cam = m_SceneViewport->GetCamera();
        m_SkyRenderer->Draw(cam.GetViewMatrix(), cam.GetProjectionMatrix());

        m_SceneRenderer->RenderEntities(
            m_Editor->GetEntities(),
            cam.GetViewMatrix(),
            cam.GetProjectionMatrix(),
            cam.GetPosition()
        );

        // Outline ao redor da entidade selecionada
        SceneEntity* sel = m_Editor->GetSelectedEntity();
        if (sel)
            m_SceneRenderer->RenderOutline(*sel, cam.GetViewMatrix(), cam.GetProjectionMatrix());

        // Grid por último (semi-transparente)
        m_SceneViewport->DrawGrid();
    }
    m_SceneViewport->UnbindFramebuffer();
}

void Application::RenderGame(float deltaTime)
{
    (void)deltaTime;

    m_GameViewport->BindFramebuffer();
    {
        glEnable(GL_DEPTH_TEST);

        // ── Câmera primária da cena ───────────────────────────────────────────
        SceneEntity* camEntity = m_Editor->GetPrimaryCamera();
        if (!camEntity || !camEntity->Camera) {
            // Sem câmera: sky vazio para indicar que falta câmera na cena
            glm::mat4 identView = glm::mat4(1.0f);
            glm::mat4 identProj = glm::perspective(
                glm::radians(60.0f),
                m_GameViewport->GetSize().x / std::max(m_GameViewport->GetSize().y, 1.0f),
                0.1f, 1000.0f);
            m_SkyRenderer->Draw(identView, identProj);
        } else {
            // Monta view matrix a partir do transform da entidade câmera
            const auto& t   = camEntity->Transform;
            const auto& cam = *camEntity->Camera;

            glm::mat4 rotMat = glm::mat4(1.0f);
            rotMat = glm::rotate(rotMat, glm::radians(t.Rotation.x), {1,0,0});
            rotMat = glm::rotate(rotMat, glm::radians(t.Rotation.y), {0,1,0});
            rotMat = glm::rotate(rotMat, glm::radians(t.Rotation.z), {0,0,1});

            glm::vec3 forward = glm::normalize(glm::vec3(rotMat * glm::vec4(0,0,-1,0)));
            glm::vec3 up      = glm::normalize(glm::vec3(rotMat * glm::vec4(0,1, 0,0)));

            glm::mat4 view = glm::lookAt(t.Position, t.Position + forward, up);
            float aspect = m_GameViewport->GetSize().x / std::max(m_GameViewport->GetSize().y, 1.0f);
            glm::mat4 proj = cam.GetProjection(aspect);

            m_SkyRenderer->Draw(view, proj);

            m_SceneRenderer->RenderEntities(
                m_Editor->GetEntities(), view, proj, t.Position);
        }
    }
    m_GameViewport->UnbindFramebuffer();
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
    m_AudioEngine.reset();
    m_ScriptEngine.reset();
    m_SkyRenderer.reset();
    m_SceneRenderer.reset();
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