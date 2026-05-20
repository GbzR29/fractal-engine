/**
 * @file EditorHierarchy.cpp
 * @brief Hierarchy panel: entity list, entity creation, deletion, and duplication.
 *
 * Draws the left-side "Hierarchy" panel showing every SceneEntity in the scene.
 * Each entity row shows a type-colored badge strip, a visibility toggle, and
 * supports right-click context menus.
 */

#include "Editor.hpp"
#include "AssetLoader.hpp"
#include "Model.hpp"
#include "PrimitiveMesh.hpp"
#include <imgui.h>
#include <algorithm>
#include <filesystem>

// ─────────────────────────────────────────────────────────────────────────────
//  Entity creation
// ─────────────────────────────────────────────────────────────────────────────

void Editor::AddEntity(const std::string& name)
{
    DeselectAll();
    auto e = std::make_unique<SceneEntity>(name);

    if (name == "Directional Light") {
        e->DirectionalLight = std::make_unique<DirectionalLightComponent>();
        e->Transform.Rotation = { 45.0f, -30.0f, 0.0f };
        Log(LogLevel::Info, "Directional Light added.");
    } else {
        Log(LogLevel::Info, "Entity created: " + name);
    }

    e->Selected = true;
    m_Selected  = e.get();
    m_Entities.push_back(std::move(e));
}

void Editor::AddCameraEntity()
{
    DeselectAll();
    auto e = std::make_unique<SceneEntity>("Camera");
    e->Camera = std::make_unique<CameraComponent>();
    if (GetPrimaryCamera()) e->Camera->IsPrimary = false;

    e->Transform.Position = { 0.0f, 5.0f, 10.0f };
    e->Transform.Rotation = { -20.0f, 0.0f, 0.0f };

    e->Selected = true;
    m_Selected  = e.get();
    Log(LogLevel::Info, "Camera added.");
    m_Entities.push_back(std::move(e));
}

void Editor::AddPrimitiveEntity(const std::string& name, const std::string& primitive)
{
    DeselectAll();
    auto e = std::make_unique<SceneEntity>(name);
    e->MeshRenderer = std::make_unique<MeshRendererComponent>();
    e->MeshRenderer->modelPath   = primitive;
    e->MeshRenderer->isPrimitive = true;

    std::shared_ptr<Model> model;
    if      (primitive == "Cube")   model = PrimitiveMesh::cube();
    else if (primitive == "Sphere") model = PrimitiveMesh::sphere();
    else if (primitive == "Plane")  model = PrimitiveMesh::plane();

    if (model) {
        e->MeshRenderer->model = model;
        Log(LogLevel::Success, "Primitive created: " + name);
    } else {
        Log(LogLevel::Error, "Failed to create primitive: " + primitive);
    }

    e->Selected = true;
    m_Selected  = e.get();
    m_Entities.push_back(std::move(e));
}

void Editor::AddEntityFromAsset(const std::string& relativePath)
{
    namespace fs = std::filesystem;
    std::string name = fs::path(relativePath).stem().string();
    if (name.empty()) name = "Entity";

    DeselectAll();
    auto e = std::make_unique<SceneEntity>(name);
    e->MeshRenderer = std::make_unique<MeshRendererComponent>();
    e->MeshRenderer->modelPath = relativePath;

    auto model = AssetLoader::loadModel(relativePath);
    if (model) {
        e->MeshRenderer->model = model;
        Log(LogLevel::Success, "Model instantiated: " + relativePath);
    } else {
        Log(LogLevel::Warning, "Model not loaded: " + relativePath);
    }

    e->Selected = true;
    m_Selected  = e.get();
    m_Entities.push_back(std::move(e));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Entity removal and duplication
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DeleteEntity(uint32_t id)
{
    m_Entities.erase(
        std::remove_if(m_Entities.begin(), m_Entities.end(),
            [id](const auto& e){ return e->ID == id; }),
        m_Entities.end()
    );
    if (m_Selected && m_Selected->ID == id)
        m_Selected = nullptr;
    Log(LogLevel::Info, "Entity removed.");
}

void Editor::DuplicateEntity(uint32_t id)
{
    for (const auto& src : m_Entities) {
        if (src->ID != id) continue;

        DeselectAll();
        auto copy = std::make_unique<SceneEntity>(src->Tag.Name + " (Copy)");
        copy->Active    = src->Active;
        copy->Transform = src->Transform;

        if (src->HasCamera()) {
            copy->Camera = std::make_unique<CameraComponent>(*src->Camera);
            copy->Camera->IsPrimary = false;
        }
        if (src->HasMeshRenderer())
            copy->MeshRenderer = std::make_unique<MeshRendererComponent>(*src->MeshRenderer);
        if (src->HasDirectionalLight())
            copy->DirectionalLight = std::make_unique<DirectionalLightComponent>(*src->DirectionalLight);
        if (src->HasScript()) {
            copy->Script = std::make_unique<ScriptComponent>(*src->Script);
            copy->Script->loaded = false;
        }
        if (src->HasAudioListener())
            copy->AudioListener = std::make_unique<AudioListenerComponent>(*src->AudioListener);
        if (src->HasAudioSource()) {
            copy->AudioSource = std::make_unique<AudioSourceComponent>(*src->AudioSource);
            copy->AudioSource->Playing      = false;
            copy->AudioSource->_startedOnce = false;
        }

        copy->Selected = true;
        m_Selected = copy.get();
        m_Entities.push_back(std::move(copy));
        Log(LogLevel::Info, "Entity duplicated.");
        return;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Hierarchy panel
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawHierarchy()
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::Color::BgBase);
    if (ImGui::Begin("Hierarchy"))
    {
        // Header: scene label + entity count + add button
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("SCENE  (%zu)", m_Entities.size());
        ImGui::PopStyleColor();
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 24.0f);

        PushAccentColor();
        if (ImGui::Button("+##addEntity", { 24.0f, 22.0f }))
            ImGui::OpenPopup("AddEntityPopup");
        PopAccentColor();

        if (ImGui::BeginPopup("AddEntityPopup"))
        {
            ImGui::SeparatorText("Entities");
            if (ImGui::MenuItem("Empty Entity"))      AddEntity("Entity");
            if (ImGui::MenuItem("Camera"))            AddCameraEntity();
            ImGui::SeparatorText("Primitives");
            if (ImGui::MenuItem("Cube"))              AddPrimitiveEntity("Cube",   "Cube");
            if (ImGui::MenuItem("Sphere"))            AddPrimitiveEntity("Sphere", "Sphere");
            if (ImGui::MenuItem("Plane"))             AddPrimitiveEntity("Plane",  "Plane");
            ImGui::SeparatorText("Lights");
            if (ImGui::MenuItem("Directional Light")) AddEntity("Directional Light");
            if (ImGui::MenuItem("Point Light"))       AddEntity("Point Light");
            ImGui::EndPopup();
        }

        ImGui::Separator();

        // Search bar with inline clear button
        static char searchBuf[64] = {};
        bool hasSearch = searchBuf[0] != '\0';

        ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::Color::BgInput);
        float searchW = hasSearch
            ? ImGui::GetContentRegionAvail().x - 28.0f
            : -1.0f;
        ImGui::SetNextItemWidth(searchW);
        ImGui::InputText("##search", searchBuf, sizeof(searchBuf));
        ImGui::PopStyleColor();

        if (hasSearch) {
            ImGui::SameLine(0, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
            if (ImGui::Button("x##clearSearch", { 24.0f, 0.0f }))
                searchBuf[0] = '\0';
            ImGui::PopStyleColor(2);
        }
        ImGui::Spacing();

        if (m_Entities.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::TextWrapped("Empty scene. Use + to add entities.");
            ImGui::PopStyleColor();
        }

        // Build lower-case search string for case-insensitive filtering
        std::string searchLower(searchBuf);
        std::transform(searchLower.begin(), searchLower.end(),
                       searchLower.begin(), ::tolower);

        for (auto& entity : m_Entities) {
            if (!searchLower.empty()) {
                std::string nameLower = entity->Tag.Name;
                std::transform(nameLower.begin(), nameLower.end(),
                               nameLower.begin(), ::tolower);
                if (nameLower.find(searchLower) == std::string::npos)
                    continue;
            }
            DrawEntityNode(*entity);
        }

        // Click on empty space = deselect
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
            && !ImGui::IsAnyItemHovered())
            DeselectAll();
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Single entity row
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawEntityNode(SceneEntity& entity)
{
    ImGui::PushID((int)entity.ID);

    // Select badge color by the entity's most significant component
    ImVec4 badgeColor = { 0.45f, 0.45f, 0.50f, 1.0f };
    if      (entity.HasCamera())              badgeColor = { 0.28f, 0.56f, 0.95f, 1.0f };
    else if (entity.HasDirectionalLight())    badgeColor = { 0.95f, 0.78f, 0.20f, 1.0f };
    else if (entity.HasMeshRenderer())        badgeColor = { 0.20f, 0.75f, 0.45f, 1.0f };
    else if (entity.HasScript())              badgeColor = { 0.75f, 0.45f, 0.90f, 1.0f };
    else if (entity.HasAudioSource()
          || entity.HasAudioListener())       badgeColor = { 0.90f, 0.55f, 0.20f, 1.0f };

    float rowH = ImGui::GetFrameHeight();
    float sp   = ImGui::GetStyle().ItemSpacing.x;

    // 3px colored left border drawn via the window DrawList
    ImVec2 screenCursor = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(
        screenCursor,
        { screenCursor.x + 3.0f, screenCursor.y + rowH },
        ImGui::ColorConvertFloat4ToU32(badgeColor)
    );
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5.0f);

    // Dim inactive entities
    if (!entity.Active)
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow    |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_FramePadding;

    if (entity.Children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (entity.Selected)
        flags |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 3.0f });
    bool open = ImGui::TreeNodeEx("##node", flags, "%s", entity.Tag.Name.c_str());
    ImGui::PopStyleVar();

    if (!entity.Active)
        ImGui::PopStyleColor();

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        DeselectAll();
        entity.Selected = true;
        m_Selected      = &entity;
    }

    if (ImGui::BeginPopupContextItem("##ctx")) {
        if (ImGui::MenuItem(entity.Active ? "Deactivate" : "Activate"))
            entity.Active = !entity.Active;
        if (ImGui::MenuItem("Duplicate"))
            m_DuplicatePending = entity.ID;
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Error);
        if (ImGui::MenuItem("Delete"))
            m_DeletePending = entity.ID;
        ImGui::PopStyleColor();
        ImGui::EndPopup();
    }

    // Visibility toggle (O = visible, - = hidden) right-aligned on the same row
    {
        float visW = 20.0f;
        float winW = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x;
        ImGui::SameLine(winW - visW - sp);
        ImGui::PushStyleColor(ImGuiCol_Button,        { 0.0f, 0.0f, 0.0f, 0.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
        ImGui::PushStyleColor(ImGuiCol_Text,
            entity.Active ? EditorTheme::Color::TextDim
                          : ImVec4{ 0.35f, 0.35f, 0.40f, 1.0f });
        if (ImGui::SmallButton(entity.Active ? "O##vis" : "-##vis"))
            entity.Active = !entity.Active;
        ImGui::PopStyleColor(3);
    }

    if (open && !entity.Children.empty()) {
        for (auto& child : entity.Children)
            DrawEntityNode(*child);
        ImGui::TreePop();
    }

    ImGui::PopID();
}
