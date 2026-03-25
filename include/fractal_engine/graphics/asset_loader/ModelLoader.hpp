#pragma once

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/material.h>
#include <assimp/types.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// ─── Engine forward declarations ──────────────────────────────────────────────
// Adjust these includes to match your actual project structure.
#include "Model.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "Skeleton.hpp"
#include "Animation.hpp"
#include "Vertex.hpp"

namespace fs = std::filesystem;

// ──────────────────────────────────────────────────────────────────────────────
/// @brief  Static utility that imports a model file via Assimp and returns a
///         fully populated Model (meshes, materials, skeleton, animations, AABB).
///
/// Usage:
/// @code
///     auto model = ModelLoader::load("assets/character.glb");
///     if (!model) { /* handle error */ }
/// @endcode
// ──────────────────────────────────────────────────────────────────────────────
class ModelLoader {
public:
    // ── Public API ─────────────────────────────────────────────────────────────

    /// Loads a model from @p path and returns a shared Model, or nullptr on error.
    static std::shared_ptr<Model> load(const std::string& path);

private:
    // ── Scene traversal ────────────────────────────────────────────────────────

    /// Recursively walks the Assimp node tree and appends meshes to @p model.
    static void processNode(const aiNode*   node,
                            const aiScene*  scene,
                            Model&          model,
                            const fs::path& dir);

    /// Converts a single aiMesh into an engine Mesh (vertices, indices, bone weights).
    static Mesh processMesh(const aiMesh*   mesh,
                            const aiScene*  scene,
                            Model&          model,
                            const fs::path& dir);

    // ── Material loading ───────────────────────────────────────────────────────

    /// Extracts PBR factors and texture paths from an aiMaterial.
    static std::shared_ptr<Material> processMaterial(const aiMaterial* mat,
                                                     const fs::path&   dir);

    /// Loads a single texture slot from an aiMaterial.
    /// @param sRGB  Pass true for colour textures (albedo, emissive).
    static std::shared_ptr<Texture> loadMaterialTexture(const aiMaterial* mat,
                                                        aiTextureType     type,
                                                        TextureType       engineType,
                                                        const fs::path&   dir,
                                                        bool              sRGB = false);

    // ── Skeletal animation ─────────────────────────────────────────────────────

    /// Registers bones from @p mesh into @p skeleton and fills per-vertex bone data.
    static void extractBones(const aiMesh*        mesh,
                             Skeleton&            skeleton,
                             std::vector<Vertex>& vertices);

    /// Recursively copies the Assimp node hierarchy into a SkeletonNode tree.
    static void buildSkeletonNode(const aiNode* node, SkeletonNode& out);

    /// Converts all aiAnimation tracks into engine Animation objects.
    static void extractAnimations(const aiScene* scene, Model& model);

    // ── Assimp → GLM conversion helpers ───────────────────────────────────────

    static glm::mat4 toGLM(const aiMatrix4x4& m);
    static glm::vec3 toGLM(const aiVector3D&  v);
    static glm::quat toGLM(const aiQuaternion& q);
};