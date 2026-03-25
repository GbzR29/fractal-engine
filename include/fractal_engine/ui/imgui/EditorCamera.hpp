#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class EditorCamera
{
public:
    EditorCamera() { RecalculateProjection(); RecalculateView(); }

    void OnUpdate(float deltaTime);
    void SetViewportSize(float width, float height);

    const glm::mat4& GetViewMatrix()       const { return m_View;       }
    const glm::mat4& GetProjectionMatrix() const { return m_Projection; }
    glm::mat4        GetViewProjection()   const { return m_Projection * m_View; }
    glm::vec3        GetPosition()         const { return m_Position;   }

    float GetNearClip() const { return m_Near; }
    float GetFarClip()  const { return m_Far;  }

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