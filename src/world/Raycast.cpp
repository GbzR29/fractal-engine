#include "fractal_engine/world/Raycast.h"
#include "fractal_engine/world/World.h"
#include <cmath>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// FIXES APLICADOS:
//
//  BUG 1 — lastBlockPos inicializado igual a blockPos do player
//           → adjacentPos ficava sendo a posição do próprio player na 1ª hit
//           FIX: avançamos ANTES de checar solidez. prev só é atualizado
//                depois que confirmamos que o bloco atual é ar.
//
//  BUG 2 — distance = tMax DEPOIS de incrementar tMax
//           → distância registrada era da PRÓXIMA face, não da atual
//           FIX: guardamos `t` ANTES de incrementar tMax.
//
//  BUG 3 — faceHit = 0 hardcoded
//           → normal da face nunca calculada, colocação/remoção descalibrada
//           FIX: faceNormal calculado como -step no eixo que avançou.
//
//  BUG 4 — Bloco inicial (olho do player) nunca era pulado explicitamente
//           → se o player estivesse na borda de um bloco, podia acertar a si mesmo
//           FIX: começamos avançando imediatamente antes do primeiro check.
// ─────────────────────────────────────────────────────────────────────────────

std::optional<RaycastHit> Raycast::raycast(
    const World& world,
    glm::vec3    origin,
    glm::vec3    direction,
    float        maxDistance)
{
    // ── Normaliza direção ──────────────────────────────────────────────────
    glm::vec3 dir = glm::normalize(direction);
    if (glm::dot(dir, dir) < 1e-6f) return std::nullopt;

    // ── Voxel inicial (bloco do olho do player) ────────────────────────────
    glm::ivec3 cur = {
        (int)std::floor(origin.x),
        (int)std::floor(origin.y),
        (int)std::floor(origin.z)
    };

    // ── Direção de passo em cada eixo ─────────────────────────────────────
    glm::ivec3 stepDir = {
        dir.x >= 0.0f ? 1 : -1,
        dir.y >= 0.0f ? 1 : -1,
        dir.z >= 0.0f ? 1 : -1
    };

    // ── tDelta: distância que o raio percorre para cruzar 1 bloco por eixo ─
    auto safeDelta = [](float d) -> float {
        return std::abs(d) > 1e-8f ? 1.0f / std::abs(d) : 1e30f;
    };
    glm::vec3 tDelta = {
        safeDelta(dir.x),
        safeDelta(dir.y),
        safeDelta(dir.z)
    };

    // ── tMax: distância até o PRIMEIRO limite de bloco em cada eixo ───────
    // Se step > 0: distância até floor(o)+1
    // Se step < 0: distância até floor(o)
    auto firstT = [](float o, float d, int s) -> float {
        if (std::abs(d) < 1e-8f) return 1e30f;
        float boundary = (s > 0) ? (std::floor(o) + 1.0f - o)
                                 : (o - std::floor(o));
        // boundary pode ser 0 se estiver exatamente na borda; força mínimo pequeno
        if (boundary < 1e-6f) boundary = 1.0f;
        return boundary / std::abs(d);
    };
    glm::vec3 tMax = {
        firstT(origin.x, dir.x, stepDir.x),
        firstT(origin.y, dir.y, stepDir.y),
        firstT(origin.z, dir.z, stepDir.z)
    };

    // ── Estado de travessia ────────────────────────────────────────────────
    glm::ivec3 prev       = cur;
    glm::ivec3 faceNormal = { 0, -stepDir.y, 0 }; // padrão: face de baixo
    float      hitT       = 0.0f;

    // ─────────────────────────────────────────────────────────────────────
    // Loop DDA:
    // 1. Adianta para o próximo bloco (menor tMax define o eixo)
    // 2. Grava `t` ANTES de incrementar tMax  ← FIX BUG 2
    // 3. Atualiza faceNormal com o eixo cruzado ← FIX BUG 3
    // 4. Checa se o novo bloco é sólido
    // 5. Se for: prev é a adjacente correta   ← FIX BUG 1 / BUG 4
    // ─────────────────────────────────────────────────────────────────────
    for (int i = 0; i < 512; i++) {

        // Escolhe o eixo com menor tMax
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            hitT = tMax.x;                    // ← distância REAL ao hit (BUG 2 fix)
            if (hitT > maxDistance) break;
            prev       = cur;
            cur.x     += stepDir.x;
            faceNormal = { -stepDir.x, 0, 0 }; // normal aponta para FORA do bloco
            tMax.x    += tDelta.x;

        } else if (tMax.y < tMax.z) {
            hitT = tMax.y;
            if (hitT > maxDistance) break;
            prev       = cur;
            cur.y     += stepDir.y;
            faceNormal = { 0, -stepDir.y, 0 };
            tMax.y    += tDelta.y;

        } else {
            hitT = tMax.z;
            if (hitT > maxDistance) break;
            prev       = cur;
            cur.z     += stepDir.z;
            faceNormal = { 0, 0, -stepDir.z };
            tMax.z    += tDelta.z;
        }

        // Checa bloco recém-entrado
        if (!world.isAirWorld(cur.x, cur.y, cur.z)) {
            RaycastHit hit;
            hit.blockPos    = cur;           // bloco sólido atingido
            hit.adjacentPos = prev;          // último ar antes do hit (para place)
            hit.faceNormal  = faceNormal;    // normal da face atingida
            hit.distance    = hitT;          // distância real calibrada
            return hit;
        }
    }

    return std::nullopt;
}

} // namespace fractal_engine::world