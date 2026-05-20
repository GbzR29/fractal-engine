/**
 * @file EditorTheme.hpp
 * @brief Dark editor colour palette and ImGui style overrides.
 *
 * All colours are defined as @c constexpr @c ImVec4 constants in the
 * @ref EditorTheme::Color namespace.  Call @ref EditorTheme::Apply() once after
 * @c ImGui::CreateContext() to apply rounding, spacing, and colour overrides.
 */
#pragma once
#include <imgui.h>

/// @brief Centralised visual style for the fractal-engine editor.
namespace EditorTheme
{
    /// @brief Colour constants used throughout all editor panels.
    namespace Color
    {
        // Backgrounds — hierarquia clara de profundidade
        constexpr ImVec4 BgDeep       = { 0.06f, 0.06f, 0.07f, 1.00f };
        constexpr ImVec4 BgBase       = { 0.10f, 0.10f, 0.11f, 1.00f };
        constexpr ImVec4 BgPanel      = { 0.15f, 0.15f, 0.17f, 1.00f };
        constexpr ImVec4 BgHover      = { 0.21f, 0.21f, 0.24f, 1.00f };
        constexpr ImVec4 BgActive     = { 0.26f, 0.26f, 0.29f, 1.00f };
        constexpr ImVec4 BgInput      = { 0.08f, 0.08f, 0.09f, 1.00f };

        // Accent — azul vivo
        constexpr ImVec4 Accent       = { 0.22f, 0.50f, 0.95f, 1.00f };
        constexpr ImVec4 AccentHover  = { 0.32f, 0.60f, 1.00f, 1.00f };
        constexpr ImVec4 AccentActive = { 0.16f, 0.40f, 0.85f, 1.00f };
        constexpr ImVec4 AccentDim    = { 0.22f, 0.50f, 0.95f, 0.18f };

        // Texto
        constexpr ImVec4 TextBright   = { 0.95f, 0.95f, 0.97f, 1.00f };
        constexpr ImVec4 TextNormal   = { 0.84f, 0.84f, 0.87f, 1.00f };
        constexpr ImVec4 TextDim      = { 0.48f, 0.48f, 0.52f, 1.00f };
        constexpr ImVec4 TextDisabled = { 0.33f, 0.33f, 0.36f, 1.00f };

        // Bordas
        constexpr ImVec4 Border       = { 0.17f, 0.17f, 0.20f, 1.00f };
        constexpr ImVec4 BorderBright = { 0.27f, 0.27f, 0.30f, 1.00f };
        constexpr ImVec4 BorderAccent = { 0.22f, 0.50f, 0.95f, 0.55f };

        // Status
        constexpr ImVec4 Success      = { 0.25f, 0.82f, 0.48f, 1.00f };
        constexpr ImVec4 Warning      = { 0.97f, 0.76f, 0.23f, 1.00f };
        constexpr ImVec4 Error        = { 0.95f, 0.34f, 0.34f, 1.00f };
        constexpr ImVec4 Info         = { 0.30f, 0.72f, 0.96f, 1.00f };

        // Transparente
        constexpr ImVec4 None         = { 0.00f, 0.00f, 0.00f, 0.00f };
    }

    /// @brief Applies all colour overrides and style variables to the current ImGui context.
    ///        Call once after @c ImGui::CreateContext() and before the first frame.
    inline void Apply()
    {
        ImGuiStyle& s = ImGui::GetStyle();

        // Arredondamento — moderno
        s.WindowRounding    = 6.0f;
        s.ChildRounding     = 4.0f;
        s.FrameRounding     = 4.0f;
        s.PopupRounding     = 6.0f;
        s.ScrollbarRounding = 6.0f;
        s.GrabRounding      = 4.0f;
        s.TabRounding       = 5.0f;

        // Bordas
        s.WindowBorderSize  = 1.0f;
        s.ChildBorderSize   = 0.0f;
        s.FrameBorderSize   = 0.0f;
        s.PopupBorderSize   = 1.0f;
        s.TabBorderSize     = 0.0f;

        // Espaçamento
        s.WindowPadding     = { 12.0f,  9.0f };
        s.FramePadding      = {  7.0f,  4.0f };
        s.ItemSpacing       = {  7.0f,  5.0f };
        s.ItemInnerSpacing  = {  5.0f,  4.0f };
        s.IndentSpacing     = 18.0f;
        s.ScrollbarSize     = 10.0f;
        s.GrabMinSize       = 12.0f;
        s.SeparatorTextBorderSize = 1.0f;

        // ── Cores ────────────────────────────────────────────────────────────
        ImVec4* c = s.Colors;

        c[ImGuiCol_Text]                 = Color::TextNormal;
        c[ImGuiCol_TextDisabled]         = Color::TextDisabled;

        c[ImGuiCol_WindowBg]             = Color::BgBase;
        c[ImGuiCol_ChildBg]              = Color::BgDeep;
        c[ImGuiCol_PopupBg]              = { 0.12f, 0.12f, 0.14f, 0.98f };

        c[ImGuiCol_Border]               = Color::Border;
        c[ImGuiCol_BorderShadow]         = Color::None;

        c[ImGuiCol_FrameBg]              = Color::BgInput;
        c[ImGuiCol_FrameBgHovered]       = Color::BgHover;
        c[ImGuiCol_FrameBgActive]        = Color::BgActive;

        c[ImGuiCol_TitleBg]              = Color::BgDeep;
        c[ImGuiCol_TitleBgActive]        = { 0.09f, 0.09f, 0.11f, 1.0f };
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

        c[ImGuiCol_Tab]                  = { 0.08f, 0.08f, 0.09f, 1.0f };
        c[ImGuiCol_TabHovered]           = Color::BgHover;
        c[ImGuiCol_TabActive]            = { 0.14f, 0.14f, 0.16f, 1.0f };
        c[ImGuiCol_TabUnfocused]         = Color::BgDeep;
        c[ImGuiCol_TabUnfocusedActive]   = { 0.11f, 0.11f, 0.13f, 1.0f };

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
        c[ImGuiCol_NavWindowingDimBg]    = { 0.0f, 0.0f, 0.0f, 0.40f };
        c[ImGuiCol_ModalWindowDimBg]     = { 0.0f, 0.0f, 0.0f, 0.55f };
    }
}
