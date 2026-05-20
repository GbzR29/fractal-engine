#include "EditorCamera.hpp"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

static float ToRad(float deg) { return glm::radians(deg); }

glm::vec3 EditorCamera::GetForward() const
{
    return glm::normalize(glm::vec3{
        cos(ToRad(m_Yaw)) * cos(ToRad(m_Pitch)),
        sin(ToRad(m_Pitch)),
        sin(ToRad(m_Yaw)) * cos(ToRad(m_Pitch))
    });
}

glm::vec3 EditorCamera::GetRight() const
{
    return glm::normalize(glm::cross(GetForward(), glm::vec3{ 0, 1, 0 }));
}

glm::vec3 EditorCamera::GetUp() const
{
    return glm::normalize(glm::cross(GetRight(), GetForward()));
}

void EditorCamera::OnUpdate(float deltaTime)
{
    if (!ImGui::IsWindowHovered()) return;

    ImGuiIO& io = ImGui::GetIO();
    glm::vec2 mousePos  = { io.MousePos.x, io.MousePos.y };

    if (m_FirstMouse)
    {
        m_LastMousePos = mousePos;
        m_FirstMouse   = false;
    }

    glm::vec2 delta = (mousePos - m_LastMousePos) * 0.003f;
    m_LastMousePos  = mousePos;

    // ── Botão do meio — pan ───────────────────────────────────────────────────
    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
    {
        float speed = m_Distance * 0.5f;
        m_FocalPoint -= GetRight() * delta.x * speed;
        m_FocalPoint += GetUp()    * delta.y * speed;
    }

    // ── Alt + LMB — orbitar ───────────────────────────────────────────────────
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && io.KeyAlt)
    {
        m_Yaw   += delta.x * 100.0f;
        m_Pitch -= delta.y * 100.0f;
        m_Pitch  = std::clamp(m_Pitch, -89.0f, 89.0f);
    }

    // ── RMB — freelook + WASD ─────────────────────────────────────────────────
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        // Rotação do olhar
        m_Yaw   += delta.x * 80.0f;
        m_Pitch -= delta.y * 80.0f;
        m_Pitch  = std::clamp(m_Pitch, -89.0f, 89.0f);

        // Velocidade base — Shift acelera
        float speed = m_FlySpeed * deltaTime;
        if (io.KeyShift) speed *= 3.0f;

        glm::vec3 pos = m_FocalPoint - GetForward() * m_Distance;

        if (ImGui::IsKeyDown(ImGuiKey_W)) pos += GetForward() * speed;
        if (ImGui::IsKeyDown(ImGuiKey_S)) pos -= GetForward() * speed;
        if (ImGui::IsKeyDown(ImGuiKey_A)) pos -= GetRight()   * speed;
        if (ImGui::IsKeyDown(ImGuiKey_D)) pos += GetRight()   * speed;
        if (ImGui::IsKeyDown(ImGuiKey_E)) pos += glm::vec3(0, 1, 0) * speed;
        if (ImGui::IsKeyDown(ImGuiKey_Q)) pos -= glm::vec3(0, 1, 0) * speed;

        // Atualiza focal point mantendo distância
        m_FocalPoint = pos + GetForward() * m_Distance;
    }

    // ── Scroll — zoom ─────────────────────────────────────────────────────────
    if (io.MouseWheel != 0)
    {
        m_Distance -= io.MouseWheel * m_Distance * 0.1f;
        m_Distance  = std::clamp(m_Distance, 0.5f, 500.0f);
    }

    // ── F — foca na origem ────────────────────────────────────────────────────
    if (ImGui::IsKeyPressed(ImGuiKey_F))
    {
        m_FocalPoint = { 0.0f, 0.0f, 0.0f };
        m_Distance   = 8.0f;
    }

    RecalculateView();
    (void)deltaTime;
}

void EditorCamera::SetViewportSize(float width, float height)
{
    if (height == 0.0f) return;
    m_AspectRatio = width / height;
    RecalculateProjection();
}

void EditorCamera::RecalculateView()
{
    m_Position = m_FocalPoint - GetForward() * m_Distance;
    m_View     = glm::lookAt(m_Position, m_FocalPoint, glm::vec3{ 0, 1, 0 });
}

void EditorCamera::RecalculateProjection()
{
    m_Projection = glm::perspective(
        ToRad(m_FOV), m_AspectRatio, m_Near, m_Far);
}

void EditorCamera::SetFromViewMatrix(const glm::mat4& view)
{
    // Extrai posição e direção da câmera a partir da view matrix
    glm::mat4 invView = glm::inverse(view);
    glm::vec3 camPos  = glm::vec3(invView[3]);
    glm::vec3 forward = glm::normalize(-glm::vec3(invView[2])); // -Z da câmera = frente

    m_Pitch      = glm::degrees(asin(glm::clamp(forward.y, -1.0f, 1.0f)));
    m_Yaw        = glm::degrees(atan2(forward.z, forward.x));
    m_FocalPoint = camPos + forward * m_Distance;

    RecalculateView();
}