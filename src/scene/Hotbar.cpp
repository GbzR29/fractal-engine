#include "fractal_engine/scene/Hotbar.h"
#include <algorithm>

namespace fractal_engine::scene {

// ─────────────────────────────────────────────────────────────────────────────
// init — preenche slots com blocos do registry
// ─────────────────────────────────────────────────────────────────────────────
void Hotbar::init() {
    slots.fill(BlockType::BLOCK_AIR);
    populateFromRegistry();
}

void Hotbar::populateFromRegistry() {
    const auto& placeable = BlockRegistry::getPlaceableBlocks();

    // Preenche até SLOTS slots com os blocos registrados como colocáveis
    // Se houver menos blocos que slots, os restantes ficam BLOCK_AIR
    for (int i = 0; i < SLOTS && i < (int)placeable.size(); i++) {
        slots[i] = placeable[i];
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// update — sem nenhum BlockType hardcoded
//
// Teclas 1-9 → seleciona slot diretamente
// Scroll     → navega entre slots (wraps ao chegar no fim)
// ─────────────────────────────────────────────────────────────────────────────
void Hotbar::update(const Input& input) {
    // ── Teclas numéricas ──────────────────────────────────────────────────
    // Mapeamento direto de tecla → índice de slot, sem saber o BlockType
    const bool keys[SLOTS] = {
        input.key1Pressed, input.key2Pressed, input.key3Pressed,
        input.key4Pressed, input.key5Pressed, input.key6Pressed,
        input.key7Pressed, input.key8Pressed, input.key9Pressed
    };

    for (int i = 0; i < SLOTS; i++) {
        if (keys[i]) {
            selectedSlot = i;
            return;  // tecla tem prioridade sobre scroll
        }
    }

    // ── Scroll do mouse ───────────────────────────────────────────────────
    int scrollDelta = input.scrollVal - prevScrollVal;
    prevScrollVal   = input.scrollVal;

    if (scrollDelta != 0) {
        selectedSlot = (selectedSlot - scrollDelta + SLOTS) % SLOTS;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
BlockType Hotbar::getSelectedBlock() const {
    return slots[selectedSlot];
}

const std::string& Hotbar::getSelectedName() const {
    BlockType type = slots[selectedSlot];
    if (type == BlockType::BLOCK_AIR)
        return BlockRegistry::get(BlockType::BLOCK_AIR).displayName;
    return BlockRegistry::get(type).displayName;
}

BlockType Hotbar::getSlot(int index) const {
    if (index < 0 || index >= SLOTS) return BlockType::BLOCK_AIR;
    return slots[index];
}

void Hotbar::setSlot(int index, BlockType type) {
    if (index < 0 || index >= SLOTS) return;
    slots[index] = type;
}

} // namespace fractal_engine::scene