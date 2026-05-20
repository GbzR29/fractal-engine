/**
 * @file EditorLayout.hpp
 * @brief Dock-space IDs and console log types shared across all editor panels.
 */
#pragma once
#include <imgui.h>
#include <string>
#include <vector>

/// @brief Persistent ImGui dock-space IDs built by @ref Editor::BuildDockLayout().
struct DockIDs
{
    ImGuiID Root        = 0; ///< Full-window root dock space.
    ImGuiID Left        = 0; ///< Left column (Hierarchy).
    ImGuiID Right       = 0; ///< Right column (Inspector).
    ImGuiID Center      = 0; ///< Central area (Scene / Game viewports).
    ImGuiID Bottom      = 0; ///< Bottom strip.
    ImGuiID BottomLeft  = 0; ///< Bottom-left cell (Console).
    ImGuiID BottomRight = 0; ///< Bottom-right cell (Asset Browser).
};

/// @brief Severity level for editor console messages.
enum class LogLevel {
    Info,    ///< General informational message (white).
    Warning, ///< Non-fatal warning (yellow).
    Error,   ///< Error condition (red).
    Success  ///< Successful operation (green).
};

/// @brief A single console log entry.
struct LogEntry
{
    LogLevel    Level;   ///< Message severity.
    std::string Message; ///< Message text.
};