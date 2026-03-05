#pragma once

#include <vector>
#include <string>
#include <memory>
#include "fractal_engine/scene/Entity.h"

namespace fractal_engine::scene {

class Scene {
public:
    Scene(const std::string& name = "Scene");
    virtual ~Scene() = default;

    virtual void update(float deltaTime);
    virtual void render();

    Entity* createEntity(const std::string& name);
    void removeEntity(Entity* entity);
    const std::vector<std::unique_ptr<Entity>>& getEntities() const { return entities; }

private:
    std::string name;
    std::vector<std::unique_ptr<Entity>> entities;
};

} // namespace fractal_engine::scene
