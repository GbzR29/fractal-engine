#pragma once
#include "SceneFramebuffer.hpp"
#include <imgui.h>
#include <functional>

class GameViewport
{
public:
    void Init(int width, int height);

    void BindFramebuffer();
    void UnbindFramebuffer();

    // Callbacks: onPlay toggle play/stop, onPause toggle pause, onStop = step
    void Begin(bool isPlaying, bool isPaused,
               std::function<void()> onPlay  = nullptr,
               std::function<void()> onPause = nullptr,
               std::function<void()> onStop  = nullptr);
    void End();

    SceneFramebuffer& GetFramebuffer() { return m_FBO;     }
    bool              IsHovered()      const { return m_Hovered; }
    ImVec2            GetSize()        const { return m_Size;    }

private:
    SceneFramebuffer m_FBO;
    ImVec2           m_Size    = { 1280, 720 };
    bool             m_Hovered = false;
};