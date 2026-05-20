/**
 * @file Vertex.hpp
 * @brief Interleaved vertex layout used for all mesh geometry.
 *
 * The layout matches the VAO attribute setup in @ref Mesh::setup():
 * - Location 0: position (vec3)
 * - Location 1: normal   (vec3)
 * - Location 2: texCoords (vec2)
 * - Location 3: tangent  (vec3)
 * - Location 4: bitangent (vec3)
 * - Location 5: boneIDs  (ivec4)
 * - Location 6: boneWeights (vec4)
 */
#pragma once

#include <glm/glm.hpp>

constexpr int MAX_BONE_INFLUENCE = 4; ///< Maximum number of bones that can influence a single vertex.

/// @brief Single vertex with full PBR + skeletal animation data.
struct Vertex {
    glm::vec3 position  = glm::vec3(0.0f); ///< Object-space position.
    glm::vec3 normal    = glm::vec3(0.0f); ///< Object-space normal (normalised).
    glm::vec2 texCoords = glm::vec2(0.0f); ///< UV texture coordinates.
    glm::vec3 tangent   = glm::vec3(0.0f); ///< Tangent vector (for normal mapping).
    glm::vec3 bitangent = glm::vec3(0.0f); ///< Bitangent vector (for normal mapping).

    int   boneIDs    [MAX_BONE_INFLUENCE] = { -1, -1, -1, -1 }; ///< Bone palette indices (-1 = unused slot).
    float boneWeights[MAX_BONE_INFLUENCE] = {  0,  0,  0,  0 }; ///< Corresponding bone weights (should sum to 1).

    /**
     * @brief Adds a bone influence to the first empty slot.
     *        Silently discards extra influences beyond @ref MAX_BONE_INFLUENCE.
     * @param boneID  Bone palette index.
     * @param weight  Influence weight.
     */
    void addBoneData(int boneID, float weight) {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
            if (boneIDs[i] < 0) {
                boneIDs[i]     = boneID;
                boneWeights[i] = weight;
                return;
            }
        }
    }
};