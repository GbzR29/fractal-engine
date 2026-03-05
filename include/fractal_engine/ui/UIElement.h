#pragma once

#include <string>
#include <third_party/glm/glm.hpp>
#include <third_party/glad/glad.h>

namespace fractal_engine::ui {

class UIElement {
public:
    UIElement() = default;
    virtual ~UIElement() = default;

    virtual void update(float deltaTime) {}
    virtual void render() {}

    void setPosition(const glm::vec2& pos) { position = pos; }
    void setSize(const glm::vec2& s) { size = s; }

protected:
    glm::vec2 position{0.0f};
    glm::vec2 size{100.0f};
    bool visible{true};
};

} // namespace fractal_engine::ui
