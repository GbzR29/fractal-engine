/**
 * @file EditorViewport.hpp
 * @brief Scene editor viewport — off-screen FBO, orbit camera, infinite grid, and gizmos.
 *
 * Render loop:
 * 1. @ref BindFramebuffer() — redirect draw calls to the off-screen FBO.
 * 2. @ref DrawGrid()        — render the XZ grid (inside the FBO).
 * 3. *(Application renders scene entities here)*
 * 4. @ref UnbindFramebuffer() — restore the default FBO.
 * 5. @ref Begin()            — open the "Scene" ImGui window and draw the FBO as a texture.
 * 6. @ref DrawGizmo()        — overlay the ImGuizmo transform gizmo.
 * 7. @ref End()              — close the ImGui window.
 */
#pragma once
#include "SceneFramebuffer.hpp"
#include "EditorCamera.hpp"
#include "EditorGrid.hpp"
#include <imgui.h>
#include <glm/glm.hpp>
#include <string>

/// @brief Active transform gizmo operation mode.
enum class GizmoMode { Translate, Rotate, Scale };

/// @brief The "Scene" editor viewport panel with its own FBO, camera, grid, and gizmos.
class EditorViewport
{
public:
    /**
     * @brief Allocates the FBO and compiles the grid shader.
     * @param width   Initial framebuffer width in pixels.
     * @param height  Initial framebuffer height in pixels.
     * @return @c true on success.
     */
    bool Init(int width, int height);

    void BindFramebuffer();   ///< Binds the off-screen FBO. Call before rendering the scene.
    void UnbindFramebuffer(); ///< Restores the default FBO. Call after rendering the scene.

    /**
     * @brief Opens the "Scene" ImGui window and presents the FBO colour texture.
     * @param deltaTime  Seconds since the last frame (forwarded to the camera).
     */
    void Begin(float deltaTime);

    void End(); ///< Closes the "Scene" ImGui window.

    /**
     * @brief Renders the ImGuizmo transform gizmo over the viewport.
     * @param transform  Reference to the selected entity's model matrix (may be modified in-place).
     */
    void DrawGizmo(glm::mat4& transform);

    /// Draws the infinite XZ grid.  Call between @ref BindFramebuffer() and @ref UnbindFramebuffer().
    void DrawGrid();

    SceneFramebuffer& GetFramebuffer() { return m_FBO;    }    ///< @return The off-screen FBO.
    EditorCamera&     GetCamera()      { return m_Camera; }    ///< @return The orbit/fly camera.
    GizmoMode         GetGizmoMode()   const { return m_GizmoMode; } ///< @return Active gizmo operation.
    bool              IsHovered()      const { return m_Hovered;   } ///< @return @c true if the mouse is over the viewport.
    bool              IsFocused()      const { return m_Focused;   } ///< @return @c true if the viewport has keyboard focus.
    ImVec2            GetSize()        const { return m_Size;      } ///< @return Current viewport size in pixels.

    /// @return @c true if a drag-and-drop asset path is waiting to be consumed.
    bool        HasPendingDrop() const { return !m_PendingDropPath.empty(); }

    /// Consumes and returns the pending drop path.  Returns an empty string if none.
    std::string TakePendingDrop() { return std::move(m_PendingDropPath); }

    /// Sets the active gizmo operation mode.
    void SetGizmoMode(GizmoMode mode) { m_GizmoMode = mode; }

private:
    SceneFramebuffer m_FBO;
    EditorCamera     m_Camera;
    EditorGrid       m_Grid;

    GizmoMode   m_GizmoMode       = GizmoMode::Translate;
    ImVec2      m_Size            = { 1280, 720 };
    ImVec2      m_WinPos          = { 0, 0 };
    bool        m_Hovered         = false;
    bool        m_Focused         = false;
    bool        m_GridInit        = false;
    int         m_PendingW        = 0;
    int         m_PendingH        = 0;
    std::string m_PendingDropPath;
};