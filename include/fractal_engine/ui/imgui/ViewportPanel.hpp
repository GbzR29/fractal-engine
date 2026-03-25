#pragma once
#include "SceneFramebuffer.hpp"
#include "EditorCamera.hpp"
#include <imgui.h>
#include <glm/glm.hpp>

enum class GizmoMode { Translate, Rotate, Scale };

class ViewportPanel
{
public:
    ViewportPanel() = default;

    void Init(int width, int height);

    // Retorna true se a viewport foi resized
    bool Begin(float deltaTime);
    void End();

    void DrawGizmo(glm::mat4& transform);

    SceneFramebuffer& GetFramebuffer()  { return m_Framebuffer; }
    EditorCamera&     GetCamera()       { return m_Camera;      }
    bool              IsHovered()       const { return m_Hovered;      }
    bool              IsFocused()       const { return m_Focused;      }
    ImVec2            GetViewportSize() const { return m_ViewportSize; }
    ImVec2            GetViewportPos()  const { return m_ViewportPos;  }
    GizmoMode         GetGizmoMode()    const { return m_GizmoMode;    }

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