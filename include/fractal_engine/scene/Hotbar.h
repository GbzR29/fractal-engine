#pragma once
#include "fractal_engine/world/BlockType.h"
#include "fractal_engine/world/BlockRegistry.h"
#include "fractal_engine/input/Input.h"
#include <array>

namespace fractal_engine::scene {

using fractal_engine::world::BlockType;
using fractal_engine::world::BlockRegistry;
using fractal_engine::input::Input;

// ─────────────────────────────────────────────────────────────────────────────
// Hotbar — 9 slots de blocos selecionáveis pelo player
//
// Não tem nenhum BlockType hardcoded.
// Os slots são preenchidos automaticamente com os blocos colocáveis
// registrados no BlockRegistry — se você adicionar 50 blocos, a hotbar
// simplesmente os terá disponíveis via scroll.
//
// Teclas 1-9 selecionam slots diretamente.
// Scroll do mouse navega entre slots.
// ─────────────────────────────────────────────────────────────────────────────
class Hotbar {
public:
    static constexpr int SLOTS = 9;

    // Preenche os slots com os primeiros SLOTS blocos colocáveis do registry
    void init();

    // Atualiza seleção via input (teclas + scroll)
    void update(const Input& input);

    // Bloco atualmente selecionado
    BlockType getSelectedBlock() const;

    // Índice do slot selecionado (0-8)
    int getSelectedIndex() const { return selectedSlot; }

    // Nome do bloco selecionado (para HUD)
    const std::string& getSelectedName() const;

    // Acesso direto a um slot
    BlockType getSlot(int index) const;
    void      setSlot(int index, BlockType type);

private:
    std::array<BlockType, SLOTS> slots {};
    int selectedSlot  = 0;
    int prevScrollVal = 0;

    // Popula slots com os blocos colocáveis do registry
    void populateFromRegistry();
};

} // namespace fractal_engine::scene