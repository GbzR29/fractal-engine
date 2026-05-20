/**
 * @file EditorInspector.cpp
 * @brief Inspector panel: per-entity component UI (Transform, Camera, Mesh, Light,
 *        Script, Audio).
 *
 * Each DrawXxxComponent() function renders a collapsible header with an optional
 * X button to remove the component, followed by the component's editable fields.
 * DrawVec3Control() provides the axis-coloured XYZ drag control reused across
 * multiple components.
 */

#include "Editor.hpp"
#include "Model.hpp"
#include "AssetLoader.hpp"
#include "LuaScriptEngine.hpp"
#include "AudioEngine.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
//  Shared helpers
// ─────────────────────────────────────────────────────────────────────────────

/** @brief Renders a right-aligned X button on the same line as a CollapsingHeader.
 *  @return true when the button was clicked (caller should remove the component). */
static bool ComponentRemoveButton(const char* id)
{
    ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - 22.0f);
    ImGui::PushStyleColor(ImGuiCol_Button,        { 0.0f, 0.0f, 0.0f, 0.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::Error);
    ImGui::PushStyleColor(ImGuiCol_Text,          EditorTheme::Color::TextDim);
    bool clicked = ImGui::SmallButton(id);
    ImGui::PopStyleColor(3);
    return clicked;
}

// ─────────────────────────────────────────────────────────────────────────────
//  XYZ vec3 drag control
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawVec3Control(const char* label, glm::vec3& values, float resetValue)
{
    ImGui::PushID(label);
    float lineH = ImGui::GetFrameHeight();
    float btnW  = lineH + 3.0f;
    float avail = ImGui::GetContentRegionAvail().x;
    float sp    = ImGui::GetStyle().ItemSpacing.x;

    // Shrink label width when the panel is too narrow to fit full controls
    float labelW   = 64.0f;
    float minInput = 36.0f;
    float fixedW   = labelW + (btnW * 3.0f) + (sp * 2.0f);
    float inputW   = (avail - fixedW) / 3.0f;
    if (inputW < minInput) {
        labelW  = std::max(0.0f, labelW - (minInput - inputW) * 3.0f);
        fixedW  = labelW + (btnW * 3.0f) + (sp * 2.0f);
        inputW  = std::max(minInput, (avail - fixedW) / 3.0f);
    }

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();
    ImGui::SameLine(labelW);

    struct Axis { const char* lbl; float* val; float r, g, b; };
    Axis axes[3] = {
        { "X", &values.x, 0.76f, 0.18f, 0.18f },
        { "Y", &values.y, 0.18f, 0.62f, 0.24f },
        { "Z", &values.z, 0.18f, 0.38f, 0.84f },
    };

    for (int i = 0; i < 3; i++) {
        ImGui::PushStyleColor(ImGuiCol_Button,
            ImVec4{ axes[i].r, axes[i].g, axes[i].b, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            ImVec4{ axes[i].r + 0.10f, axes[i].g + 0.10f, axes[i].b + 0.10f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextBright);
        if (ImGui::Button(axes[i].lbl, { btnW, lineH }))
            *axes[i].val = resetValue;
        ImGui::PopStyleColor(3);
        ImGui::SameLine(0, 0);
        ImGui::SetNextItemWidth(inputW);
        ImGui::DragFloat((std::string("##v") + axes[i].lbl).c_str(),
                         axes[i].val, 0.1f);
        if (i < 2) ImGui::SameLine(0, sp);
    }
    ImGui::PopID();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Transform
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawTransformComponent(SceneEntity& e)
{
    ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);
    bool open = ImGui::CollapsingHeader("  Transform",
                    ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::PopStyleColor(2);

    if (!open) return;
    ImGui::Spacing();
    DrawVec3Control("Position", e.Transform.Position, 0.0f);
    ImGui::Spacing();
    DrawVec3Control("Rotation", e.Transform.Rotation, 0.0f);
    ImGui::Spacing();
    DrawVec3Control("Scale",    e.Transform.Scale,    1.0f);
    ImGui::Spacing();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Camera
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawCameraComponent(SceneEntity& e)
{
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);
    bool open = ImGui::CollapsingHeader("  Camera",
                    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);
    ImGui::PopStyleColor(2);
    if (ComponentRemoveButton("X##rmvCam")) { e.Camera.reset(); return; }
    if (!open || !e.Camera) return;

    auto& cam = *e.Camera;
    ImGui::Spacing();

    if (cam.IsPrimary) {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Success);
        ImGui::Text("Primary Camera");
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("Secondary Camera");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::SmallButton("Make Primary")) {
            for (auto& ent : m_Entities)
                if (ent->HasCamera()) ent->Camera->IsPrimary = false;
            cam.IsPrimary = true;
        }
    }

    ImGui::Spacing();
    ImGui::Checkbox("Orthographic", &cam.IsOrthographic);
    if (!cam.IsOrthographic) {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("FOV");
        ImGui::PopStyleColor();
        ImGui::SameLine(72.0f);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::DragFloat("##fov", &cam.FOV, 0.5f, 10.0f, 170.0f, "%.1f");
    }

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text("Near"); ImGui::PopStyleColor();
    ImGui::SameLine(72.0f); ImGui::SetNextItemWidth(-1.0f);
    ImGui::DragFloat("##near", &cam.Near, 0.01f, 0.01f, 10.0f, "%.3f");

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text("Far"); ImGui::PopStyleColor();
    ImGui::SameLine(72.0f); ImGui::SetNextItemWidth(-1.0f);
    ImGui::DragFloat("##far", &cam.Far, 1.0f, 10.0f, 10000.0f, "%.0f");
    ImGui::Spacing();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Mesh Renderer
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawMeshRendererComponent(SceneEntity& e)
{
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);
    bool open = ImGui::CollapsingHeader("  Mesh Renderer",
                    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);
    ImGui::PopStyleColor(2);
    if (ComponentRemoveButton("X##rmvMesh")) { e.MeshRenderer.reset(); return; }
    if (!open || !e.MeshRenderer) return;

    auto& mr = *e.MeshRenderer;
    ImGui::Spacing();

    if (mr.HasModel()) {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Success);
        ImGui::Text("  Model loaded");
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("  Meshes: %zu  |  Materials: %zu  |  Anims: %zu",
            mr.model->meshes.size(), mr.model->materials.size(),
            mr.model->animations.size());
        if (mr.model->hasSkeleton())
            ImGui::Text("  Bones: %d", mr.model->skeleton->boneCount);
        ImGui::PopStyleColor();
    } else if (!mr.modelPath.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Warning);
        ImGui::Text("  Model not loaded");
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("  No model selected");
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();

    if (mr.isPrimitive) {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("  Geometry: %s (procedural)", mr.modelPath.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
        return;
    }

#ifndef FE_USE_ASSIMP
    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Warning);
    ImGui::TextWrapped("  Assimp not compiled. Place source in include/third_party/assimp/ or install via vcpkg.");
    ImGui::PopStyleColor();
    ImGui::Spacing();
#endif

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text("Path:");
    ImGui::PopStyleColor();

    char pathBuf[512] = {};
    std::strncpy(pathBuf, mr.modelPath.c_str(), sizeof(pathBuf) - 1);

    ImGui::PushID((int)e.ID);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 64.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::Color::BgInput);
    if (ImGui::InputText("##modelPath", pathBuf, sizeof(pathBuf)))
        mr.modelPath = pathBuf;
    ImGui::PopStyleColor();
    ImGui::SameLine();

    PushAccentColor();
    if (ImGui::Button("Load", { 58.0f, 0 })) {
        const std::string path(pathBuf);
        if (!path.empty()) {
            mr.modelPath = path;
            auto model   = AssetLoader::loadModel(path);
            if (model) {
                mr.model = model;
                Log(LogLevel::Success, "Model loaded: " + path);
            } else {
                Log(LogLevel::Error, "Failed to load: " + path);
            }
        }
    }
    PopAccentColor();

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
            const std::string droppedPath(static_cast<const char*>(p->Data));
            mr.modelPath = droppedPath;
            auto model   = AssetLoader::loadModel(droppedPath);
            if (model) {
                mr.model = model;
                Log(LogLevel::Success, "Model loaded: " + droppedPath);
            } else {
                Log(LogLevel::Error, "Failed to load: " + droppedPath);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (mr.HasModel()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
        if (ImGui::SmallButton("Remove model")) {
            mr.model.reset();
            mr.animator.reset();
            mr.modelPath.clear();
            Log(LogLevel::Info, "Model removed.");
        }
        ImGui::PopStyleColor(2);
    }

    ImGui::PopID();
    ImGui::Spacing();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Directional Light
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawDirectionalLightComponent(SceneEntity& e)
{
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);
    bool open = ImGui::CollapsingHeader("  Directional Light",
                    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);
    ImGui::PopStyleColor(2);
    if (ComponentRemoveButton("X##rmvDLight")) { e.DirectionalLight.reset(); return; }
    if (!open || !e.DirectionalLight) return;

    auto& dl = *e.DirectionalLight;
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::TextWrapped("Use Transform > Rotation to control light direction.");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text("%-9s", "Color");
    ImGui::PopStyleColor();
    ImGui::SameLine(72.0f);
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::ColorEdit3("##lightColor", &dl.Color.x);

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text("%-9s", "Intensity");
    ImGui::PopStyleColor();
    ImGui::SameLine(72.0f);
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::DragFloat("##lightIntensity", &dl.Intensity, 0.05f, 0.0f, 20.0f, "%.2f");
    ImGui::Spacing();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Script (Lua)
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawScriptComponent(SceneEntity& e)
{
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);
    bool open = ImGui::CollapsingHeader("  Script (Lua)",
                    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);
    ImGui::PopStyleColor(2);
    if (ComponentRemoveButton("X##rmvScript")) { e.Script.reset(); return; }
    if (!open || !e.Script) return;

    auto& sc = *e.Script;
    ImGui::Spacing();

#ifndef FE_ENABLE_LUA
    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Warning);
    ImGui::TextWrapped("Lua not compiled. Add source to include/third_party/lua/.");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    return;
#endif

    if (sc.loaded) {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Success);
        ImGui::TextUnformatted("  Script loaded");
        ImGui::PopStyleColor();
    } else if (!sc.lastError.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Error);
        ImGui::TextWrapped("  Error: %s", sc.lastError.c_str());
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::TextUnformatted("  No script loaded");
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::TextUnformatted("Script:");
    ImGui::PopStyleColor();

    ImGui::PushID((int)e.ID + 5000);
    char pathBuf[512] = {};
    std::strncpy(pathBuf, sc.scriptPath.c_str(), sizeof(pathBuf) - 1);

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 64.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::Color::BgInput);
    if (ImGui::InputText("##scriptPath", pathBuf, sizeof(pathBuf)))
        sc.scriptPath = pathBuf;
    ImGui::PopStyleColor();
    ImGui::SameLine();

    PushAccentColor();
    if (ImGui::Button("Load", { 58.0f, 0 }) && m_ScriptEngine) {
        sc.scriptPath = pathBuf;
        m_ScriptEngine->ReloadScript(e);
        if (sc.loaded)
            Log(LogLevel::Success, "Script loaded: " + sc.scriptPath);
        else
            Log(LogLevel::Error, "Script error: " + sc.lastError);
    }
    PopAccentColor();

    if (ImGui::TreeNode("API reference")) {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::TextUnformatted("self.position.x/y/z");
        ImGui::TextUnformatted("self.rotation.x/y/z");
        ImGui::TextUnformatted("self.scale.x/y/z");
        ImGui::TextUnformatted("self.name  |  self.active");
        ImGui::TextUnformatted("print(msg)");
        ImGui::Separator();
        ImGui::TextUnformatted("function OnStart() end");
        ImGui::TextUnformatted("function OnUpdate(dt) end");
        ImGui::TextUnformatted("function OnDestroy() end");
        ImGui::PopStyleColor();
        ImGui::TreePop();
    }

    ImGui::PopID();
    ImGui::Spacing();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Audio Listener
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawAudioListenerComponent(SceneEntity& e)
{
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);
    bool open = ImGui::CollapsingHeader("  Audio Listener",
                    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);
    ImGui::PopStyleColor(2);
    if (ComponentRemoveButton("X##rmvAList")) { e.AudioListener.reset(); return; }
    if (!open || !e.AudioListener) return;

    auto& al = *e.AudioListener;
    ImGui::Spacing();
    ImGui::Checkbox("Enabled", &al.Enabled);
    ImGui::SameLine(120.0f);
    ImGui::Checkbox("Muted",   &al.Muted);
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text("%-9s", "Volume");
    ImGui::PopStyleColor();
    ImGui::SameLine(72.0f);
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::DragFloat("##alVol", &al.Volume, 0.01f, 0.0f, 2.0f, "%.2f");
    ImGui::Spacing();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Audio Source
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawAudioSourceComponent(SceneEntity& e)
{
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);
    bool open = ImGui::CollapsingHeader("  Audio Source",
                    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);
    ImGui::PopStyleColor(2);
    if (ComponentRemoveButton("X##rmvASrc")) { e.AudioSource.reset(); return; }
    if (!open || !e.AudioSource) return;

    auto& as = *e.AudioSource;
    ImGui::PushID((int)e.ID + 9000);
    ImGui::Spacing();

    if (as.Playing) {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Success);
        ImGui::TextUnformatted("  Playing");
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::TextUnformatted("File:");
    ImGui::PopStyleColor();
    char pathBuf[512] = {};
    std::strncpy(pathBuf, as.soundPath.c_str(), sizeof(pathBuf) - 1);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::Color::BgInput);
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputText("##sndPath", pathBuf, sizeof(pathBuf)))
        as.soundPath = pathBuf;
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text("%-9s", "Volume"); ImGui::PopStyleColor();
    ImGui::SameLine(72.0f); ImGui::SetNextItemWidth(-1.0f);
    ImGui::DragFloat("##asVol",   &as.Volume, 0.01f,  0.0f,   2.0f, "%.2f");

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text("%-9s", "Pitch"); ImGui::PopStyleColor();
    ImGui::SameLine(72.0f); ImGui::SetNextItemWidth(-1.0f);
    ImGui::DragFloat("##asPitch", &as.Pitch,  0.01f,  0.1f,   4.0f, "%.2f");

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text("%-9s", "Range"); ImGui::PopStyleColor();
    ImGui::SameLine(72.0f); ImGui::SetNextItemWidth(-1.0f);
    ImGui::DragFloat("##asRange", &as.Range,  0.5f,   1.0f, 500.0f, "%.0f");
    ImGui::Spacing();

    ImGui::Checkbox("Loop",         &as.Loop);
    ImGui::SameLine(90.0f);
    ImGui::Checkbox("3D",           &as.Is3D);
    ImGui::SameLine(150.0f);
    ImGui::Checkbox("Play on Start",&as.PlayOnStart);
    ImGui::Spacing();

    if (m_AudioEngine) {
        if (!as.Playing) {
            PushAccentColor();
            if (ImGui::Button("Play##asPlay", { 60.0f, 0.0f }))
                m_AudioEngine->Play(e);
            PopAccentColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4{ 0.70f, 0.15f, 0.15f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                ImVec4{ 0.90f, 0.25f, 0.25f, 1.0f });
            if (ImGui::Button("Stop##asStop", { 60.0f, 0.0f }))
                m_AudioEngine->Stop(e);
            ImGui::PopStyleColor(2);
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
            if (ImGui::Button("Pause##asPause", { 60.0f, 0.0f }))
                m_AudioEngine->Pause(e);
            ImGui::PopStyleColor(2);
        }
    }

    ImGui::PopID();
    ImGui::Spacing();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Inspector panel
// ─────────────────────────────────────────────────────────────────────────────

void Editor::DrawInspector()
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::Color::BgBase);
    if (ImGui::Begin("Inspector"))
    {
        if (!m_Selected) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::TextWrapped("Select an entity in the Hierarchy.");
            ImGui::PopStyleColor();
            ImGui::End();
            ImGui::PopStyleColor();
            return;
        }

        // ── Entity header card ─────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_ChildBg, EditorTheme::Color::BgPanel);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
        if (ImGui::BeginChild("##entityCard", { 0.0f, 44.0f }, false))
        {
            ImVec2 wp = ImGui::GetWindowPos();
            ImGui::GetWindowDrawList()->AddRectFilled(
                wp, { wp.x + 3.0f, wp.y + 44.0f },
                ImGui::ColorConvertFloat4ToU32(EditorTheme::Color::Accent)
            );
            ImGui::SetCursorPos({ 12.0f, 10.0f });
            ImGui::Checkbox("##active", &m_Selected->Active);
            ImGui::SameLine(0, 8.0f);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.0f, 0.0f, 0.0f, 0.0f });
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 50.0f);
            char nameBuf[128];
            std::strncpy(nameBuf, m_Selected->Tag.Name.c_str(), sizeof(nameBuf) - 1);
            nameBuf[sizeof(nameBuf) - 1] = '\0';
            if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf)))
                m_Selected->Tag.Name = nameBuf;
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::Text("#%u", m_Selected->ID);
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::Spacing();

        // Components
        DrawTransformComponent(*m_Selected);
        if (m_Selected->HasCamera())           DrawCameraComponent(*m_Selected);
        if (m_Selected->HasMeshRenderer())     DrawMeshRendererComponent(*m_Selected);
        if (m_Selected->HasDirectionalLight()) DrawDirectionalLightComponent(*m_Selected);
        if (m_Selected->HasScript())           DrawScriptComponent(*m_Selected);
        if (m_Selected->HasAudioListener())    DrawAudioListenerComponent(*m_Selected);
        if (m_Selected->HasAudioSource())      DrawAudioSourceComponent(*m_Selected);

        // ── Add Component button ───────────────────────────────────────────
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
            ImGui::SeparatorText("Logic");
            if (!m_Selected->HasCamera())
                if (ImGui::MenuItem("Camera")) {
                    m_Selected->Camera = std::make_unique<CameraComponent>();
                    if (!GetPrimaryCamera()) m_Selected->Camera->IsPrimary = true;
                    Log(LogLevel::Info, "Camera component added.");
                }
            if (!m_Selected->HasMeshRenderer())
                if (ImGui::MenuItem("Mesh Renderer")) {
                    m_Selected->MeshRenderer = std::make_unique<MeshRendererComponent>();
                    Log(LogLevel::Info, "Mesh Renderer component added.");
                }
            if (!m_Selected->HasDirectionalLight())
                if (ImGui::MenuItem("Directional Light")) {
                    m_Selected->DirectionalLight = std::make_unique<DirectionalLightComponent>();
                    Log(LogLevel::Info, "Directional Light component added.");
                }
            if (!m_Selected->HasScript())
                if (ImGui::MenuItem("Script (Lua)")) {
                    m_Selected->Script = std::make_unique<ScriptComponent>();
                    Log(LogLevel::Info, "Script component added.");
                }
            ImGui::SeparatorText("Audio");
            if (!m_Selected->HasAudioListener())
                if (ImGui::MenuItem("Audio Listener")) {
                    m_Selected->AudioListener = std::make_unique<AudioListenerComponent>();
                    Log(LogLevel::Info, "Audio Listener added.");
                }
            if (!m_Selected->HasAudioSource())
                if (ImGui::MenuItem("Audio Source")) {
                    m_Selected->AudioSource = std::make_unique<AudioSourceComponent>();
                    Log(LogLevel::Info, "Audio Source added.");
                }
            ImGui::SeparatorText("Physics");
            ImGui::MenuItem("Rigidbody");
            ImGui::MenuItem("Collider");
            ImGui::EndPopup();
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
}
