#include "InputManager.hpp"

#include <GLFW/glfw3.h>
#include <iostream>

InputManager* InputManager::s_Instance = nullptr;

void InputManager::Init(GLFWwindow* window)
{
    m_Window    = window;
    s_Instance  = this;

    glfwSetKeyCallback        (window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback  (window, CursorPosCallback);
    glfwSetScrollCallback     (window, ScrollCallback);

    // Posição inicial do cursor
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    m_MousePos     = { static_cast<float>(x), static_cast<float>(y) };
    m_MousePosLast = m_MousePos;
}

void InputManager::Poll()
{
    // Salva estado anterior
    m_KeysPrevious     = m_KeysCurrent;
    m_ButtonsPrevious  = m_ButtonsCurrent;

    // Calcula delta do mouse
    m_MouseDelta  = m_MousePos - m_MousePosLast;
    m_MousePosLast = m_MousePos;

    // Transfere scroll acumulado e zera
    m_ScrollDelta = m_ScrollAccum;
    m_ScrollAccum = 0.0f;

    glfwPollEvents();
}

// ── Teclado ───────────────────────────────────────────────────────────────────

bool InputManager::IsKeyDown(KeyCode key) const
{
    if (key < 0 || key >= MAX_KEYS) return false;
    return m_KeysCurrent[key];
}

bool InputManager::IsKeyPressed(KeyCode key) const
{
    if (key < 0 || key >= MAX_KEYS) return false;
    return m_KeysCurrent[key] && !m_KeysPrevious[key];
}

bool InputManager::IsKeyReleased(KeyCode key) const
{
    if (key < 0 || key >= MAX_KEYS) return false;
    return !m_KeysCurrent[key] && m_KeysPrevious[key];
}

// ── Mouse – botões ────────────────────────────────────────────────────────────

bool InputManager::IsMouseButtonDown(MouseButton btn) const
{
    if (btn < 0 || btn >= MAX_BUTTONS) return false;
    return m_ButtonsCurrent[btn];
}

bool InputManager::IsMouseButtonPressed(MouseButton btn) const
{
    if (btn < 0 || btn >= MAX_BUTTONS) return false;
    return m_ButtonsCurrent[btn] && !m_ButtonsPrevious[btn];
}

bool InputManager::IsMouseButtonReleased(MouseButton btn) const
{
    if (btn < 0 || btn >= MAX_BUTTONS) return false;
    return !m_ButtonsCurrent[btn] && m_ButtonsPrevious[btn];
}

// ── Callbacks ─────────────────────────────────────────────────────────────────

void InputManager::KeyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (!s_Instance) return;
    if (key < 0 || key >= MAX_KEYS) return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
        s_Instance->m_KeysCurrent[key] = true;
    else if (action == GLFW_RELEASE)
        s_Instance->m_KeysCurrent[key] = false;
}

void InputManager::MouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/)
{
    if (!s_Instance) return;
    if (button < 0 || button >= MAX_BUTTONS) return;

    s_Instance->m_ButtonsCurrent[button] = (action == GLFW_PRESS);
}

void InputManager::CursorPosCallback(GLFWwindow* /*window*/, double xpos, double ypos)
{
    if (!s_Instance) return;
    s_Instance->m_MousePos = { static_cast<float>(xpos), static_cast<float>(ypos) };
}

void InputManager::ScrollCallback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset)
{
    if (!s_Instance) return;
    s_Instance->m_ScrollAccum += static_cast<float>(yoffset);
}