#pragma once

#include <glm/glm.hpp>
#include <array>

struct GLFWwindow;

// Aliases legíveis para os key codes do GLFW
// (você pode mapear para os seus próprios enums futuramente)
using KeyCode    = int;
using MouseButton = int;

class InputManager
{
public:
    InputManager()  = default;
    ~InputManager() = default;

    void Init(GLFWwindow* window);

    // Chamado uma vez por frame antes do Update
    void Poll();

    // ── Teclado ─────────────────────────────────────────────────────────────
    bool IsKeyDown    (KeyCode key) const;  // mantido pressionado
    bool IsKeyPressed (KeyCode key) const;  // apenas no frame em que foi apertado
    bool IsKeyReleased(KeyCode key) const;  // apenas no frame em que foi solto

    // ── Mouse – botões ───────────────────────────────────────────────────────
    bool IsMouseButtonDown    (MouseButton btn) const;
    bool IsMouseButtonPressed (MouseButton btn) const;
    bool IsMouseButtonReleased(MouseButton btn) const;

    // ── Mouse – posição / delta ───────────────────────────────────────────────
    glm::vec2 GetMousePosition() const { return m_MousePos;   }
    glm::vec2 GetMouseDelta()    const { return m_MouseDelta; }
    float     GetScrollDelta()   const { return m_ScrollDelta; }

private:
    // Callbacks estáticos GLFW
    static void KeyCallback       (GLFWwindow*, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow*, int button, int action, int mods);
    static void CursorPosCallback  (GLFWwindow*, double xpos, double ypos);
    static void ScrollCallback     (GLFWwindow*, double xoffset, double yoffset);

    // Ponteiro estático para instância (padrão callback GLFW)
    static InputManager* s_Instance;

private:
    GLFWwindow* m_Window = nullptr;

    // Teclado: estado atual e estado no frame anterior
    static constexpr int MAX_KEYS = 512;
    std::array<bool, MAX_KEYS> m_KeysCurrent  {};
    std::array<bool, MAX_KEYS> m_KeysPrevious {};

    // Mouse: botões
    static constexpr int MAX_BUTTONS = 8;
    std::array<bool, MAX_BUTTONS> m_ButtonsCurrent  {};
    std::array<bool, MAX_BUTTONS> m_ButtonsPrevious {};

    // Mouse: posição e scroll
    glm::vec2 m_MousePos      { 0.0f };
    glm::vec2 m_MousePosLast  { 0.0f };
    glm::vec2 m_MouseDelta    { 0.0f };
    float     m_ScrollDelta   = 0.0f;
    float     m_ScrollAccum   = 0.0f;  // acumula callbacks no mesmo frame
};