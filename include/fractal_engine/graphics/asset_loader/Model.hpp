/**
 * @file Model.hpp
 * @brief 3-D model aggregate — meshes, PBR materials, skeleton, and animation clips.
 *
 * Models are loaded via @ref ModelLoader and cached by @ref AssetManager.
 * GPU data is uploaded lazily: call @ref uploadToGPU() on the OpenGL thread after an
 * async load completes.
 */
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

/// @brief Container for all data that makes up a 3-D asset.
class Model {
public:
    std::string name;       ///< Human-readable name (usually the filename stem).
    std::string sourcePath; ///< Absolute path from which this model was loaded.

    std::vector<Mesh>                      meshes;     ///< All submeshes.
    std::vector<std::shared_ptr<Material>> materials;  ///< Material palette indexed by Mesh::materialIndex.
    std::unique_ptr<Skeleton>              skeleton;   ///< Bone hierarchy; nullptr for static meshes.
    std::vector<Animation>                 animations; ///< Animation clips (empty for static meshes).

    /**
     * @brief Draws all submeshes using the given shader (static / non-animated).
     * @param shader  A bound PBR shader that accepts material uniforms.
     */
    void draw(Shader& shader) const;

    /**
     * @brief Draws all submeshes with skinning bone transforms uploaded from the animator.
     * @param shader    A bound skinning PBR shader.
     * @param animator  The animator that owns the current bone transform array.
     */
    void drawAnimated(Shader& shader, const Animator& animator) const;

    bool hasSkeleton()   const { return skeleton != nullptr;  } ///< @return @c true if the model has a bone hierarchy.
    bool hasAnimations() const { return !animations.empty();  } ///< @return @c true if at least one animation clip exists.

    /**
     * @brief Searches for an animation clip by name.
     * @param name  Clip name as stored in the source file.
     * @return Pointer to the clip, or @c nullptr if not found.
     */
    const Animation* findAnimation(const std::string& name) const {
        for (auto& a : animations)
            if (a.name == name) return &a;
        return nullptr;
    }

    /// Uploads all mesh vertex/index data to the GPU.  Must be called on the OpenGL thread.
    void uploadToGPU();

    glm::vec3 boundsMin = glm::vec3(0.0f); ///< Axis-aligned bounding box minimum corner (set by @ref ModelLoader).
    glm::vec3 boundsMax = glm::vec3(0.0f); ///< Axis-aligned bounding box maximum corner.

private:
    bool m_uploaded = false;
};