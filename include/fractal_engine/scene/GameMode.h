#pragma once

namespace fractal_engine::scene {

enum class GameMode {
    Creative,   // slots infinitos, sem consumo
    Survival    // stack limitado (max 64), consumo ao colocar
};

} // namespace fractal_engine::scene