/**
 * @file GameViewport.hpp
 * @brief Runtime game preview viewport — shows what the primary camera sees in Play mode.
 *
 * The Game viewport renders the scene from the @ref CameraComponent marked as primary
 * into its own off-screen FBO, then presents the result as an ImGui image.
 */
#pragma once
#include "SceneFramebuffer.hpp"
#include <imgui.h>
#include <functional>

/// @brief "Game" panel that previews the scene through the primary camera.
class GameViewport
{
public:
    /**
     * @brief Allocates the FBO.
     * @param width   Initial framebuffer width in pixels.
     * @param height  Initial framebuffer height in pixels.
     */
    void Init(int width, int height);

    void BindFramebuffer();   ///< Binds the off-screen FBO.
    void UnbindFramebuffer(); ///< Restores the default FBO.

    /**
     * @brief Opens the "Game" ImGui window and presents the FBO as a texture.
     * @param isPlaying  Whether the scene is currently in Play mode.
     * @param isPaused   Whether the scene is currently paused.
     * @param onPlay     Callback for the Play/Stop toggle button.
     * @param onPause    Callback for the Pause toggle button.
     * @param onStop     Callback for the Step (single-frame advance) button.
     */
    void Begin(bool isPlaying, bool isPaused,
               std::function<void()> onPlay  = nullptr,
               std::function<void()> onPause = nullptr,
               std::function<void()> onStop  = nullptr);

    void End(); ///< Closes the "Game" ImGui window.

    SceneFramebuffer& GetFramebuffer() { return m_FBO;     }   ///< @return The off-screen FBO.
    bool              IsHovered()      const { return m_Hovered; } ///< @return @c true if the mouse is over this viewport.
    ImVec2            GetSize()        const { return m_Size;    } ///< @return Current viewport size in pixels.

private:
    SceneFramebuffer m_FBO;
    ImVec2           m_Size     = { 1280, 720 };
    bool             m_Hovered  = false;
    int              m_PendingW = 0;
    int              m_PendingH = 0;
};