/**
 * @file Animation.hpp
 * @brief Skeletal animation: keyframe data, animation clips, and the Animator runtime.
 *
 * Data flow:
 * 1. @ref ModelLoader populates @ref Animation clips inside a @ref Model.
 * 2. @ref Animator::play() selects an active clip.
 * 3. @ref Animator::update() advances time and fills @c m_boneTransforms.
 * 4. @ref Model::drawAnimated() uploads the bone transforms to the skinning shader.
 */
#pragma once

#include "Skeleton.hpp"
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>
#include <unordered_map>

// ─────────────────────────────────────────────
//  Keyframe types
// ─────────────────────────────────────────────

struct KeyPosition { glm::vec3 value; float time; }; ///< A single position keyframe.
struct KeyRotation { glm::quat value; float time; }; ///< A single rotation keyframe (quaternion).
struct KeyScale    { glm::vec3 value; float time; }; ///< A single scale keyframe.

// ─────────────────────────────────────────────
//  Per-bone animation channel
// ─────────────────────────────────────────────

/// @brief Keyframe tracks for a single bone within one animation clip.
struct BoneChannel {
    std::string              boneName;  ///< Bone name; must match a key in @ref Skeleton::boneMap.
    std::vector<KeyPosition> positions; ///< Position keyframe track.
    std::vector<KeyRotation> rotations; ///< Rotation keyframe track.
    std::vector<KeyScale>    scales;    ///< Scale keyframe track.

    /**
     * @brief Evaluates the combined TRS matrix at the given animation time.
     * @param animTime  Time in animation ticks.
     * @return Interpolated local-space bone transform.
     */
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

// ─────────────────────────────────────────────
//  Animation clip
// ─────────────────────────────────────────────

/// @brief One named animation clip as imported from the source asset.
class Animation {
public:
    std::string name;                                        ///< Clip name (e.g. "Walk", "Idle").
    float       duration       = 0.0f;                      ///< Total duration in animation ticks.
    float       ticksPerSecond = 25.0f;                     ///< Ticks-per-second rate from the source file.

    std::unordered_map<std::string, BoneChannel> channels;  ///< Per-bone keyframe tracks, keyed by bone name.

    float durationSeconds() const { return duration / ticksPerSecond; }         ///< @return Clip duration in seconds.
    float toTicks(float seconds) const { return seconds * ticksPerSecond; }     ///< @return Converts seconds to animation ticks.
};

// ─────────────────────────────────────────────
//  Animator — drives bone transforms
// ─────────────────────────────────────────────

/// @brief Runtime animation player: evaluates a clip each frame and produces bone transforms.
class Animator {
public:
    static constexpr int MAX_BONES = 100; ///< Maximum number of bones per skeleton.

    /**
     * @brief Advances the playback time and recomputes the bone transform palette.
     * @param deltaTime  Seconds elapsed since the last frame.
     * @param skeleton   The skeleton whose bind pose is used as the base.
     */
    void update(float deltaTime, const Skeleton& skeleton);

    /**
     * @brief Starts playback of an animation clip.
     * @param anim  Clip to play; must remain valid for the lifetime of the Animator.
     * @param loop  Whether to loop the clip when it reaches the end.
     */
    void play(const Animation* anim, bool loop = true);

    /**
     * @brief Cross-fades to a new animation clip over the given duration.
     * @param anim           Target clip.
     * @param blendDuration  Blend-in time in seconds.
     * @param loop           Whether to loop the target clip.
     */
    void crossFadeTo(const Animation* anim, float blendDuration, bool loop = true);

    /// @return The current bone transform palette (one mat4 per bone, up to @ref MAX_BONES).
    const std::vector<glm::mat4>& getBoneTransforms() const { return m_boneTransforms; }

    /**
     * @brief Recursively evaluates the skeleton hierarchy and fills the transform palette.
     * @param node             Current skeleton node.
     * @param parentTransform  Accumulated parent world transform.
     * @param skeleton         Skeleton definition.
     * @param animTime         Current animation time in ticks.
     */
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