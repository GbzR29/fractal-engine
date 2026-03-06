#include "fractal_engine/world/Raycast.h"
#include "fractal_engine/world/World.h"
#include <cmath>
#include <iostream>

namespace fractal_engine::world {

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
    // FIX: Removido o clamp "if (boundary < 1e-6f) boundary = 1.0f"
    // Esse clamp causava um overshoot de 1 bloco inteiro quando o player
    // estava exatamente sobre uma borda de bloco (ex.: Y=65.0 após pousar).
    // Com boundary ≈ 0, tMax ≈ 0, o DDA avança imediatamente — correto.
    auto firstT = [](float o, float d, int s) -> float {
        if (std::abs(d) < 1e-8f) return 1e30f;
        float boundary = (s > 0) ? (std::floor(o) + 1.0f - o)
                                 : (o - std::floor(o));
        // boundary = 0 significa que estamos exatamente na borda:
        // tMax = 0 → passo imediato na próxima iteração. Isso é correto.
        return boundary / std::abs(d);
    };
    glm::vec3 tMax = {
        firstT(origin.x, dir.x, stepDir.x),
        firstT(origin.y, dir.y, stepDir.y),
        firstT(origin.z, dir.z, stepDir.z)
    };

    // ── Estado de travessia ────────────────────────────────────────────────
    glm::ivec3 prev       = cur;
    glm::ivec3 faceNormal = { 0, -stepDir.y, 0 };
    float      hitT       = 0.0f;

    for (int i = 0; i < 512; i++) {

        // Escolhe o eixo com menor tMax
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            hitT = tMax.x;
            if (hitT > maxDistance) break;
            prev       = cur;
            cur.x     += stepDir.x;
            faceNormal = { -stepDir.x, 0, 0 };
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
            hit.blockPos    = cur;    // bloco sólido atingido
            hit.adjacentPos = prev;   // último bloco de ar antes do hit
            hit.faceNormal  = faceNormal;
            hit.distance    = hitT;
            return hit;
        }
    }

    return std::nullopt;
}

} // namespace fractal_engine::world