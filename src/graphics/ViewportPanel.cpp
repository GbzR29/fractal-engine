#include "ViewportPanel.hpp"
#include "EditorTheme.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#ifdef FE_HAS_IMGUIZMO
#include <ImGuizmo.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────
void ViewportPanel::Init(int width, int height)
{
    FramebufferSpec spec;
    spec.Width  = width;
    spec.Height = height;
    m_Framebuffer.Init(spec);
    m_Camera.SetViewportSize((float)width, (float)height);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Begin
// ─────────────────────────────────────────────────────────────────────────────
bool ViewportPanel::Begin(float deltaTime)
{
    bool resized = false;

    // Sem padding para a imagem encostar nas bordas
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.05f, 0.05f, 0.06f, 1.0f });
    ImGui::Begin("Viewport");
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    m_Hovered = ImGui::IsWindowHovered();
    m_Focused = ImGui::IsWindowFocused();

    // ── Toolbar flutuante no topo da viewport ─────────────────────────────────
    DrawToolbar();

    // ── Tamanho disponível ────────────────────────────────────────────────────
    ImVec2 size = ImGui::GetContentRegionAvail();
    // Garante tamanho mínimo
    if (size.x < 8.0f) size.x = 8.0f;
    if (size.y < 8.0f) size.y = 8.0f;

    m_ViewportPos = ImGui::GetCursorScreenPos();

    if ((int)size.x != m_Framebuffer.GetWidth() ||
        (int)size.y != m_Framebuffer.GetHeight())
    {
        m_Framebuffer.Resize((int)size.x, (int)size.y);
        m_Camera.SetViewportSize(size.x, size.y);
        m_ViewportSize = size;
        resized = true;
    }

    // ── Exibe a textura do framebuffer ────────────────────────────────────────
    // UV invertido: OpenGL origin = bottom-left, ImGui = top-left
    ImGui::Image(
        (ImTextureID)(intptr_t)m_Framebuffer.GetColorAttachment(),
        size,
        ImVec2(0, 1),   // uv0
        ImVec2(1, 0)    // uv1
    );

    // ── Drag & Drop de assets para a cena ────────────────────────────────────
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("ASSET_PATH"))
        {
            // TODO: criar entidade com asset arrastado
            (void)payload;
        }
        ImGui::EndDragDropTarget();
    }

    // ── Câmera ────────────────────────────────────────────────────────────────
    if (m_Hovered || m_Focused)
        m_Camera.OnUpdate(deltaTime);

    // ── ImGuizmo setup ────────────────────────────────────────────────────────
#ifdef FE_HAS_IMGUIZMO
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(
        m_ViewportPos.x, m_ViewportPos.y,
        size.x, size.y
    );
#endif

    return resized;
}

void ViewportPanel::End()
{
    ImGui::End();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Gizmo
// ─────────────────────────────────────────────────────────────────────────────
void ViewportPanel::DrawGizmo(glm::mat4& transform)
{
#ifdef FE_HAS_IMGUIZMO
    // Atalhos de teclado só quando a viewport está focada
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

    const glm::mat4& view = m_Camera.GetViewMatrix();
    const glm::mat4& proj = m_Camera.GetProjectionMatrix();

    ImGuizmo::Manipulate(
        glm::value_ptr(view),
        glm::value_ptr(proj),
        op,
        ImGuizmo::LOCAL,
        glm::value_ptr(transform)
    );
#else
    (void)transform;
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  Toolbar — overlay no topo da viewport, não uma janela separada
// ─────────────────────────────────────────────────────────────────────────────
void ViewportPanel::DrawToolbar()
{
    // Posiciona dentro da viewport como overlay
    ImVec2 windowPos  = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    float  toolbarH   = 32.0f;
    float  toolbarW   = 200.0f;
    ImVec2 toolbarPos = {
        windowPos.x + (windowSize.x - toolbarW) * 0.5f,
        windowPos.y + 8.0f
    };

    ImGui::SetNextWindowPos(toolbarPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ toolbarW, toolbarH }, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.80f);

    ImGuiWindowFlags overlayFlags =
        ImGuiWindowFlags_NoDecoration      |
        ImGuiWindowFlags_NoInputs          |
        ImGuiWindowFlags_NoMove            |
        ImGuiWindowFlags_NoSavedSettings   |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoFocusOnAppearing|
        ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  { 6.0f, 4.0f });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.12f, 0.12f, 0.14f, 0.90f });

    if (ImGui::Begin("##ViewportToolbar", nullptr, overlayFlags))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,  { 3.0f, 0.0f });
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 8.0f, 4.0f });

        auto GizmoBtn = [&](const char* label, GizmoMode mode)
        {
            bool active = (m_GizmoMode == mode);
            ImGui::PushStyleColor(ImGuiCol_Button,
                active ? EditorTheme::Color::Accent
                       : EditorTheme::Color::BgPanel);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                EditorTheme::Color::AccentHover);
            // Remove NoInputs temporariamente para os botões clicáveis
            // Os botões ficam visualmente corretos mas input está na viewport
            if (ImGui::Button(label, { 54.0f, 22.0f }))
                m_GizmoMode = mode;
            ImGui::PopStyleColor(2);
            ImGui::SameLine();
        };

        ImGui::SetCursorPosY((toolbarH - 22.0f) * 0.5f);
        GizmoBtn("T", GizmoMode::Translate);
        GizmoBtn("R", GizmoMode::Rotate);
        GizmoBtn("S", GizmoMode::Scale);

        ImGui::PopStyleVar(2);
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    // Overlay de info no canto inferior esquerdo
    ImVec2 infoPos = { windowPos.x + 8.0f, windowPos.y + windowSize.y - 24.0f };
    ImGui::SetNextWindowPos(infoPos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowSize({ 300.0f, 20.0f }, ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
    if (ImGui::Begin("##ViewportInfo", nullptr, overlayFlags))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("%.0fx%.0f  |  %.1f FPS  |  W E R",
            windowSize.x, windowSize.y,
            ImGui::GetIO().Framerate);
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}