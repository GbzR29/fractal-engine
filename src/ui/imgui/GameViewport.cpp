#include "GameViewport.hpp"
#include "EditorTheme.hpp"
#include <imgui.h>
#include <glad/glad.h>
#include <cstdio>

void GameViewport::Init(int width, int height)
{
    FramebufferSpec spec{ width, height };
    m_FBO.Init(spec);
}

void GameViewport::BindFramebuffer()
{
    m_FBO.Bind();
    glClearColor(0.05f, 0.05f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GameViewport::UnbindFramebuffer()
{
    m_FBO.Unbind();
}

void GameViewport::Begin(bool isPlaying, bool isPaused,
                         std::function<void()> onPlay,
                         std::function<void()> onPause,
                         std::function<void()> onStop)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.05f, 0.05f, 0.06f, 1.0f });
    ImGui::Begin("Game");
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    m_Hovered = ImGui::IsWindowHovered();

    // Resize
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.x < 4.0f) avail.x = 4.0f;
    if (avail.y < 4.0f) avail.y = 4.0f;

    if ((int)avail.x != m_FBO.GetWidth() || (int)avail.y != m_FBO.GetHeight())
    {
        m_FBO.Resize((int)avail.x, (int)avail.y);
        m_Size = avail;
    }

    // Imagem do FBO
    ImGui::Image(
        (ImTextureID)(intptr_t)m_FBO.GetColorAttachment(),
        avail, { 0, 1 }, { 1, 0 }
    );

    ImDrawList* dl   = ImGui::GetWindowDrawList();
    ImVec2      wPos = ImGui::GetWindowPos();
    ImVec2      wSz  = ImGui::GetWindowSize();

    // ── Borda colorida quando playing ─────────────────────────────────────────
    if (isPlaying)
    {
        ImU32 borderCol = isPaused
            ? IM_COL32(200, 150, 0,   200)
            : IM_COL32(40,  130, 240, 200);
        dl->AddRect(wPos, { wPos.x + wSz.x, wPos.y + wSz.y },
                    borderCol, 0.0f, 0, 2.5f);
    }

    // ── Botões overlay centralizados no topo ──────────────────────────────────
    const float btnW  = 60.0f;
    const float btnH  = 26.0f;
    const float gap   = 4.0f;
    const float barW  = btnW * 3.0f + gap * 2.0f;
    const float barH  = btnH + 10.0f;
    const float barX  = wPos.x + (wSz.x - barW) * 0.5f;
    const float barY  = wPos.y + 8.0f;

    ImGui::SetNextWindowPos({ barX, barY }, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ barW, barH });
    ImGui::SetNextWindowBgAlpha(0.85f);

    ImGuiWindowFlags overlayFlags =
        ImGuiWindowFlags_NoDecoration    |
        ImGuiWindowFlags_NoMove          |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoNav           |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  { 5.0f, 5.0f });
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,    { gap, 0.0f });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.10f, 0.10f, 0.12f, 0.92f });

    if (ImGui::Begin("##GameControls", nullptr, overlayFlags))
    {
        // ── Play / Stop ───────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Button,
            isPlaying ? ImVec4{ 0.70f, 0.15f, 0.15f, 1.0f }
                      : EditorTheme::Color::Accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            isPlaying ? ImVec4{ 0.90f, 0.25f, 0.25f, 1.0f }
                      : EditorTheme::Color::AccentHover);

        if (ImGui::Button(isPlaying ? "Stop" : "Play", { btnW, btnH }))
            if (onPlay) onPlay();

        ImGui::PopStyleColor(2);
        ImGui::SameLine();

        // ── Pause ─────────────────────────────────────────────────────────────
        ImGui::BeginDisabled(!isPlaying);
        ImGui::PushStyleColor(ImGuiCol_Button,
            isPaused ? EditorTheme::Color::Warning
                     : EditorTheme::Color::BgPanel);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);

        if (ImGui::Button(isPaused ? "Resume" : "Pause", { btnW, btnH }))
            if (onPause) onPause();

        ImGui::PopStyleColor(2);
        ImGui::EndDisabled();
        ImGui::SameLine();

        // ── Step ──────────────────────────────────────────────────────────────
        ImGui::BeginDisabled(!isPlaying || !isPaused);
        ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);

        if (ImGui::Button("Step", { btnW, btnH }))
            if (onStop) onStop();   // reutilizado como Step por enquanto

        ImGui::PopStyleColor(2);
        ImGui::EndDisabled();
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);

    // ── Mensagem quando parado ────────────────────────────────────────────────
    if (!isPlaying)
    {
        const char* hint = "Pressione Play para iniciar";
        ImVec2 tsz = ImGui::CalcTextSize(hint);
        dl->AddText(
            { wPos.x + (wSz.x - tsz.x) * 0.5f,
              wPos.y + (wSz.y - tsz.y) * 0.5f },
            IM_COL32(70, 70, 85, 220), hint
        );
    }

    // ── Resolução no canto inferior direito ───────────────────────────────────
    char resBuf[32];
    snprintf(resBuf, sizeof(resBuf), "%.0f x %.0f", avail.x, avail.y);
    ImVec2 rsz = ImGui::CalcTextSize(resBuf);
    dl->AddText(
        { wPos.x + wSz.x - rsz.x - 8.0f, wPos.y + wSz.y - rsz.y - 6.0f },
        IM_COL32(60, 60, 75, 180), resBuf
    );
}

void GameViewport::End()
{
    ImGui::End();
}