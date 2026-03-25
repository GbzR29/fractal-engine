#pragma once
#include <imgui.h>
#include <string>
#include <vector>

// IDs dos dock spaces
struct DockIDs
{
    ImGuiID Root        = 0;
    ImGuiID Left        = 0;
    ImGuiID Right       = 0;
    ImGuiID Center      = 0;
    ImGuiID Bottom      = 0;
    ImGuiID BottomLeft  = 0;
    ImGuiID BottomRight = 0;
};

// Nível de log do console
enum class LogLevel { Info, Warning, Error, Success };

struct LogEntry
{
    LogLevel    Level;
    std::string Message;
};