/**
 * @file ViewportPanel.hpp
 * @brief Generic viewport panel base combining a @ref SceneFramebuffer, @ref EditorCamera,
 *        and ImGuizmo gizmo — intended as a reusable building block.
 *
 * @note @ref EditorViewport and @ref GameViewport are the specialised implementations currently
 *       used by the engine.  This class is kept for future multi-viewport support.
 */
#pragma once
#include "SceneFramebuffer.hpp"
#include "EditorCamera.hpp"
#include <imgui.h>
#include <glm/glm.hpp>

/// @brief Active gizmo operation — duplicated here for files that include only this header.
enum class GizmoMode { Translate, Rotate, Scale };

/// @brief Reusable viewport panel with FBO, orbit camera, toolbar, and gizmo support.
class ViewportPanel
{
public:
    ViewportPanel() = default;

    /**
     * @brief Allocates the FBO at the given initial size.
     * @param width   Initial width in pixels.
     * @param height  Initial height in pixels.
     */
    void Init(int width, int height);

    /**
     * @brief Opens the ImGui viewport window and updates the camera.
     * @param deltaTime  Seconds since the last frame.
     * @return @c true if the viewport was resized this frame (FBO was rebuilt).
     */
    bool Begin(float deltaTime);

    void End(); ///< Closes the ImGui viewport window.

    /**
     * @brief Renders the ImGuizmo transform gizmo.
     * @param transform  Reference to the selected entity's model matrix.
     */
    void DrawGizmo(glm::mat4& transform);

    SceneFramebuffer& GetFramebuffer()  { return m_Framebuffer;   }           ///< @return The FBO.
    EditorCamera&     GetCamera()       { return m_Camera;        }           ///< @return The orbit camera.
    bool              IsHovered()       const { return m_Hovered;     }       ///< @return Mouse-over state.
    bool              IsFocused()       const { return m_Focused;     }       ///< @return Keyboard focus state.
    ImVec2            GetViewportSize() const { return m_ViewportSize; }      ///< @return Current size in pixels.
    ImVec2            GetViewportPos()  const { return m_ViewportPos;  }      ///< @return Window-space top-left corner.
    GizmoMode         GetGizmoMode()    const { return m_GizmoMode;    }      ///< @return Active gizmo mode.

private:
    void DrawToolbar();

    SceneFramebuffer m_Framebuffer;
    EditorCamera     m_Camera;

    GizmoMode m_GizmoMode   = GizmoMode::Translate;
    ImVec2    m_ViewportSize = { 1280, 720 };
    ImVec2    m_ViewportPos  = { 0, 0 };
    bool      m_Hovered      = false;
    bool      m_Focused      = false;
};