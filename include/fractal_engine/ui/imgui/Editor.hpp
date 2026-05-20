/**
 * @file Editor.hpp
 * @brief Main editor class — owns the scene, all UI panels, and the undo stack.
 *
 * @ref Editor::Render() is called once per ImGui frame by @ref Application::RenderImGui().
 * Implementation is split across several translation units to keep each file focused:
 *
 * | File                  | Responsibility                                      |
 * |-----------------------|-----------------------------------------------------|
 * | Editor.cpp            | Render loop, menu bar, console, dock layout, gizmos |
 * | EditorHierarchy.cpp   | Entity creation, deletion, duplication, hierarchy UI |
 * | EditorInspector.cpp   | Per-component inspector panels                       |
 * | EditorProject.cpp     | Scene save / load / new + file dialogs               |
 */
#pragma once
#include "EditorTheme.hpp"
#include "EditorLayout.hpp"
#include "SceneEntity.hpp"
#include "AssetBrowser.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <string>
#include <vector>
#include <memory>

class EditorViewport;
class GameViewport;

/// @brief Controls which project dialog (Save / Open / None) is currently open.
enum class ProjectDialogMode { None, Save, Open };

/// @brief Editor — orchestrates all editor panels and owns the scene entity list.
class Editor
{
public:
    Editor();
    ~Editor() = default;

    /**
     * @brief Renders the entire editor UI for one frame.
     * @param scene      Scene viewport (may be @c nullptr before init).
     * @param game       Game viewport (may be @c nullptr before init).
     * @param deltaTime  Seconds since the last frame.
     */
    void Render(EditorViewport* scene, GameViewport* game, float deltaTime);

    /// @return @c true while the scene is in Play mode.
    bool IsPlaying() const { return m_Playing; }

    /// @return @c true while the scene is paused in Play mode.
    bool IsPaused()  const { return m_Paused;  }

    /// @return Mutable reference to the scene entity list (used by @ref Application for rendering).
    std::vector<std::unique_ptr<SceneEntity>>& GetEntities() { return m_Entities; }

    /// @return The entity whose @ref CameraComponent::IsPrimary is @c true, or @c nullptr.
    SceneEntity* GetPrimaryCamera();

    /// @return The currently selected entity, or @c nullptr if nothing is selected.
    SceneEntity* GetSelectedEntity() { return m_Selected; }

private:
    // Layout
    void BuildDockLayout();

    // Painéis
    void DrawMenuBar();
    void DrawHierarchy();
    void DrawInspector();
    void DrawConsole();

    // Hierarquia helpers
    void DrawEntityNode(SceneEntity& entity);
    void AddEntity(const std::string& name);
    void AddCameraEntity();
    void AddPrimitiveEntity(const std::string& name, const std::string& primitive);
    void AddEntityFromAsset(const std::string& relativePath);
    void DeleteEntity(uint32_t id);
    void DuplicateEntity(uint32_t id);
    void DeselectAll();

    // Projeto
    void NewProject();
    bool SaveProject(const std::string& path);
    bool LoadProject(const std::string& path);
    void DrawProjectDialog();

    // Inspector helpers
    void DrawTransformComponent(SceneEntity& e);
    void DrawCameraComponent(SceneEntity& e);
    void DrawMeshRendererComponent(SceneEntity& e);
    void DrawDirectionalLightComponent(SceneEntity& e);
    void DrawScriptComponent(SceneEntity& e);
    void DrawAudioListenerComponent(SceneEntity& e);
    void DrawAudioSourceComponent(SceneEntity& e);
    void DrawVec3Control(const char* label, glm::vec3& values, float resetValue = 0.0f);

    // Util
    void Log(LogLevel level, const std::string& msg);
    void PushAccentColor();
    void PopAccentColor();

    // Estado
    DockIDs m_Docks;
    bool    m_LayoutBuilt = false;

    // Cena — inicia VAZIA
    std::vector<std::unique_ptr<SceneEntity>> m_Entities;
    SceneEntity*                              m_Selected = nullptr;
    uint32_t                                  m_DeletePending    = 0;
    uint32_t                                  m_DuplicatePending = 0;

    // Console
    std::vector<LogEntry> m_Logs;
    bool                  m_ScrollToBottom = false;
    char                  m_ConsoleInput[256] = {};

    // Projeto
    std::string         m_ProjectPath;         ///< Caminho do projeto atual (vazio = sem projeto)
    std::string         m_ProjectName = "Untitled";
    ProjectDialogMode   m_ProjectDialog   = ProjectDialogMode::None;
    char                m_ProjectFileBuf[512] = {};
    bool                m_NewProjectConfirm   = false;

    // Playback
    bool m_Playing = false;
    bool m_Paused  = false;

    // Asset browser
    AssetBrowser       m_AssetBrowser;

    // Undo — histórico de transforms antes de cada operação do gizmo
    struct UndoRecord { uint32_t entityID; TransformComponent transform; };
    std::vector<UndoRecord> m_UndoStack;
    bool m_GizmoWasUsing = false;

    // Referências a engines externos
    class LuaScriptEngine* m_ScriptEngine = nullptr;
    class AudioEngine*     m_AudioEngine  = nullptr;

public:
    void SetScriptEngine(LuaScriptEngine* engine) { m_ScriptEngine = engine; }
    void SetAudioEngine(AudioEngine*      engine) { m_AudioEngine  = engine; }
};