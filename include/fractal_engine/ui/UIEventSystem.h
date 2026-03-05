#pragma once

#include <functional>
#include <vector>
#include <string>

namespace fractal_engine::ui {

class UIEventSystem {
public:
    UIEventSystem() = default;
    virtual ~UIEventSystem() = default;

    using EventCallback = std::function<void(void*)>;

    void subscribe(const std::string& eventName, EventCallback callback);
    void dispatch(const std::string& eventName, void* data = nullptr);

private:
    std::unordered_map<std::string, std::vector<EventCallback>> listeners;
};

} // namespace fractal_engine::ui
