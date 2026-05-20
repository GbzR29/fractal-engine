/**
 * @file EditorCamera.hpp
 * @brief Orbit / fly camera for the scene editor viewport.
 *
 * Controls:
 * - **Alt + LMB drag** — tumble (orbit) around the focal point.
 * - **MMB drag** — pan the focal point.
 * - **Scroll wheel** — dolly in/out.
 * - **RMB + WASD** — free-fly mode.
 */
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/// @brief Perspective orbit/fly camera used in the @ref EditorViewport.
class EditorCamera
{
public:
    EditorCamera() { RecalculateProjection(); RecalculateView(); }

    /**
     * @brief Processes input and updates the camera matrices.
     * @param deltaTime  Seconds since the last frame.
     */
    void OnUpdate(float deltaTime);

    /**
     * @brief Updates the aspect ratio when the viewport is resized.
     * @param width   Viewport width in pixels.
     * @param height  Viewport height in pixels.
     */
    void SetViewportSize(float width, float height);

    const glm::mat4& GetViewMatrix()       const { return m_View;                  } ///< @return Current view matrix.
    const glm::mat4& GetProjectionMatrix() const { return m_Projection;            } ///< @return Current projection matrix.
    glm::mat4        GetViewProjection()   const { return m_Projection * m_View;   } ///< @return Combined view-projection matrix.
    glm::vec3        GetPosition()         const { return m_Position;              } ///< @return Camera world-space position.

    float GetNearClip()  const { return m_Near;     } ///< @return Near clipping plane distance.
    float GetFarClip()   const { return m_Far;      } ///< @return Far clipping plane distance.
    float GetDistance()  const { return m_Distance; } ///< @return Distance from the focal point.

    /**
     * @brief Reconstructs internal yaw/pitch/distance from an external view matrix.
     *        Used to synchronise the camera after the ImGuizmo ViewManipulate widget.
     * @param view  The new view matrix to decompose.
     */
    void SetFromViewMatrix(const glm::mat4& view);

private:
    void RecalculateView();
    void RecalculateProjection();

    glm::vec3 GetForward() const;
    glm::vec3 GetRight()   const;
    glm::vec3 GetUp()      const;

    // Posição inicial: acima e atrás da origem, olhando levemente para baixo
    glm::vec3 m_FocalPoint  = { 0.0f, 0.0f, 0.0f };
    float     m_Distance    = 10.0f;
    float     m_Yaw         = -90.0f;   // olhando para -Z
    float     m_Pitch       = -30.0f;   // levemente para baixo

    float m_FOV         = 60.0f;
    float m_AspectRatio = 1280.0f / 720.0f;
    float m_Near        = 0.1f;
    float m_Far         = 1000.0f;
    float m_FlySpeed    = 8.0f;   // unidades/segundo no modo RMB+WASD

    glm::vec2 m_LastMousePos = { 0.0f, 0.0f };
    bool      m_FirstMouse   = true;

    glm::mat4 m_View       = glm::mat4(1.0f);
    glm::mat4 m_Projection = glm::mat4(1.0f);
    glm::vec3 m_Position   = { 0.0f, 0.0f, 0.0f };
};