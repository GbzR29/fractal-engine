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

class Editor
{
public:
    Editor();
    ~Editor() = default;

    void Render(EditorViewport* scene, GameViewport* game, float deltaTime);

    bool IsPlaying() const { return m_Playing; }
    bool IsPaused()  const { return m_Paused;  }

    // Acesso à cena para o Application renderizar
    std::vector<std::unique_ptr<SceneEntity>>& GetEntities() { return m_Entities; }
    SceneEntity* GetPrimaryCamera();

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
    void DeleteEntity(uint32_t id);
    void DeselectAll();

    // Inspector helpers
    void DrawTransformComponent(SceneEntity& e);
    void DrawCameraComponent(SceneEntity& e);
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
    uint32_t                                  m_DeletePending = 0;

    // Console
    std::vector<LogEntry> m_Logs;
    bool                  m_ScrollToBottom = false;
    char                  m_ConsoleInput[256] = {};

    // Playback
    bool m_Playing = false;
    bool m_Paused  = false;

    // Asset browser
    AssetBrowser       m_AssetBrowser;
};