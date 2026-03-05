#pragma once

#include <third_party/glad/glad.h>
#include "fractal_engine/ui/UIElement.h"

namespace fractal_engine::ui {

class UIRenderer {
public:
    UIRenderer() = default;
    virtual ~UIRenderer() = default;

    void renderElement(const UIElement& element);
    void setupFramebuffer(int width, int height);

private:
    GLuint framebuffer{0};
    GLuint colorTexture{0};
};

} // namespace fractal_engine::ui
