#pragma once
#include <imgui.h>

namespace EditorTheme
{
    // ── Paleta de cores ──────────────────────────────────────────────────────
    namespace Color
    {
        // Backgrounds
        constexpr ImVec4 BgDeep       = { 0.08f, 0.08f, 0.09f, 1.00f }; // mais escuro
        constexpr ImVec4 BgBase       = { 0.11f, 0.11f, 0.12f, 1.00f }; // painéis
        constexpr ImVec4 BgPanel      = { 0.13f, 0.13f, 0.14f, 1.00f }; // headers
        constexpr ImVec4 BgHover      = { 0.18f, 0.18f, 0.20f, 1.00f };
        constexpr ImVec4 BgActive     = { 0.22f, 0.22f, 0.24f, 1.00f };
        constexpr ImVec4 BgInput      = { 0.09f, 0.09f, 0.10f, 1.00f };

        // Accent — azul-elétrico
        constexpr ImVec4 Accent       = { 0.20f, 0.52f, 0.98f, 1.00f };
        constexpr ImVec4 AccentHover  = { 0.30f, 0.60f, 1.00f, 1.00f };
        constexpr ImVec4 AccentActive = { 0.15f, 0.42f, 0.88f, 1.00f };
        constexpr ImVec4 AccentDim    = { 0.20f, 0.52f, 0.98f, 0.25f };

        // Texto
        constexpr ImVec4 TextBright   = { 0.95f, 0.95f, 0.96f, 1.00f };
        constexpr ImVec4 TextNormal   = { 0.78f, 0.78f, 0.80f, 1.00f };
        constexpr ImVec4 TextDim      = { 0.45f, 0.45f, 0.48f, 1.00f };
        constexpr ImVec4 TextDisabled = { 0.32f, 0.32f, 0.34f, 1.00f };

        // Bordas
        constexpr ImVec4 Border       = { 0.22f, 0.22f, 0.24f, 1.00f };
        constexpr ImVec4 BorderBright = { 0.32f, 0.32f, 0.35f, 1.00f };
        constexpr ImVec4 BorderAccent = { 0.20f, 0.52f, 0.98f, 0.60f };

        // Status
        constexpr ImVec4 Success      = { 0.22f, 0.78f, 0.45f, 1.00f };
        constexpr ImVec4 Warning      = { 0.95f, 0.72f, 0.20f, 1.00f };
        constexpr ImVec4 Error        = { 0.92f, 0.32f, 0.32f, 1.00f };
        constexpr ImVec4 Info         = { 0.30f, 0.70f, 0.95f, 1.00f };

        // Transparente
        constexpr ImVec4 None         = { 0.00f, 0.00f, 0.00f, 0.00f };
    }

    // ── Aplicar tema ─────────────────────────────────────────────────────────
    inline void Apply()
    {
        ImGuiStyle& s = ImGui::GetStyle();

        // Arredondamento geral
        s.WindowRounding    = 4.0f;
        s.ChildRounding     = 4.0f;
        s.FrameRounding     = 3.0f;
        s.PopupRounding     = 4.0f;
        s.ScrollbarRounding = 4.0f;
        s.GrabRounding      = 3.0f;
        s.TabRounding       = 4.0f;

        // Espessura de borda
        s.WindowBorderSize  = 1.0f;
        s.ChildBorderSize   = 1.0f;
        s.FrameBorderSize   = 0.0f;
        s.PopupBorderSize   = 1.0f;
        s.TabBorderSize     = 0.0f;

        // Espaçamento
        s.WindowPadding     = { 10.0f,  8.0f };
        s.FramePadding      = {  6.0f,  4.0f };
        s.ItemSpacing       = {  6.0f,  5.0f };
        s.ItemInnerSpacing  = {  5.0f,  4.0f };
        s.IndentSpacing     = 16.0f;
        s.ScrollbarSize     = 11.0f;
        s.GrabMinSize       = 10.0f;


        // Separadores
        s.SeparatorTextBorderSize = 1.0f;

        // ── Cores ────────────────────────────────────────────────────────────
        ImVec4* c = s.Colors;

        c[ImGuiCol_WindowBg]             = Color::BgBase;
        c[ImGuiCol_ChildBg]              = Color::BgDeep;
        c[ImGuiCol_PopupBg]              = Color::BgPanel;

        c[ImGuiCol_Border]               = Color::Border;
        c[ImGuiCol_BorderShadow]         = Color::None;

        c[ImGuiCol_FrameBg]              = Color::BgInput;
        c[ImGuiCol_FrameBgHovered]       = Color::BgHover;
        c[ImGuiCol_FrameBgActive]        = Color::BgActive;

        c[ImGuiCol_TitleBg]              = Color::BgDeep;
        c[ImGuiCol_TitleBgActive]        = Color::BgDeep;
        c[ImGuiCol_TitleBgCollapsed]     = Color::BgDeep;

        c[ImGuiCol_MenuBarBg]            = Color::BgDeep;

        c[ImGuiCol_ScrollbarBg]          = Color::BgDeep;
        c[ImGuiCol_ScrollbarGrab]        = Color::BgActive;
        c[ImGuiCol_ScrollbarGrabHovered] = Color::BgHover;
        c[ImGuiCol_ScrollbarGrabActive]  = Color::Accent;

        c[ImGuiCol_CheckMark]            = Color::Accent;
        c[ImGuiCol_SliderGrab]           = Color::Accent;
        c[ImGuiCol_SliderGrabActive]     = Color::AccentActive;

        c[ImGuiCol_Button]               = Color::BgPanel;
        c[ImGuiCol_ButtonHovered]        = Color::BgHover;
        c[ImGuiCol_ButtonActive]         = Color::AccentActive;

        c[ImGuiCol_Header]               = Color::AccentDim;
        c[ImGuiCol_HeaderHovered]        = Color::BgHover;
        c[ImGuiCol_HeaderActive]         = Color::Accent;

        c[ImGuiCol_Separator]            = Color::Border;
        c[ImGuiCol_SeparatorHovered]     = Color::Accent;
        c[ImGuiCol_SeparatorActive]      = Color::AccentHover;

        c[ImGuiCol_ResizeGrip]           = Color::None;
        c[ImGuiCol_ResizeGripHovered]    = Color::Accent;
        c[ImGuiCol_ResizeGripActive]     = Color::AccentHover;

        c[ImGuiCol_Tab]                  = Color::BgDeep;
        c[ImGuiCol_TabHovered]           = Color::BgHover;
        c[ImGuiCol_TabActive]            = Color::BgPanel;
        c[ImGuiCol_TabUnfocused]         = Color::BgDeep;
        c[ImGuiCol_TabUnfocusedActive]   = Color::BgBase;

#ifdef IMGUI_HAS_DOCK
        c[ImGuiCol_DockingPreview]       = Color::AccentDim;
        c[ImGuiCol_DockingEmptyBg]       = Color::BgDeep;
#endif

        c[ImGuiCol_PlotLines]            = Color::Accent;
        c[ImGuiCol_PlotLinesHovered]     = Color::AccentHover;
        c[ImGuiCol_PlotHistogram]        = Color::Accent;
        c[ImGuiCol_PlotHistogramHovered] = Color::AccentHover;

        c[ImGuiCol_TableHeaderBg]        = Color::BgPanel;
        c[ImGuiCol_TableBorderStrong]    = Color::Border;
        c[ImGuiCol_TableBorderLight]     = Color::BgHover;
        c[ImGuiCol_TableRowBg]           = Color::None;
        c[ImGuiCol_TableRowBgAlt]        = { 1.0f, 1.0f, 1.0f, 0.02f };

        c[ImGuiCol_TextSelectedBg]       = Color::AccentDim;
        c[ImGuiCol_DragDropTarget]       = Color::Accent;
        c[ImGuiCol_NavHighlight]         = Color::Accent;
        c[ImGuiCol_NavWindowingHighlight]= Color::Accent;
        c[ImGuiCol_NavWindowingDimBg]    = { 0.0f, 0.0f, 0.0f, 0.4f };
        c[ImGuiCol_ModalWindowDimBg]     = { 0.0f, 0.0f, 0.0f, 0.5f };

        c[ImGuiCol_Text]                 = Color::TextNormal;
        c[ImGuiCol_TextDisabled]         = Color::TextDisabled;
    }
}