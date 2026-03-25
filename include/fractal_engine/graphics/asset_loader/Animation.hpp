#pragma once

#include "Skeleton.hpp"
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <string>
#include <unordered_map>

// ──────────────────────────────────────────────
//  Keyframes
// ──────────────────────────────────────────────
struct KeyPosition { glm::vec3 value; float time; };
struct KeyRotation { glm::quat value; float time; };
struct KeyScale    { glm::vec3 value; float time; };

// ──────────────────────────────────────────────
//  Per-bone animation channel
// ──────────────────────────────────────────────
struct BoneChannel {
    std::string            boneName;
    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale>    scales;

    glm::mat4 interpolate(float animTime) const;

private:
    template<typename T>
    static int findIndex(const std::vector<T>& keys, float t) {
        for (int i = 0; i + 1 < (int)keys.size(); ++i)
            if (t < keys[i + 1].time) return i;
        return (int)keys.size() - 2;
    }

    static float factor(float t0, float t1, float t) {
        return (t - t0) / (t1 - t0);
    }
};

// ──────────────────────────────────────────────
//  Animation clip
// ──────────────────────────────────────────────
class Animation {
public:
    std::string name;
    float       duration       = 0.0f; // in ticks
    float       ticksPerSecond = 25.0f;

    std::unordered_map<std::string, BoneChannel> channels;

    float durationSeconds() const { return duration / ticksPerSecond; }
    float toTicks(float seconds) const { return seconds * ticksPerSecond; }
};

// ──────────────────────────────────────────────
//  Animator — drives bone transforms
// ──────────────────────────────────────────────
class Animator {
public:
    static constexpr int MAX_BONES = 100;

    void update(float deltaTime);
    void play(const Animation* anim, bool loop = true);
    void crossFadeTo(const Animation* anim, float blendDuration, bool loop = true);

    const std::vector<glm::mat4>& getBoneTransforms() const { return m_boneTransforms; }

    void computeTransforms(const SkeletonNode& node,
                           const glm::mat4&    parentTransform,
                           const Skeleton&     skeleton,
                           float               animTime);

private:
    const Animation* m_current     = nullptr;
    const Animation* m_next        = nullptr;
    float            m_time        = 0.0f;
    float            m_blendFactor = 0.0f;
    float            m_blendDur    = 0.0f;
    bool             m_loop        = true;

    std::vector<glm::mat4> m_boneTransforms = std::vector<glm::mat4>(MAX_BONES, glm::mat4(1.0f));

    glm::mat4 sampleChannel(const Animation* anim, const std::string& bone, float t) const;
};

// ──────────────────────────────────────────────
//  BoneChannel::interpolate  (inline impl)
// ──────────────────────────────────────────────
inline glm::mat4 BoneChannel::interpolate(float animTime) const {
    glm::vec3 pos = glm::vec3(0.0f);
    if (!positions.empty()) {
        if (positions.size() == 1) {
            pos = positions[0].value;
        } else {
            int i  = findIndex(positions, animTime);
            float f = factor(positions[i].time, positions[i+1].time, animTime);
            pos     = glm::mix(positions[i].value, positions[i+1].value, f);
        }
    }

    glm::quat rot = glm::quat(1,0,0,0);
    if (!rotations.empty()) {
        if (rotations.size() == 1) {
            rot = rotations[0].value;
        } else {
            int i  = findIndex(rotations, animTime);
            float f = factor(rotations[i].time, rotations[i+1].time, animTime);
            rot     = glm::slerp(rotations[i].value, rotations[i+1].value, f);
        }
    }

    glm::vec3 scl = glm::vec3(1.0f);
    if (!scales.empty()) {
        if (scales.size() == 1) {
            scl = scales[0].value;
        } else {
            int i  = findIndex(scales, animTime);
            float f = factor(scales[i].time, scales[i+1].time, animTime);
            scl     = glm::mix(scales[i].value, scales[i+1].value, f);
        }
    }

    return glm::translate(glm::mat4(1.0f), pos)
         * glm::mat4_cast(rot)
         * glm::scale(glm::mat4(1.0f), scl);
}