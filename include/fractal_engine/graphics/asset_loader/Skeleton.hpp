/**
 * @file Skeleton.hpp
 * @brief Bone hierarchy data used for skeletal animation.
 *
 * @ref Skeleton is built by @ref ModelLoader from the imported scene's node tree.
 * @ref Animator traverses the @ref SkeletonNode hierarchy each frame to produce
 * the final bone transform palette.
 */
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>
#include <unordered_map>

// ─────────────────────────────────────────────
//  Bone data
// ─────────────────────────────────────────────

/// @brief Per-bone index and bind-pose inverse matrix.
struct BoneInfo {
    int       id;           ///< Index into the bone transform palette (0-based).
    glm::mat4 offsetMatrix; ///< Transform from mesh space to bone-local space (bind-pose inverse).
};

// ─────────────────────────────────────────────
//  Skeleton hierarchy node
// ─────────────────────────────────────────────

/// @brief One node in the skeleton hierarchy tree.
struct SkeletonNode {
    std::string               name;           ///< Node name; must match bone name in @ref BoneInfo.
    glm::mat4                 localTransform; ///< Default local transform from the source file.
    std::vector<SkeletonNode> children;       ///< Child nodes.
};

// ─────────────────────────────────────────────
//  Skeleton
// ─────────────────────────────────────────────

/// @brief Complete skeleton: bone map, hierarchy root, and global inverse transform.
class Skeleton {
public:
    std::unordered_map<std::string, BoneInfo> boneMap;                         ///< Maps bone name → BoneInfo.
    SkeletonNode                              rootNode;                         ///< Root of the skeleton hierarchy.
    glm::mat4                                 globalInverseTransform = glm::mat4(1.0f); ///< Inverse of the root node's global transform at bind pose.
    int                                       boneCount = 0;                   ///< Total number of bones registered.

    /// @return @c true if a bone with the given name is registered.
    bool hasBone(const std::string& name) const {
        return boneMap.count(name) > 0;
    }

    /**
     * @brief Returns the palette index for the given bone name.
     * @param name  Bone name.
     * @return Index in [0, boneCount), or @c -1 if the bone is not found.
     */
    int getBoneID(const std::string& name) const {
        auto it = boneMap.find(name);
        return it != boneMap.end() ? it->second.id : -1;
    }

    /**
     * @brief Registers a new bone, or returns its existing index if already registered.
     * @param name          Bone name.
     * @param offsetMatrix  Bind-pose inverse matrix.
     * @return The bone's palette index.
     */
    int addBone(const std::string& name, const glm::mat4& offsetMatrix) {
        if (boneMap.count(name)) return boneMap[name].id;
        boneMap[name] = { boneCount, offsetMatrix };
        return boneCount++;
    }
};