#pragma once

#include <third_party/glm/glm.hpp>
#include <third_party/glm/gtc/matrix_transform.hpp>
#include <third_party/glad/glad.h>

namespace fractal_engine::renderer {

class Renderer {
public:
    Renderer() = default;
    virtual ~Renderer() = default;

    // Renderer methods would go here
};

} // namespace fractal_engine::renderer
