#pragma once

#include "Mesh.hpp"
#include "Material.hpp"
#include "Skeleton.hpp"
#include "Animation.hpp"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

class Shader;

class Model {
public:
    std::string name;
    std::string sourcePath;

    std::vector<Mesh>                    meshes;
    std::vector<std::shared_ptr<Material>> materials;
    std::unique_ptr<Skeleton>            skeleton;
    std::vector<Animation>               animations;

    // ── Rendering ──────────────────────────────
    void draw(Shader& shader) const;
    void drawAnimated(Shader& shader, const Animator& animator) const;

    // ── Queries ────────────────────────────────
    bool hasSkeleton()   const { return skeleton != nullptr; }
    bool hasAnimations() const { return !animations.empty(); }

    const Animation* findAnimation(const std::string& name) const {
        for (auto& a : animations)
            if (a.name == name) return &a;
        return nullptr;
    }

    // ── GPU upload (call on GL thread after async load) ──
    void uploadToGPU();

    // ── AABB (set during load) ──────────────────
    glm::vec3 boundsMin = glm::vec3(0.0f);
    glm::vec3 boundsMax = glm::vec3(0.0f);

private:
    bool m_uploaded = false;
};