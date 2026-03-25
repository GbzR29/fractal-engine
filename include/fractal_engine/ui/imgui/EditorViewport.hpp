#pragma once
#include "SceneFramebuffer.hpp"
#include "EditorCamera.hpp"
#include "EditorGrid.hpp"
#include <imgui.h>
#include <glm/glm.hpp>

enum class GizmoMode { Translate, Rotate, Scale };

// Viewport de edição de cena:
// - FBO próprio
// - EditorCamera (orbital Alt+LMB / MMB / Scroll)
// - Grid infinito no plano Y=0
// - Toolbar com botões T/R/S e info overlay
class EditorViewport
{
public:
    bool Init(int width, int height);

    // Chame ANTES de renderizar a cena
    void BindFramebuffer();
    void UnbindFramebuffer();

    // Chame dentro do loop ImGui (abre/fecha a janela "Scene")
    void Begin(float deltaTime);
    void End();

    // Chame entre Begin() e End() para desenhar o gizmo
    void DrawGizmo(glm::mat4& transform);

    // Grid — chame dentro do BindFramebuffer()/Unbind()
    void DrawGrid();

    SceneFramebuffer& GetFramebuffer() { return m_FBO;    }
    EditorCamera&     GetCamera()      { return m_Camera; }
    GizmoMode         GetGizmoMode()   const { return m_GizmoMode; }
    bool              IsHovered()      const { return m_Hovered;   }
    bool              IsFocused()      const { return m_Focused;   }
    ImVec2            GetSize()        const { return m_Size;      }

private:
    void DrawOverlayToolbar();
    void DrawInfoOverlay();

    SceneFramebuffer m_FBO;
    EditorCamera     m_Camera;
    EditorGrid       m_Grid;

    GizmoMode m_GizmoMode = GizmoMode::Translate;
    ImVec2    m_Size      = { 1280, 720 };
    ImVec2    m_WinPos    = { 0, 0 };
    bool      m_Hovered   = false;
    bool      m_Focused   = false;
    bool      m_GridInit  = false;
};