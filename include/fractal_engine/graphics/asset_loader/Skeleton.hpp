#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>
#include <unordered_map>

// ──────────────────────────────────────────────
//  Bone data
// ──────────────────────────────────────────────
struct BoneInfo {
    int       id;
    glm::mat4 offsetMatrix; // mesh-space → bone-local (bind-pose inverse)
};

// ──────────────────────────────────────────────
//  Skeleton hierarchy node
// ──────────────────────────────────────────────
struct SkeletonNode {
    std::string            name;
    glm::mat4              localTransform; // default transform from file
    std::vector<SkeletonNode> children;
};

// ──────────────────────────────────────────────
//  Skeleton
// ──────────────────────────────────────────────
class Skeleton {
public:
    std::unordered_map<std::string, BoneInfo> boneMap;
    SkeletonNode                              rootNode;
    glm::mat4                                 globalInverseTransform = glm::mat4(1.0f);
    int                                       boneCount = 0;

    bool hasBone(const std::string& name) const {
        return boneMap.count(name) > 0;
    }

    int getBoneID(const std::string& name) const {
        auto it = boneMap.find(name);
        return it != boneMap.end() ? it->second.id : -1;
    }

    int addBone(const std::string& name, const glm::mat4& offsetMatrix) {
        if (boneMap.count(name)) return boneMap[name].id;
        boneMap[name] = { boneCount, offsetMatrix };
        return boneCount++;
    }
};