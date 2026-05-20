/**
 * @file InputManager.hpp
 * @brief Keyboard and mouse state manager built on GLFW callbacks.
 *
 * Call @ref Poll() once per frame (before any game logic) to snapshot the current
 * input state.  Use @ref IsKeyPressed() for one-shot actions and @ref IsKeyDown()
 * for held keys.  GLFW key codes (e.g. @c GLFW_KEY_W) are used directly as
 * @ref KeyCode values.
 */
#pragma once

#include <glm/glm.hpp>
#include <array>

struct GLFWwindow;

using KeyCode     = int; ///< GLFW key code (e.g. @c GLFW_KEY_SPACE).
using MouseButton = int; ///< GLFW mouse button index (e.g. @c GLFW_MOUSE_BUTTON_LEFT).

/// @brief Polls keyboard, mouse buttons, cursor position, and scroll wheel each frame.
class InputManager
{
public:
    InputManager()  = default;
    ~InputManager() = default;

    /**
     * @brief Registers GLFW callbacks and stores the window pointer.
     * @param window  The GLFW window to read input from.
     */
    void Init(GLFWwindow* window);

    /// Snapshots the current GLFW input state; call once per frame before @c Update().
    void Poll();

    // ── Keyboard ──────────────────────────────────────────────────────────────

    bool IsKeyDown    (KeyCode key) const; ///< @return @c true while the key is held.
    bool IsKeyPressed (KeyCode key) const; ///< @return @c true only on the frame the key was pressed.
    bool IsKeyReleased(KeyCode key) const; ///< @return @c true only on the frame the key was released.

    // ── Mouse buttons ─────────────────────────────────────────────────────────

    bool IsMouseButtonDown    (MouseButton btn) const; ///< @return @c true while the button is held.
    bool IsMouseButtonPressed (MouseButton btn) const; ///< @return @c true only on the frame the button was pressed.
    bool IsMouseButtonReleased(MouseButton btn) const; ///< @return @c true only on the frame the button was released.

    // ── Mouse position / delta ────────────────────────────────────────────────

    glm::vec2 GetMousePosition() const { return m_MousePos;    } ///< @return Cursor position in screen pixels.
    glm::vec2 GetMouseDelta()    const { return m_MouseDelta;  } ///< @return Cursor movement since the last frame.
    float     GetScrollDelta()   const { return m_ScrollDelta; } ///< @return Vertical scroll amount this frame.

private:
    static void KeyCallback        (GLFWwindow*, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow*, int button, int action, int mods);
    static void CursorPosCallback  (GLFWwindow*, double xpos, double ypos);
    static void ScrollCallback     (GLFWwindow*, double xoffset, double yoffset);

    static InputManager* s_Instance; ///< Singleton pointer used by static GLFW callbacks.

private:
    GLFWwindow* m_Window = nullptr;

    static constexpr int MAX_KEYS    = 512;
    static constexpr int MAX_BUTTONS = 8;

    std::array<bool, MAX_KEYS>    m_KeysCurrent   {};
    std::array<bool, MAX_KEYS>    m_KeysPrevious  {};

    std::array<bool, MAX_BUTTONS> m_ButtonsCurrent  {};
    std::array<bool, MAX_BUTTONS> m_ButtonsPrevious {};

    glm::vec2 m_MousePos     { 0.0f };
    glm::vec2 m_MousePosLast { 0.0f };
    glm::vec2 m_MouseDelta   { 0.0f };
    float     m_ScrollDelta  = 0.0f;
    float     m_ScrollAccum  = 0.0f; ///< Accumulates multiple scroll callbacks within one frame.
};