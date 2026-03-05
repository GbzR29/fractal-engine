#pragma once

#include <memory>
#include <vector>
#include "fractal_engine/ui/UIElement.h"
#include "fractal_engine/ui/UIEventSystem.h"

namespace fractal_engine::ui {

class UIManager {
public:
    UIManager() = default;
    virtual ~UIManager() = default;

    void update(float deltaTime);
    void render();

    void addElement(std::unique_ptr<UIElement> element);
    void removeElement(UIElement* element);

    UIEventSystem& getEventSystem() { return eventSystem; }

private:
    std::vector<std::unique_ptr<UIElement>> elements;
    UIEventSystem eventSystem;
};

} // namespace fractal_engine::ui
