#include "fractal_engine/scene/Hotbar.h"
#include <algorithm>

namespace fractal_engine::scene {

// ─────────────────────────────────────────────────────────────────────────────
// init
// ─────────────────────────────────────────────────────────────────────────────
void Hotbar::init(GameMode mode) {
    gameMode = mode;
    for (auto& s : slots) s = {};
    populateFromRegistry();
}

void Hotbar::setGameMode(GameMode mode) {
    gameMode = mode;
    // Ajusta counts existentes
    for (auto& s : slots) {
        if (s.type == BlockType::BLOCK_AIR) continue;
        if (mode == GameMode::Creative) {
            s.count = -1;
        } else {
            if (s.count < 0) s.count = MAX_STACK; // infinito → 64
        }
    }
}

void Hotbar::populateFromRegistry() {
    const auto& placeable = BlockRegistry::getPlaceableBlocks();
    for (int i = 0; i < SLOTS && i < (int)placeable.size(); i++) {
        slots[i].type  = placeable[i];
        slots[i].count = (gameMode == GameMode::Creative) ? -1 : MAX_STACK;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// update
// ─────────────────────────────────────────────────────────────────────────────
void Hotbar::update(const Input& input) {
    const bool keys[SLOTS] = {
        input.key1Pressed, input.key2Pressed, input.key3Pressed,
        input.key4Pressed, input.key5Pressed, input.key6Pressed,
        input.key7Pressed, input.key8Pressed, input.key9Pressed
    };
    for (int i = 0; i < SLOTS; i++) {
        if (keys[i]) { selectedSlot = i; return; }
    }

    int scrollDelta = input.scrollVal - prevScrollVal;
    prevScrollVal   = input.scrollVal;
    if (scrollDelta != 0)
        selectedSlot = (selectedSlot - scrollDelta + SLOTS) % SLOTS;
}

// ─────────────────────────────────────────────────────────────────────────────
// Getters
// ─────────────────────────────────────────────────────────────────────────────
BlockType Hotbar::getSelectedBlock() const {
    return slots[selectedSlot].type;
}

const std::string& Hotbar::getSelectedName() const {
    return BlockRegistry::get(slots[selectedSlot].type).displayName;
}

int Hotbar::getSelectedCount() const {
    return slots[selectedSlot].count;
}

const SlotStack& Hotbar::getSlot(int index) const {
    static SlotStack empty{};
    if (index < 0 || index >= SLOTS) return empty;
    return slots[index];
}

void Hotbar::setSlot(int index, BlockType type, int count) {
    if (index < 0 || index >= SLOTS) return;
    slots[index].type  = type;
    slots[index].count = (gameMode == GameMode::Creative) ? -1 : count;
}

// ─────────────────────────────────────────────────────────────────────────────
// consumeSelected — só consome em Survival
// ─────────────────────────────────────────────────────────────────────────────
bool Hotbar::consumeSelected() {
    if (gameMode == GameMode::Creative) return true; // infinito

    auto& slot = slots[selectedSlot];
    if (slot.isEmpty()) return false;

    slot.count--;
    if (slot.count <= 0) {
        slot = {}; // esvazia o slot
        return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// addItem — empilha em slot existente ou ocupa slot vazio
// ─────────────────────────────────────────────────────────────────────────────
int Hotbar::addItem(BlockType type, int count) {
    if (gameMode == GameMode::Creative) return 0; // criativo: sem coleta

    int remaining = count;

    // Primeiro: tenta empilhar em slots do mesmo tipo
    for (auto& slot : slots) {
        if (remaining <= 0) break;
        if (slot.type == type && slot.count < MAX_STACK) {
            int space = MAX_STACK - slot.count;
            int add   = std::min(space, remaining);
            slot.count  += add;
            remaining   -= add;
        }
    }

    // Segundo: ocupa slots vazios
    for (auto& slot : slots) {
        if (remaining <= 0) break;
        if (slot.isEmpty()) {
            int add     = std::min(MAX_STACK, remaining);
            slot.type   = type;
            slot.count  = add;
            remaining  -= add;
        }
    }

    return remaining; // quantidade que não coube
}

} // namespace fractal_engine::scene