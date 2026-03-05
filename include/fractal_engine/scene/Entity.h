#pragma once

#include <string>
#include <vector>
#include <third_party/glm/glm.hpp>
#include "fractal_engine/scene/Components.h"

namespace fractal_engine::scene {

class Entity {
public:
    Entity(const std::string& name = "Entity");
    virtual ~Entity() = default;

    virtual void update(float deltaTime);
    virtual void render();

    const std::string& getName() const { return name; }
    void setPosition(const glm::vec3& pos) { position = pos; }
    const glm::vec3& getPosition() const { return position; }

private:
    std::string name;
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
    std::vector<Component*> components;
};

} // namespace fractal_engine::scene
