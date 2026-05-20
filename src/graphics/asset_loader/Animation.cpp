#include "Animation.hpp"

#include <algorithm>
#include <cmath>

void Animator::play(const Animation* anim, bool loop)
{
    m_current     = anim;
    m_next        = nullptr;
    m_time        = 0.0f;
    m_blendFactor = 0.0f;
    m_blendDur    = 0.0f;
    m_loop        = loop;
}

void Animator::crossFadeTo(const Animation* anim, float blendDuration, bool loop)
{
    (void)blendDuration;
    play(anim, loop);
}

void Animator::update(float deltaTime, const Skeleton& skeleton)
{
    if (!m_current) return;

    m_time += m_current->toTicks(deltaTime);

    if (m_loop && m_current->duration > 0.0f) {
        m_time = std::fmod(m_time, m_current->duration);
        if (m_time < 0.0f)
            m_time += m_current->duration;
    } else if (!m_loop && m_time > m_current->duration) {
        m_time = m_current->duration;
    }

    std::fill(m_boneTransforms.begin(), m_boneTransforms.end(), glm::mat4(1.0f));
    computeTransforms(skeleton.rootNode, glm::mat4(1.0f), skeleton, m_time);
}

void Animator::computeTransforms(const SkeletonNode& node,
                                 const glm::mat4&    parentTransform,
                                 const Skeleton&     skeleton,
                                 float               animTime)
{
    glm::mat4 local = node.localTransform;
    if (m_current) {
        auto it = m_current->channels.find(node.name);
        if (it != m_current->channels.end())
            local = it->second.interpolate(animTime);
    }

    const glm::mat4 global = parentTransform * local;

    if (skeleton.hasBone(node.name)) {
        const int id = skeleton.getBoneID(node.name);
        if (id >= 0 && id < MAX_BONES) {
            const glm::mat4& offset = skeleton.boneMap.at(node.name).offsetMatrix;
            m_boneTransforms[static_cast<size_t>(id)] =
                skeleton.globalInverseTransform * global * offset;
        }
    }

    for (const auto& child : node.children)
        computeTransforms(child, global, skeleton, animTime);
}

glm::mat4 Animator::sampleChannel(const Animation* anim, const std::string& bone, float t) const
{
    if (!anim) return glm::mat4(1.0f);
    auto it = anim->channels.find(bone);
    if (it == anim->channels.end()) return glm::mat4(1.0f);
    return it->second.interpolate(t);
}
