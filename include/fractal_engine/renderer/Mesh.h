#pragma once

#include <third_party/glm/glm.hpp>
#include <third_party/glad/glad.h>
#include <vector>

namespace fractal_engine::renderer {

class Mesh {
public:
    Mesh() = default;
    virtual ~Mesh() = default;

    // Mesh methods would go here
};

} // namespace fractal_engine::renderer
