#include "EditorViewport.hpp"
#include "EditorTheme.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>

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
    // Aplica resize pendente ANTES de renderizar — evita frame preto no resize
    if (m_PendingW > 0 && m_PendingH > 0)
    {
        m_FBO.Resize(m_PendingW, m_PendingH);
        m_Camera.SetViewportSize((float)m_PendingW, (float)m_PendingH);
        m_PendingW = 0;
        m_PendingH = 0;
    }
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
//  Begin — janela "Scene" com toolbar integrada
// ─────────────────────────────────────────────────────────────────────────────
void EditorViewport::Begin(float deltaTime)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.09f, 0.09f, 0.10f, 1.0f });
    ImGui::Begin("Scene");
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    m_WinPos  = ImGui::GetWindowPos();
    m_Focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

    // ── Toolbar integrada ─────────────────────────────────────────────────────
    const float tbH = 30.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5.0f, 4.0f });
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   { 3.0f, 0.0f });
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4{ 0.12f, 0.12f, 0.14f, 1.0f });
    ImGui::BeginChild("##SVBar", { 0.0f, tbH }, false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        const float btnH = 22.0f;
        const float btnW = 56.0f;

        auto ToolBtn = [&](const char* lbl, GizmoMode mode)
        {
            bool active = (m_GizmoMode == mode);
            ImGui::PushStyleColor(ImGuiCol_Button,
                active ? EditorTheme::Color::Accent
                       : ImVec4{ 0.18f, 0.18f, 0.21f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                active ? EditorTheme::Color::AccentHover
                       : ImVec4{ 0.24f, 0.24f, 0.28f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_Text,
                active ? EditorTheme::Color::TextBright
                       : EditorTheme::Color::TextDim);
            if (ImGui::Button(lbl, { btnW, btnH }))
                m_GizmoMode = mode;
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                if (mode == GizmoMode::Translate) ImGui::TextUnformatted("Translate  [W]");
                else if (mode == GizmoMode::Rotate) ImGui::TextUnformatted("Rotate  [E]");
                else ImGui::TextUnformatted("Scale  [R]");
                ImGui::EndTooltip();
            }
            ImGui::PopStyleColor(3);
            ImGui::SameLine(0, 3.0f);
        };

        ToolBtn("Move",   GizmoMode::Translate);
        ToolBtn("Rotate", GizmoMode::Rotate);
        ToolBtn("Scale",  GizmoMode::Scale);

        // Info no lado direito
        char info[48];
        snprintf(info, sizeof(info), "%.0f x %.0f  |  Ctrl: snap",
            m_Size.x, m_Size.y);
        float tw = ImGui::CalcTextSize(info).x + 8.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - tw);
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::TextUnformatted(info);
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    // ── Imagem do FBO ─────────────────────────────────────────────────────────
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.x < 4.0f) avail.x = 4.0f;
    if (avail.y < 4.0f) avail.y = 4.0f;

    // Registra resize pendente — será aplicado no próximo BindFramebuffer()
    if ((int)avail.x != m_FBO.GetWidth() || (int)avail.y != m_FBO.GetHeight())
    {
        m_PendingW = (int)avail.x;
        m_PendingH = (int)avail.y;
        m_Size     = avail;
    }

    ImVec2 imageOrigin = ImGui::GetCursorScreenPos();
    ImGui::Image(
        (ImTextureID)(intptr_t)m_FBO.GetColorAttachment(),
        avail, { 0, 1 }, { 1, 0 }
    );
    m_Hovered = ImGui::IsItemHovered();

    // Drag & drop — captura caminho e deixa o Editor instanciar
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("ASSET_PATH"))
            m_PendingDropPath = std::string(static_cast<const char*>(p->Data));
        ImGui::EndDragDropTarget();
    }

    // Câmera
    if (m_Hovered || m_Focused)
        m_Camera.OnUpdate(deltaTime);

    // ImGuizmo — rect alinhado ao image area (abaixo da toolbar)
#ifdef FE_HAS_IMGUIZMO
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(imageOrigin.x, imageOrigin.y, avail.x, avail.y);

    // Cubo de orientação — canto superior direito da área de imagem
    const float vmSize = 88.0f;
    glm::mat4 viewForVM = m_Camera.GetViewMatrix();
    ImGuizmo::ViewManipulate(
        glm::value_ptr(viewForVM),
        m_Camera.GetDistance(),
        ImVec2(imageOrigin.x + avail.x - vmSize - 4.0f, imageOrigin.y + 4.0f),
        ImVec2(vmSize, vmSize),
        0x10101088
    );
    if (viewForVM != m_Camera.GetViewMatrix())
        m_Camera.SetFromViewMatrix(viewForVM);
#endif
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

    // Snap com Ctrl
    const bool snapOn  = ImGui::GetIO().KeyCtrl;
    float snapT[3] = { 0.5f,  0.5f,  0.5f  };
    float snapR[3] = { 15.0f, 15.0f, 15.0f };
    float snapS[3] = { 0.25f, 0.25f, 0.25f };
    float* snap = nullptr;
    if (snapOn) {
        if (op == ImGuizmo::TRANSLATE) snap = snapT;
        else if (op == ImGuizmo::ROTATE) snap = snapR;
        else snap = snapS;
    }

    ImGuizmo::Manipulate(
        glm::value_ptr(m_Camera.GetViewMatrix()),
        glm::value_ptr(m_Camera.GetProjectionMatrix()),
        op, ImGuizmo::LOCAL,
        glm::value_ptr(transform),
        nullptr, snap
    );
#else
    (void)transform;
#endif
}
