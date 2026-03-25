#include "Editor.hpp"
#include "EditorViewport.hpp"
#include "AssetBrowser.hpp"
#include "GameViewport.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cstring>
#include <cstdio>

Editor::Editor()
{
    Log(LogLevel::Info,    "FractalEngine iniciado.");
    Log(LogLevel::Success, "OpenGL 4.6 Core carregado.");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
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
//  Adicionar entidades
// ─────────────────────────────────────────────────────────────────────────────
void Editor::AddEntity(const std::string& name)
{
    DeselectAll();
    auto e = std::make_unique<SceneEntity>(name);
    e->Selected = true;
    m_Selected  = e.get();
    Log(LogLevel::Info, "Entidade criada: " + name);
    m_Entities.push_back(std::move(e));
}

void Editor::AddCameraEntity()
{
    DeselectAll();
    auto e = std::make_unique<SceneEntity>("Camera");
    e->Camera = std::make_unique<CameraComponent>();

    // Se já existe uma câmera primária, essa não é primária
    if (GetPrimaryCamera())
        e->Camera->IsPrimary = false;

    // Posiciona um pouco acima e atrás da origem
    e->Transform.Position = { 0.0f, 5.0f, 10.0f };
    e->Transform.Rotation = { -20.0f, 0.0f, 0.0f };

    e->Selected = true;
    m_Selected  = e.get();
    Log(LogLevel::Info, "Camera adicionada.");
    m_Entities.push_back(std::move(e));
}

void Editor::DeleteEntity(uint32_t id)
{
    m_Entities.erase(
        std::remove_if(m_Entities.begin(), m_Entities.end(),
            [id](const auto& e){ return e->ID == id; }),
        m_Entities.end()
    );
    if (m_Selected && m_Selected->ID == id)
        m_Selected = nullptr;
    Log(LogLevel::Info, "Entidade removida.");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Render
// ─────────────────────────────────────────────────────────────────────────────
void Editor::Render(EditorViewport* scene, GameViewport* game, float deltaTime)
{
    // Host window
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGuiWindowFlags hostFlags =
        ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize    | ImGuiWindowFlags_NoMove     |
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

    // Painéis
    DrawHierarchy();
    DrawInspector();
    m_AssetBrowser.Draw();
    DrawConsole();

    // Deletar pendente (não pode deletar durante iteração)
    if (m_DeletePending)
    {
        DeleteEntity(m_DeletePending);
        m_DeletePending = 0;
    }

    // Viewports
    if (scene) { scene->Begin(deltaTime); scene->End(); }

    if (game)
    {
        game->Begin(
            m_Playing, m_Paused,
            [this]{ // Play/Stop
                m_Playing = !m_Playing;
                if (!m_Playing) m_Paused = false;
                Log(LogLevel::Info, m_Playing ? "Play iniciado." : "Play parado.");
            },
            [this]{ // Pause
                m_Paused = !m_Paused;
                Log(LogLevel::Info, m_Paused ? "Pausado." : "Resumido.");
            },
            [this]{ // Step
                Log(LogLevel::Info, "Step.");
            }
        );
        game->End();
    }

    (void)deltaTime;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Dock layout
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
//  Menu Bar
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
            if (ImGui::MenuItem("New Scene",  "Ctrl+N")) m_Entities.clear();
            ImGui::MenuItem("Open Scene", "Ctrl+O");
            ImGui::MenuItem("Save Scene", "Ctrl+S");
            ImGui::Separator();
            ImGui::MenuItem("Build",      "Ctrl+B");
            ImGui::Separator();
            ImGui::MenuItem("Exit",       "Alt+F4");
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
            if (ImGui::MenuItem("Empty Entity"))   AddEntity("Entity");
            if (ImGui::MenuItem("Camera"))         AddCameraEntity();
            ImGui::Separator();
            if (ImGui::MenuItem("Cube"))           AddEntity("Cube");
            if (ImGui::MenuItem("Sphere"))         AddEntity("Sphere");
            if (ImGui::MenuItem("Plane"))          AddEntity("Plane");
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
        ImGui::EndMenuBar();
    }
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Hierarchy
// ─────────────────────────────────────────────────────────────────────────────
void Editor::DrawHierarchy()
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::Color::BgBase);
    if (ImGui::Begin("Hierarchy"))
    {
        // Header
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("SCENE");
        ImGui::PopStyleColor();
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 22.0f);

        // Botão "+" com popup
        PushAccentColor();
        if (ImGui::Button("+", { 22, 22 }))
            ImGui::OpenPopup("AddEntityPopup");
        PopAccentColor();

        if (ImGui::BeginPopup("AddEntityPopup"))
        {
            ImGui::SeparatorText("Adicionar");
            if (ImGui::MenuItem("Empty Entity"))      AddEntity("Entity");
            if (ImGui::MenuItem("Camera"))            AddCameraEntity();
            ImGui::Separator();
            if (ImGui::MenuItem("Cube"))              AddEntity("Cube");
            if (ImGui::MenuItem("Sphere"))            AddEntity("Sphere");
            if (ImGui::MenuItem("Plane"))             AddEntity("Plane");
            ImGui::Separator();
            if (ImGui::MenuItem("Directional Light")) AddEntity("Directional Light");
            if (ImGui::MenuItem("Point Light"))       AddEntity("Point Light");
            ImGui::EndPopup();
        }

        ImGui::Separator();

        // Busca
        static char searchBuf[64] = {};
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::Color::BgInput);
        ImGui::InputText("##search", searchBuf, sizeof(searchBuf));
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // Lista vazia
        if (m_Entities.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::TextWrapped("Cena vazia. Use + ou o menu Entity para adicionar.");
            ImGui::PopStyleColor();
        }

        // Nós da cena
        for (auto& entity : m_Entities)
            DrawEntityNode(*entity);

        // Click no fundo da lista = deselecionar
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
            && !ImGui::IsAnyItemHovered())
            DeselectAll();
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

void Editor::DrawEntityNode(SceneEntity& entity)
{
    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow   |
        ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_FramePadding;

    if (entity.Children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (entity.Selected)
        flags |= ImGuiTreeNodeFlags_Selected;

    // Ícone por tipo
    const char* icon = "  ";
    if (entity.HasCamera()) icon = "C ";

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 3.0f });
    std::string label = std::string(icon) + entity.Tag.Name;
    bool open = ImGui::TreeNodeEx(
        (void*)(intptr_t)entity.ID, flags, "%s", label.c_str());
    ImGui::PopStyleVar();

    // Seleção
    if (ImGui::IsItemClicked())
    {
        DeselectAll();
        entity.Selected = true;
        m_Selected      = &entity;
    }

    // Context menu (RMB)
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Duplicar"))
        {
            // TODO: duplicar entidade
        }
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Error);
        if (ImGui::MenuItem("Deletar"))
            m_DeletePending = entity.ID;
        ImGui::PopStyleColor();
        ImGui::EndPopup();
    }

    if (open && !entity.Children.empty())
    {
        for (auto& child : entity.Children)
            DrawEntityNode(*child);
        ImGui::TreePop();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Inspector
// ─────────────────────────────────────────────────────────────────────────────
void Editor::DrawVec3Control(const char* label, glm::vec3& values, float resetValue)
{
    ImGui::PushID(label);
    float lineH  = ImGui::GetFrameHeight();
    float btnW   = lineH + 3.0f;
    float avail  = ImGui::GetContentRegionAvail().x;
    float labelW = 72.0f;
    float inputW = (avail - labelW - btnW * 3.0f
                    - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;
    if (inputW < 40.0f) inputW = 40.0f;

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text("%-9s", label);
    ImGui::PopStyleColor();
    ImGui::SameLine(labelW);

    struct Axis { const char* label; float* val; float r, g, b; };
    Axis axes[3] = {
        { "X", &values.x, 0.75f, 0.18f, 0.18f },
        { "Y", &values.y, 0.18f, 0.60f, 0.22f },
        { "Z", &values.z, 0.18f, 0.38f, 0.82f },
    };

    for (int i = 0; i < 3; i++)
    {
        ImGui::PushStyleColor(ImGuiCol_Button,
            ImVec4{ axes[i].r, axes[i].g, axes[i].b, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            ImVec4{ axes[i].r + 0.1f, axes[i].g + 0.1f, axes[i].b + 0.1f, 1.0f });
        if (ImGui::Button(axes[i].label, { btnW, lineH }))
            *axes[i].val = resetValue;
        ImGui::PopStyleColor(2);
        ImGui::SameLine(0, 0);
        ImGui::SetNextItemWidth(inputW);
        ImGui::DragFloat((std::string("##") + axes[i].label).c_str(),
                         axes[i].val, 0.1f);
        if (i < 2) ImGui::SameLine();
    }
    ImGui::PopID();
}

void Editor::DrawTransformComponent(SceneEntity& e)
{
    ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);
    bool open = ImGui::CollapsingHeader("  Transform",
                    ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::PopStyleColor(2);

    if (open)
    {
        ImGui::Spacing();
        DrawVec3Control("Position", e.Transform.Position, 0.0f);
        DrawVec3Control("Rotation", e.Transform.Rotation, 0.0f);
        DrawVec3Control("Scale",    e.Transform.Scale,    1.0f);
        ImGui::Spacing();
    }
}

void Editor::DrawCameraComponent(SceneEntity& e)
{
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);
    bool open = ImGui::CollapsingHeader("  Camera",
                    ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::PopStyleColor(2);

    if (open && e.Camera)
    {
        ImGui::Spacing();

        // Primary badge
        if (e.Camera->IsPrimary)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Success);
            ImGui::Text("Câmera Principal");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::Text("Câmera Secundária");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::SmallButton("Tornar Principal"))
            {
                // Remove primary de todas
                for (auto& ent : m_Entities)
                    if (ent->HasCamera()) ent->Camera->IsPrimary = false;
                e.Camera->IsPrimary = true;
            }
        }

        ImGui::Spacing();
        ImGui::Checkbox("Ortográfica", &e.Camera->IsOrthographic);
        if (!e.Camera->IsOrthographic)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::Text("FOV");
            ImGui::PopStyleColor();
            ImGui::SameLine(72.0f);
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::DragFloat("##fov", &e.Camera->FOV, 0.5f, 10.0f, 170.0f, "%.1f");
        }

        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("Near");
        ImGui::PopStyleColor();
        ImGui::SameLine(72.0f);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::DragFloat("##near", &e.Camera->Near, 0.01f, 0.01f, 10.0f, "%.3f");

        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("Far");
        ImGui::PopStyleColor();
        ImGui::SameLine(72.0f);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::DragFloat("##far",  &e.Camera->Far,  1.0f, 10.0f, 10000.0f, "%.0f");

        ImGui::Spacing();
    }
}

void Editor::DrawInspector()
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::Color::BgBase);
    if (ImGui::Begin("Inspector"))
    {
        if (!m_Selected)
        {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::TextWrapped("Selecione uma entidade na Hierarchy.");
            ImGui::PopStyleColor();
            ImGui::End();
            ImGui::PopStyleColor();
            return;
        }

        // Ativo / Nome
        ImGui::Spacing();
        ImGui::Checkbox("##active", &m_Selected->Active);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::Color::BgInput);
        ImGui::SetNextItemWidth(-1.0f);
        char nameBuf[128];
        std::strncpy(nameBuf, m_Selected->Tag.Name.c_str(), sizeof(nameBuf) - 1);
        if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf)))
            m_Selected->Tag.Name = nameBuf;
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();

        // Componentes
        DrawTransformComponent(*m_Selected);
        if (m_Selected->HasCamera())
            DrawCameraComponent(*m_Selected);

        // Botão Add Component
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        PushAccentColor();
        if (ImGui::Button("+ Add Component",
                { ImGui::GetContentRegionAvail().x, 28.0f }))
            ImGui::OpenPopup("AddComp");
        PopAccentColor();

        if (ImGui::BeginPopup("AddComp"))
        {
            ImGui::SeparatorText("Components");
            if (!m_Selected->HasCamera())
                if (ImGui::MenuItem("Camera"))
                {
                    m_Selected->Camera = std::make_unique<CameraComponent>();
                    if (!GetPrimaryCamera())
                        m_Selected->Camera->IsPrimary = true;
                    Log(LogLevel::Info, "Componente Camera adicionado.");
                }
            ImGui::MenuItem("Rigidbody");
            ImGui::MenuItem("Collider");
            ImGui::MenuItem("AudioSource");
            ImGui::MenuItem("Script");
            ImGui::EndPopup();
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Console
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
                case LogLevel::Warning: col = EditorTheme::Color::Warning; pfx = "[WARN] "; break;
                case LogLevel::Error:   col = EditorTheme::Color::Error;   pfx = "[ERR]  "; break;
                case LogLevel::Success: col = EditorTheme::Color::Success; pfx = "[OK]   "; break;
                default:                col = EditorTheme::Color::TextNormal; pfx = "[INFO] "; break;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, col);
            ImGui::TextUnformatted(pfx);
            ImGui::PopStyleColor();
            ImGui::SameLine(0, 0);
            ImGui::TextUnformatted(entry.Message.c_str());
        }

        if (m_ScrollToBottom)
        {
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