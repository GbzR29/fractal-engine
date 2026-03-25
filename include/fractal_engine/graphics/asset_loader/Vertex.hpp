#pragma once

#include <glm/glm.hpp>

constexpr int MAX_BONE_INFLUENCE = 4;

struct Vertex {
    glm::vec3 position    = glm::vec3(0.0f);
    glm::vec3 normal      = glm::vec3(0.0f);
    glm::vec2 texCoords   = glm::vec2(0.0f);
    glm::vec3 tangent     = glm::vec3(0.0f);
    glm::vec3 bitangent   = glm::vec3(0.0f);

    // Skeletal animation
    int       boneIDs[MAX_BONE_INFLUENCE]     = { -1, -1, -1, -1 };
    float     boneWeights[MAX_BONE_INFLUENCE] = {  0,  0,  0,  0 };

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