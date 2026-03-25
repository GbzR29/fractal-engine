#include "EditorViewport.hpp"
#include "EditorTheme.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#ifdef FE_HAS_IMGUIZMO
#include <ImGuizmo.h>
#endif

bool EditorViewport::Init(int width, int height)
{
    FramebufferSpec spec{ width, height };
    m_FBO.Init(spec);
    m_Camera.SetViewportSize((float)width, (float)height);
    m_GridInit = m_Grid.Init();
    return m_GridInit;
}

// ─────────────────────────────────────────────────────────────────────────────
void EditorViewport::BindFramebuffer()
{
    m_FBO.Bind();
    glClearColor(0.09f, 0.09f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void EditorViewport::UnbindFramebuffer()
{
    m_FBO.Unbind();
}

void EditorViewport::DrawGrid()
{
    if (!m_GridInit) return;
    m_Grid.Draw(
        m_Camera.GetViewMatrix(),
        m_Camera.GetProjectionMatrix(),
        m_Camera.GetPosition()
    );
}

// ─────────────────────────────────────────────────────────────────────────────
//  Begin — janela "Scene"
// ─────────────────────────────────────────────────────────────────────────────
void EditorViewport::Begin(float deltaTime)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.09f, 0.09f, 0.10f, 1.0f });
    ImGui::Begin("Scene");
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    m_Hovered = ImGui::IsWindowHovered();
    m_Focused = ImGui::IsWindowFocused();
    m_WinPos  = ImGui::GetWindowPos();

    // Resize do FBO se necessário
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.x < 4.0f) avail.x = 4.0f;
    if (avail.y < 4.0f) avail.y = 4.0f;
    if ((int)avail.x != m_FBO.GetWidth() || (int)avail.y != m_FBO.GetHeight())
    {
        m_FBO.Resize((int)avail.x, (int)avail.y);
        m_Camera.SetViewportSize(avail.x, avail.y);
        m_Size = avail;
    }

    // Imagem do FBO (UV invertido para OpenGL)
    ImGui::Image(
        (ImTextureID)(intptr_t)m_FBO.GetColorAttachment(),
        avail, { 0, 1 }, { 1, 0 }
    );

    // Drag & drop
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("ASSET_PATH"))
            (void)p; // TODO: instanciar entidade
        ImGui::EndDragDropTarget();
    }

    // Câmera
    if (m_Hovered || m_Focused)
        m_Camera.OnUpdate(deltaTime);

    // ImGuizmo
#ifdef FE_HAS_IMGUIZMO
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(m_WinPos.x, m_WinPos.y, m_Size.x, m_Size.y);
#endif

    // Overlays
    DrawOverlayToolbar();
    DrawInfoOverlay();
}

void EditorViewport::End()
{
    ImGui::End();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Gizmo
// ─────────────────────────────────────────────────────────────────────────────
void EditorViewport::DrawGizmo(glm::mat4& transform)
{
#ifdef FE_HAS_IMGUIZMO
    if ((m_Focused || m_Hovered) && !ImGui::IsAnyItemActive())
    {
        if (ImGui::IsKeyPressed(ImGuiKey_W)) m_GizmoMode = GizmoMode::Translate;
        if (ImGui::IsKeyPressed(ImGuiKey_E)) m_GizmoMode = GizmoMode::Rotate;
        if (ImGui::IsKeyPressed(ImGuiKey_R)) m_GizmoMode = GizmoMode::Scale;
    }

    ImGuizmo::OPERATION op;
    switch (m_GizmoMode)
    {
        case GizmoMode::Rotate: op = ImGuizmo::ROTATE;    break;
        case GizmoMode::Scale:  op = ImGuizmo::SCALE;     break;
        default:                op = ImGuizmo::TRANSLATE; break;
    }

    ImGuizmo::Manipulate(
        glm::value_ptr(m_Camera.GetViewMatrix()),
        glm::value_ptr(m_Camera.GetProjectionMatrix()),
        op, ImGuizmo::LOCAL,
        glm::value_ptr(transform)
    );
#else
    (void)transform;
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  Overlay toolbar (T/R/S)
// ─────────────────────────────────────────────────────────────────────────────
void EditorViewport::DrawOverlayToolbar()
{
    ImVec2 winPos  = ImGui::GetWindowPos();
    ImVec2 winSize = ImGui::GetWindowSize();
    float  btnW    = 52.0f, btnH = 22.0f, pad = 6.0f;
    float  barW    = btnW * 3.0f + pad * 4.0f;

    ImGui::SetNextWindowPos({
        winPos.x + (winSize.x - barW) * 0.5f,
        winPos.y + 10.0f
    }, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ barW, btnH + pad * 2.0f });
    ImGui::SetNextWindowBgAlpha(0.82f);

    ImGuiWindowFlags f =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  { pad, pad });
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,    { pad * 0.5f, 0.0f });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.12f, 0.12f, 0.14f, 0.90f });

    if (ImGui::Begin("##SceneToolbar", nullptr, f))
    {
        auto Btn = [&](const char* label, GizmoMode mode, const char* tip)
        {
            bool active = (m_GizmoMode == mode);
            ImGui::PushStyleColor(ImGuiCol_Button,
                active ? EditorTheme::Color::Accent : EditorTheme::Color::BgPanel);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::AccentHover);
            if (ImGui::Button(label, { btnW, btnH })) m_GizmoMode = mode;
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tip);
            ImGui::PopStyleColor(2);
            ImGui::SameLine();
        };

        Btn("T  Mov", GizmoMode::Translate, "Translate [W]");
        Btn("R  Rot", GizmoMode::Rotate,    "Rotate    [E]");
        Btn("S  Esc", GizmoMode::Scale,     "Scale     [R]");
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Overlay info (canto inferior esquerdo)
// ─────────────────────────────────────────────────────────────────────────────
void EditorViewport::DrawInfoOverlay()
{
    ImVec2 winPos  = ImGui::GetWindowPos();
    ImVec2 winSize = ImGui::GetWindowSize();

    ImGui::SetNextWindowPos({
        winPos.x + 8.0f,
        winPos.y + winSize.y - 28.0f
    }, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowSize({ 320.0f, 22.0f });

    ImGuiWindowFlags f =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
    if (ImGui::Begin("##SceneInfo", nullptr, f))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("SCENE  %.0fx%.0f  |  Alt+LMB orbitar  |  MMB pan  |  Scroll zoom",
            m_Size.x, m_Size.y);
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}