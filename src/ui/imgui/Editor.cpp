/**
 * @file Editor.cpp
 * @brief Core editor orchestration: main render loop, menu bar, console, dock layout.
 *
 * Entity management lives in EditorHierarchy.cpp, component inspection in
 * EditorInspector.cpp, and project save/load in EditorProject.cpp.
 */

#include "Editor.hpp"
#include "EditorViewport.hpp"
#include "GameViewport.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cstring>

#ifdef FE_HAS_IMGUIZMO
#include <ImGuizmo.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────

Editor::Editor()
{
    Log(LogLevel::Info,    "FractalEngine started.");
    Log(LogLevel::Success, "OpenGL 4.6 Core loaded.");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Utility helpers
// ─────────────────────────────────────────────────────────────────────────────

void Editor::Log(LogLevel level, const std::string& msg)
{
    m_Logs.push_back({ level, msg });
    m_ScrollToBottom = true;
}

void Editor::PushAccentColor()
{
    ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::Accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::AccentHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  EditorTheme::Color::AccentActive);
}

void Editor::PopAccentColor() { ImGui::PopStyleColor(3); }

void Editor::DeselectAll()
{
    m_Selected = nullptr;
    for (auto& e : m_Entities) e->Selected = false;
}

SceneEntity* Editor::GetPrimaryCamera()
{
    for (auto& e : m_Entities)
        if (e->HasCamera() && e->Camera->IsPrimary)
            return e.get();
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Main render frame
// ─────────────────────────────────────────────────────────────────────────────

void Editor::Render(EditorViewport* scene, GameViewport* game, float deltaTime)
{
    // Full-screen host window that owns the dockspace and menu bar
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGuiWindowFlags hostFlags =
        ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoCollapse  |
        ImGuiWindowFlags_NoResize    | ImGuiWindowFlags_NoMove      |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_MenuBar     | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    { 0.0f, 0.0f });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::Color::BgDeep);
    ImGui::Begin("##Host", nullptr, hostFlags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);

    DrawMenuBar();

    m_Docks.Root = ImGui::GetID("DockSpace");
    ImGui::DockSpace(m_Docks.Root, { 0, 0 }, ImGuiDockNodeFlags_None);
    if (!m_LayoutBuilt) { BuildDockLayout(); m_LayoutBuilt = true; }

    ImGui::End();

    // Panels
    DrawHierarchy();
    DrawInspector();
    m_AssetBrowser.Draw();
    DrawConsole();

    // Deferred entity operations (must happen outside the hierarchy iteration)
    if (m_DeletePending) {
        DeleteEntity(m_DeletePending);
        m_DeletePending = 0;
    }
    if (m_DuplicatePending) {
        DuplicateEntity(m_DuplicatePending);
        m_DuplicatePending = 0;
    }

    // Project dialogs (outside host window to maintain correct z-order)
    DrawProjectDialog();

    // ── Scene viewport ────────────────────────────────────────────────────────
    if (scene)
    {
        scene->Begin(deltaTime);

        if (m_Selected)
        {
            // Directional lights can only be rotated, not translated
            if (m_Selected->HasDirectionalLight() &&
                scene->GetGizmoMode() != GizmoMode::Rotate)
                scene->SetGizmoMode(GizmoMode::Rotate);

#ifdef FE_HAS_IMGUIZMO
            // Recompose to matrix using ImGuizmo convention (prevents Euler drift)
            float tArr[3] = { m_Selected->Transform.Position.x,
                               m_Selected->Transform.Position.y,
                               m_Selected->Transform.Position.z };
            float rArr[3] = { m_Selected->Transform.Rotation.x,
                               m_Selected->Transform.Rotation.y,
                               m_Selected->Transform.Rotation.z };
            float sArr[3] = { m_Selected->Transform.Scale.x,
                               m_Selected->Transform.Scale.y,
                               m_Selected->Transform.Scale.z };
            glm::mat4 transform;
            ImGuizmo::RecomposeMatrixFromComponents(tArr, rArr, sArr,
                glm::value_ptr(transform));

            // Snapshot for Ctrl+Z before the drag starts
            TransformComponent preSnap = m_Selected->Transform;
            scene->DrawGizmo(transform);

            bool isUsing = ImGuizmo::IsUsing();
            if (!m_GizmoWasUsing && isUsing)
                m_UndoStack.push_back({ m_Selected->ID, preSnap });
            m_GizmoWasUsing = isUsing;

            if (isUsing)
            {
                ImGuizmo::DecomposeMatrixToComponents(
                    glm::value_ptr(transform), tArr, rArr, sArr);
                m_Selected->Transform.Position = { tArr[0], tArr[1], tArr[2] };
                m_Selected->Transform.Rotation = { rArr[0], rArr[1], rArr[2] };
                m_Selected->Transform.Scale = {
                    std::max(0.001f, sArr[0]),
                    std::max(0.001f, sArr[1]),
                    std::max(0.001f, sArr[2]) };
            }
#else
            glm::mat4 transform = m_Selected->Transform.GetMatrix();
            scene->DrawGizmo(transform);
#endif
        }

        // Ctrl+Z: undo last gizmo transform
#ifdef FE_HAS_IMGUIZMO
        if (ImGui::IsKeyPressed(ImGuiKey_Z) && ImGui::GetIO().KeyCtrl
            && !ImGui::IsAnyItemActive() && !m_UndoStack.empty())
        {
            auto& rec = m_UndoStack.back();
            for (auto& e : m_Entities)
                if (e->ID == rec.entityID) { e->Transform = rec.transform; break; }
            m_UndoStack.pop_back();
        }
#endif

        // Del: mark selected entity for deletion
        if (m_Selected && ImGui::IsKeyPressed(ImGuiKey_Delete)
            && !ImGui::IsAnyItemActive())
            m_DeletePending = m_Selected->ID;

        // Drag-drop from AssetBrowser onto the viewport
        if (scene->HasPendingDrop())
            AddEntityFromAsset(scene->TakePendingDrop());

        scene->End();
    }

    // ── Game viewport ─────────────────────────────────────────────────────────
    if (game)
    {
        game->Begin(
            m_Playing, m_Paused,
            [this]{
                m_Playing = !m_Playing;
                if (!m_Playing) m_Paused = false;
                Log(LogLevel::Info, m_Playing ? "Play started." : "Play stopped.");
            },
            [this]{
                m_Paused = !m_Paused;
                Log(LogLevel::Info, m_Paused ? "Paused." : "Resumed.");
            },
            [this]{ Log(LogLevel::Info, "Step."); }
        );
        game->End();
    }

    (void)deltaTime;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Dock layout (first-run only, resetable via View > Reset Layout)
// ─────────────────────────────────────────────────────────────────────────────

void Editor::BuildDockLayout()
{
    ImGuiID root = m_Docks.Root;
    ImGui::DockBuilderRemoveNode(root);
    ImGui::DockBuilderAddNode(root, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(root, ImGui::GetMainViewport()->WorkSize);

    ImGui::DockBuilderSplitNode(root, ImGuiDir_Left, 0.18f,
        &m_Docks.Left, &m_Docks.Center);
    ImGui::DockBuilderSplitNode(m_Docks.Center, ImGuiDir_Right, 0.22f,
        &m_Docks.Right, &m_Docks.Center);
    ImGui::DockBuilderSplitNode(m_Docks.Center, ImGuiDir_Down, 0.26f,
        &m_Docks.Bottom, &m_Docks.Center);
    ImGui::DockBuilderSplitNode(m_Docks.Bottom, ImGuiDir_Left, 0.50f,
        &m_Docks.BottomLeft, &m_Docks.BottomRight);

    ImGui::DockBuilderDockWindow("Hierarchy",     m_Docks.Left);
    ImGui::DockBuilderDockWindow("Inspector",     m_Docks.Right);
    ImGui::DockBuilderDockWindow("Scene",         m_Docks.Center);
    ImGui::DockBuilderDockWindow("Game",          m_Docks.Center);
    ImGui::DockBuilderDockWindow("Asset Browser", m_Docks.BottomLeft);
    ImGui::DockBuilderDockWindow("Console",       m_Docks.BottomRight);

    ImGui::DockBuilderFinish(root);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Menu bar
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawMenuBar()
{
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, EditorTheme::Color::BgDeep);
    if (ImGui::BeginMenuBar())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Accent);
        ImGui::Text("FRACTAL");
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 4);
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("ENGINE");
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 16);

        if (ImGui::BeginMenu("File"))
        {
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::Text("Scene: %s", m_ProjectName.c_str());
            ImGui::PopStyleColor();
            ImGui::Separator();

            if (ImGui::MenuItem("New Scene",  "Ctrl+N"))
                m_NewProjectConfirm = true;
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                m_ProjectDialog = ProjectDialogMode::Open;
                if (!m_ProjectPath.empty())
                    std::strncpy(m_ProjectFileBuf, m_ProjectPath.c_str(),   sizeof(m_ProjectFileBuf) - 1);
                else
                    std::strncpy(m_ProjectFileBuf, "scenes/",               sizeof(m_ProjectFileBuf) - 1);
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                if (!m_ProjectPath.empty())
                    SaveProject(m_ProjectPath);
                else {
                    m_ProjectDialog = ProjectDialogMode::Save;
                    std::strncpy(m_ProjectFileBuf, "scenes/untitled.json", sizeof(m_ProjectFileBuf) - 1);
                }
            }
            if (ImGui::MenuItem("Save As...")) {
                m_ProjectDialog = ProjectDialogMode::Save;
                if (!m_ProjectPath.empty())
                    std::strncpy(m_ProjectFileBuf, m_ProjectPath.c_str(),        sizeof(m_ProjectFileBuf) - 1);
                else
                    std::strncpy(m_ProjectFileBuf, "scenes/untitled.json",       sizeof(m_ProjectFileBuf) - 1);
            }
            ImGui::Separator();
            ImGui::MenuItem("Build",  "Ctrl+B");
            ImGui::Separator();
            ImGui::MenuItem("Exit",   "Alt+F4");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::MenuItem("Undo", "Ctrl+Z");
            ImGui::MenuItem("Redo", "Ctrl+Y");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Entity"))
        {
            if (ImGui::MenuItem("Empty Entity"))      AddEntity("Entity");
            if (ImGui::MenuItem("Camera"))            AddCameraEntity();
            ImGui::Separator();
            if (ImGui::MenuItem("Cube"))              AddPrimitiveEntity("Cube",   "Cube");
            if (ImGui::MenuItem("Sphere"))            AddPrimitiveEntity("Sphere", "Sphere");
            if (ImGui::MenuItem("Plane"))             AddPrimitiveEntity("Plane",  "Plane");
            ImGui::Separator();
            if (ImGui::MenuItem("Directional Light")) AddEntity("Directional Light");
            if (ImGui::MenuItem("Point Light"))       AddEntity("Point Light");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Reset Layout"))
                m_LayoutBuilt = false;
            ImGui::EndMenu();
        }

        // ── Centered Play / Pause / Step controls ────────────────────────────
        {
            const float btnW  = 58.0f;
            const float btnH  = ImGui::GetFrameHeight();
            const float sp    = ImGui::GetStyle().ItemSpacing.x;
            const float barW  = btnW * 3.0f + sp * 2.0f;
            float targetX = (ImGui::GetWindowWidth() - barW) * 0.5f;
            float curX    = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(targetX > curX ? targetX : curX);

            ImGui::PushStyleColor(ImGuiCol_Button,
                m_Playing ? ImVec4{0.70f,0.15f,0.15f,1.0f} : EditorTheme::Color::Accent);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                m_Playing ? ImVec4{0.90f,0.25f,0.25f,1.0f} : EditorTheme::Color::AccentHover);
            if (ImGui::Button(m_Playing ? "Stop" : "Play", { btnW, btnH })) {
                m_Playing = !m_Playing;
                if (!m_Playing) m_Paused = false;
                Log(LogLevel::Info, m_Playing ? "Play started." : "Play stopped.");
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine();

            ImGui::BeginDisabled(!m_Playing);
            ImGui::PushStyleColor(ImGuiCol_Button,
                m_Paused ? EditorTheme::Color::Warning : EditorTheme::Color::BgPanel);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
            if (ImGui::Button(m_Paused ? "Resume" : "Pause", { btnW, btnH })) {
                m_Paused = !m_Paused;
                Log(LogLevel::Info, m_Paused ? "Paused." : "Resumed.");
            }
            ImGui::PopStyleColor(2);
            ImGui::EndDisabled();
            ImGui::SameLine();

            ImGui::BeginDisabled(!m_Playing || !m_Paused);
            ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
            if (ImGui::Button("Step", { btnW, btnH }))
                Log(LogLevel::Info, "Step.");
            ImGui::PopStyleColor(2);
            ImGui::EndDisabled();
        }

        ImGui::EndMenuBar();
    }
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Console panel
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawConsole()
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::Color::BgBase);
    if (ImGui::Begin("Console"))
    {
        ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
        if (ImGui::Button("Clear")) m_Logs.clear();
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Info);
        ImGui::SmallButton("Info");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Warning);
        ImGui::SmallButton("Warn");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Error);
        ImGui::SmallButton("Error");
        ImGui::PopStyleColor();
        ImGui::Separator();

        float inputH = ImGui::GetFrameHeightWithSpacing() + 8.0f;
        ImGui::BeginChild("##logs",
            { 0, ImGui::GetContentRegionAvail().y - inputH }, false);

        for (auto& entry : m_Logs)
        {
            ImVec4 col;
            const char* pfx;
            switch (entry.Level)
            {
                case LogLevel::Warning: col = EditorTheme::Color::Warning;    pfx = "[WARN] "; break;
                case LogLevel::Error:   col = EditorTheme::Color::Error;      pfx = "[ERR]  "; break;
                case LogLevel::Success: col = EditorTheme::Color::Success;    pfx = "[OK]   "; break;
                default:                col = EditorTheme::Color::TextNormal; pfx = "[INFO] "; break;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, col);
            ImGui::TextUnformatted(pfx);
            ImGui::PopStyleColor();
            ImGui::SameLine(0, 0);
            ImGui::TextUnformatted(entry.Message.c_str());
        }

        if (m_ScrollToBottom) {
            ImGui::SetScrollHereY(1.0f);
            m_ScrollToBottom = false;
        }
        ImGui::EndChild();

        ImGui::Separator();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 62.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::Color::BgInput);
        bool enter = ImGui::InputText("##cmd", m_ConsoleInput,
            sizeof(m_ConsoleInput), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        PushAccentColor();
        if (ImGui::Button("Run", { 58.0f, 0 }) || enter)
        {
            if (m_ConsoleInput[0] != '\0')
            {
                Log(LogLevel::Info, std::string("> ") + m_ConsoleInput);
                m_ConsoleInput[0] = '\0';
            }
            ImGui::SetKeyboardFocusHere(-1);
        }
        PopAccentColor();
    }
    ImGui::End();
    ImGui::PopStyleColor();
}
