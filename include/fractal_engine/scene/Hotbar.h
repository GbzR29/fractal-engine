#pragma once
#include "fractal_engine/world/BlockType.h"
#include "fractal_engine/world/BlockRegistry.h"
#include "fractal_engine/input/Input.h"
#include "fractal_engine/scene/GameMode.h"
#include <array>
#include <string>

namespace fractal_engine::scene {

using fractal_engine::world::BlockType;
using fractal_engine::world::BlockRegistry;
using fractal_engine::input::Input;

// ─────────────────────────────────────────────────────────────────────────────
// SlotStack — bloco + quantidade
//
// Em modo criativo: count = -1 (infinito)
// Em modo survival: count >= 0, max = MAX_STACK
// ─────────────────────────────────────────────────────────────────────────────
struct SlotStack {
    BlockType type  = BlockType::BLOCK_AIR;
    int       count = 0;   // -1 = infinito (criativo)

    bool isEmpty()    const { return type == BlockType::BLOCK_AIR || count == 0; }
    bool isInfinite() const { return count == -1; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Hotbar — 9 slots com suporte a GameMode
// ─────────────────────────────────────────────────────────────────────────────
class Hotbar {
public:
    static constexpr int SLOTS     = 9;
    static constexpr int MAX_STACK = 64;

    // Inicializa com o modo de jogo
    void init(GameMode mode = GameMode::Creative);

    // Muda o modo de jogo (reseta stacks)
    void setGameMode(GameMode mode);
    GameMode getGameMode() const { return gameMode; }

    // Atualiza seleção via input
    void update(const Input& input);

    // ── Acesso ao slot selecionado ─────────────────────────────────────────
    BlockType          getSelectedBlock() const;
    const std::string& getSelectedName()  const;
    int                getSelectedCount() const;
    int                getSelectedIndex() const { return selectedSlot; }

    // ── Acesso direto a slots ──────────────────────────────────────────────
    const SlotStack& getSlot(int index) const;
    void             setSlot(int index, BlockType type, int count = -1);

    // ── Operações de survival ──────────────────────────────────────────────
    // Consome 1 do slot selecionado (no-op em criativo)
    // Retorna false se o slot ficou vazio
    bool consumeSelected();

    // Adiciona count itens ao slot (ou ao primeiro slot compatível)
    // Retorna quantidade que não coube
    int  addItem(BlockType type, int count = 1);

private:
    std::array<SlotStack, SLOTS> slots {};
    int      selectedSlot  = 0;
    int      prevScrollVal = 0;
    GameMode gameMode      = GameMode::Creative;

    void populateFromRegistry();
};

} // namespace fractal_engine::scene