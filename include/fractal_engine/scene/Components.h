#pragma once

#include <string>
#include <unordered_map>

namespace fractal_engine::scene {

class Component {
public:
    virtual ~Component() = default;
    virtual void update(float deltaTime) {}
};

class Components {
public:
    Components() = default;
    virtual ~Components() = default;

    // Component management methods would go here
};

} // namespace fractal_engine::scene
