/**
 * @file EditorProject.cpp
 * @brief Scene save / load / new operations and their UI dialogs.
 *
 * Scenes are stored as JSON files under assets/scenes/.
 * Format version 1 serialises entities with their transforms, camera, and
 * mesh renderer path; other components will be added in future versions.
 */

#include "Editor.hpp"
#include "AssetLoader.hpp"
#include "Model.hpp"
#include <imgui.h>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────────────────────
//  Scene lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void Editor::NewProject()
{
    m_Entities.clear();
    m_Selected    = nullptr;
    m_ProjectPath = "";
    m_ProjectName = "Untitled";
    Log(LogLevel::Info, "New scene created.");
}

bool Editor::SaveProject(const std::string& path)
{
    try {
        json root;
        root["version"]  = 1;
        root["name"]     = m_ProjectName;
        root["entities"] = json::array();

        for (const auto& e : m_Entities) {
            json ej;
            ej["id"]     = e->ID;
            ej["name"]   = e->Tag.Name;
            ej["active"] = e->Active;

            json tr;
            tr["px"] = e->Transform.Position.x; tr["py"] = e->Transform.Position.y;
            tr["pz"] = e->Transform.Position.z;
            tr["rx"] = e->Transform.Rotation.x; tr["ry"] = e->Transform.Rotation.y;
            tr["rz"] = e->Transform.Rotation.z;
            tr["sx"] = e->Transform.Scale.x;    tr["sy"] = e->Transform.Scale.y;
            tr["sz"] = e->Transform.Scale.z;
            ej["transform"] = tr;

            if (e->HasCamera()) {
                json cam;
                cam["fov"]       = e->Camera->FOV;
                cam["near"]      = e->Camera->Near;
                cam["far"]       = e->Camera->Far;
                cam["isPrimary"] = e->Camera->IsPrimary;
                cam["isOrtho"]   = e->Camera->IsOrthographic;
                ej["camera"] = cam;
            } else {
                ej["camera"] = nullptr;
            }

            ej["meshRenderer"] = e->HasMeshRenderer()
                ? json(e->MeshRenderer->modelPath)
                : json(nullptr);

            root["entities"].push_back(ej);
        }

        std::filesystem::create_directories(
            std::filesystem::path(path).parent_path());

        std::ofstream file(path);
        if (!file.is_open()) {
            Log(LogLevel::Error, "Failed to save: " + path);
            return false;
        }
        file << root.dump(2);
        m_ProjectPath = path;
        m_ProjectName = std::filesystem::path(path).stem().string();
        Log(LogLevel::Success, "Scene saved: " + path);
        return true;
    } catch (const std::exception& ex) {
        Log(LogLevel::Error, std::string("Save error: ") + ex.what());
        return false;
    }
}

bool Editor::LoadProject(const std::string& path)
{
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            Log(LogLevel::Error, "File not found: " + path);
            return false;
        }

        json root = json::parse(file);
        m_Entities.clear();
        m_Selected    = nullptr;
        m_ProjectName = root.value("name", "Untitled");

        for (const auto& ej : root["entities"]) {
            auto e = std::make_unique<SceneEntity>(ej["name"].get<std::string>());
            e->Active = ej.value("active", true);

            const auto& tr = ej["transform"];
            e->Transform.Position = { tr["px"], tr["py"], tr["pz"] };
            e->Transform.Rotation = { tr["rx"], tr["ry"], tr["rz"] };
            e->Transform.Scale    = { tr["sx"], tr["sy"], tr["sz"] };

            if (!ej["camera"].is_null()) {
                const auto& cam = ej["camera"];
                e->Camera = std::make_unique<CameraComponent>();
                e->Camera->FOV            = cam.value("fov",       60.0f);
                e->Camera->Near           = cam.value("near",       0.1f);
                e->Camera->Far            = cam.value("far",     1000.0f);
                e->Camera->IsPrimary      = cam.value("isPrimary", true);
                e->Camera->IsOrthographic = cam.value("isOrtho",  false);
            }

            if (!ej["meshRenderer"].is_null()) {
                std::string modelPath = ej["meshRenderer"].get<std::string>();
                e->MeshRenderer = std::make_unique<MeshRendererComponent>();
                e->MeshRenderer->modelPath = modelPath;
                if (!modelPath.empty()) {
                    auto model = AssetLoader::loadModel(modelPath);
                    if (model) {
                        e->MeshRenderer->model = model;
                        Log(LogLevel::Success, "Model loaded: " + modelPath);
                    } else {
                        Log(LogLevel::Warning, "Model not found: " + modelPath);
                    }
                }
            }

            m_Entities.push_back(std::move(e));
        }

        m_ProjectPath = path;
        Log(LogLevel::Success, "Scene loaded: " + path +
            " (" + std::to_string(m_Entities.size()) + " entities)");
        return true;
    } catch (const std::exception& ex) {
        Log(LogLevel::Error, std::string("Load error: ") + ex.what());
        return false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Project dialogs (New / Save / Open)
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawProjectDialog()
{
    if (m_NewProjectConfirm) {
        ImGui::OpenPopup("ConfirmNewProject");
        m_NewProjectConfirm = false;
    }

    ImGui::SetNextWindowSize({ 320, 0 }, ImGuiCond_Always);
    if (ImGui::BeginPopupModal("ConfirmNewProject", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Warning);
        ImGui::Text("New scene");
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::TextWrapped("Create a new scene? Unsaved changes will be lost.");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::Warning);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.7f, 0.1f, 1.0f });
        if (ImGui::Button("Confirm", { 100, 0 })) {
            NewProject();
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
        if (ImGui::Button("Cancel", { 100, 0 }))
            ImGui::CloseCurrentPopup();
        ImGui::PopStyleColor(2);

        ImGui::EndPopup();
    }

    if (m_ProjectDialog != ProjectDialogMode::None) {
        const char* title = (m_ProjectDialog == ProjectDialogMode::Save)
            ? "Save Scene" : "Open Scene";
        ImGui::OpenPopup(title);
    }

    auto drawDialog = [&](const char* title, bool isSave) {
        ImGui::SetNextWindowSize({ 480, 0 }, ImGuiCond_Always);
        if (ImGui::BeginPopupModal(title, nullptr,
                ImGuiWindowFlags_AlwaysAutoResize))
        {
            m_ProjectDialog = ProjectDialogMode::None;

            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Accent);
            ImGui::Text("%s", title);
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::TextWrapped("Scene files live in assets/scenes/");
            ImGui::TextWrapped("Example: assets/scenes/my_scene.json");
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::Color::BgInput);
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputText("##projFile", m_ProjectFileBuf,
                sizeof(m_ProjectFileBuf));
            ImGui::PopStyleColor();
            ImGui::Spacing();
            ImGui::Separator();

            ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::Accent);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::AccentHover);
            const char* btnLabel = isSave ? "Save" : "Open";
            if (ImGui::Button(btnLabel, { 100, 0 }) && m_ProjectFileBuf[0] != '\0') {
                std::string p(m_ProjectFileBuf);
                if (!std::filesystem::path(p).is_absolute())
                    p = (AssetLoader::assetsRoot() / p).string();
                if (isSave) SaveProject(p);
                else        LoadProject(p);
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
            if (ImGui::Button("Cancel", { 100, 0 }))
                ImGui::CloseCurrentPopup();
            ImGui::PopStyleColor(2);

            ImGui::EndPopup();
        }
    };

    drawDialog("Save Scene", true);
    drawDialog("Open Scene", false);
}
