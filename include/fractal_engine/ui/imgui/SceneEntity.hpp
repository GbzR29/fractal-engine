#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ─────────────────────────────────────────────────────────────────────────────
//  Componentes básicos
// ─────────────────────────────────────────────────────────────────────────────
struct TransformComponent
{
    glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f }; // Euler em graus
    glm::vec3 Scale    = { 1.0f, 1.0f, 1.0f };

    glm::mat4 GetMatrix() const
    {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), Position);
        glm::mat4 r = glm::rotate(glm::mat4(1.0f),
            glm::radians(Rotation.x), { 1, 0, 0 });
        r = glm::rotate(r, glm::radians(Rotation.y), { 0, 1, 0 });
        r = glm::rotate(r, glm::radians(Rotation.z), { 0, 0, 1 });
        glm::mat4 s = glm::scale(glm::mat4(1.0f), Scale);
        return t * r * s;
    }
};

struct CameraComponent
{
    float FOV        = 60.0f;
    float Near       = 0.1f;
    float Far        = 1000.0f;
    bool  IsPrimary  = true;   // câmera usada pelo GameViewport
    bool  IsOrthographic = false;

    glm::mat4 GetProjection(float aspectRatio) const
    {
        if (IsOrthographic)
            return glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, Near, Far);
        return glm::perspective(glm::radians(FOV), aspectRatio, Near, Far);
    }
};

struct TagComponent
{
    std::string Name = "Entity";
    std::string Tag  = "Untagged";
};

// ─────────────────────────────────────────────────────────────────────────────
//  Entidade — nó da cena
// ─────────────────────────────────────────────────────────────────────────────
struct SceneEntity
{
    uint32_t   ID = 0;
    bool       Active   = true;
    bool       Selected = false;
    bool       Open     = false;  // tree node aberto na hierarquia

    TagComponent       Tag;
    TransformComponent Transform;

    // Componentes opcionais (nullptr = não tem)
    std::unique_ptr<CameraComponent> Camera;

    bool HasCamera() const { return Camera != nullptr; }

    // Filhos na hierarquia
    std::vector<std::unique_ptr<SceneEntity>> Children;

    // Construtor com nome
    explicit SceneEntity(const std::string& name = "Entity")
    {
        static uint32_t s_NextID = 1;
        ID       = s_NextID++;
        Tag.Name = name;
    }
};