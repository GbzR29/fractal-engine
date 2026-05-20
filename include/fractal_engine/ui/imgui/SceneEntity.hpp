/**
 * @file SceneEntity.hpp
 * @brief Scene entity and all component structs used by the editor and renderer.
 *
 * Components are stored as @c std::unique_ptr members inside @ref SceneEntity;
 * a @c nullptr pointer means the component is absent.  Use the @c HasXxx() helpers
 * before accessing a component pointer.
 *
 * Component overview:
 * - @ref TransformComponent  — position / rotation (Euler°) / scale
 * - @ref CameraComponent     — perspective or orthographic projection
 * - @ref TagComponent        — human-readable name and tag string
 * - @ref DirectionalLightComponent — sun-like light (direction from Transform.Rotation)
 * - @ref AudioListenerComponent    — marks the entity that hears spatial audio
 * - @ref AudioSourceComponent      — plays a sound file from this entity's position
 * - @ref ScriptComponent           — attaches a Lua script to the entity
 * - @ref MeshRendererComponent     — GPU mesh + material loaded via @ref AssetLoader
 */
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Model;
class Animator;

// ─────────────────────────────────────────────────────────────────────────────
//  Basic components
// ─────────────────────────────────────────────────────────────────────────────

/// @brief World-space transform: position, Euler-degree rotation, and scale.
struct TransformComponent
{
    glm::vec3 Position = { 0.0f, 0.0f, 0.0f }; ///< World-space position.
    glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f }; ///< Euler angles in degrees (X=pitch, Y=yaw, Z=roll).
    glm::vec3 Scale    = { 1.0f, 1.0f, 1.0f }; ///< Non-uniform scale.

    /// @brief Computes the TRS model matrix from the current position/rotation/scale.
    /// @return 4×4 model-to-world transformation matrix.
    glm::mat4 GetMatrix() const
    {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), Position);
        glm::mat4 r = glm::rotate(glm::mat4(1.0f),
            glm::radians(Rotation.x), { 1, 0, 0 });
        r = glm::rotate(r, glm::radians(Rotation.y), { 0, 1, 0 });
        r = glm::rotate(r, glm::radians(Rotation.z), { 0, 0, 1 });
        glm::mat4 s = glm::scale(glm::mat4(1.0f), Scale);
        return t * r * s;
    }
};

/// @brief Perspective or orthographic camera.  The entity's @ref TransformComponent supplies
///        the view position and orientation.
struct CameraComponent
{
    float FOV             = 60.0f;   ///< Vertical field of view in degrees (perspective only).
    float Near            = 0.1f;    ///< Near clipping plane distance.
    float Far             = 1000.0f; ///< Far clipping plane distance.
    bool  IsPrimary       = true;    ///< When @c true, this camera is used by the GameViewport.
    bool  IsOrthographic  = false;   ///< Use orthographic projection instead of perspective.

    /**
     * @brief Builds the projection matrix.
     * @param aspectRatio  Viewport width divided by height.
     * @return Projection matrix suitable for use in a vertex shader.
     */
    glm::mat4 GetProjection(float aspectRatio) const
    {
        if (IsOrthographic)
            return glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, Near, Far);
        return glm::perspective(glm::radians(FOV), aspectRatio, Near, Far);
    }
};

/// @brief Human-readable identification attached to every entity.
struct TagComponent
{
    std::string Name = "Entity";   ///< Display name shown in the hierarchy panel.
    std::string Tag  = "Untagged"; ///< Gameplay tag used for categorisation / queries.
};

// ─────────────────────────────────────────────────────────────────────────────
//  DirectionalLight component
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Sun-like directional light.  Direction is derived from the entity's
///        @ref TransformComponent::Rotation (X = elevation, Y = azimuth).
struct DirectionalLightComponent
{
    glm::vec3 Color     = { 1.0f, 0.95f, 0.85f }; ///< Linear-space light colour.
    float     Intensity = 3.0f;                    ///< Brightness multiplier.
};

// ─────────────────────────────────────────────────────────────────────────────
//  Audio components
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Marks the entity as the audio listener (the "ears" of the scene).
///        Only one listener should be active at a time.
struct AudioListenerComponent
{
    bool  Enabled = true;  ///< Whether this listener is active.
    float Volume  = 1.0f;  ///< Listener-side volume scale.
    bool  Muted   = false; ///< Mutes all audio output when @c true.
};

/// @brief Attaches a sound source to the entity's world position.
struct AudioSourceComponent
{
    std::string soundPath;            ///< Relative path to the audio file (e.g. "sounds/pop.wav").
    float Volume       = 1.0f;        ///< Playback volume in [0, 1].
    float Pitch        = 1.0f;        ///< Pitch multiplier (1.0 = original pitch).
    float Range        = 50.0f;       ///< Maximum audible distance for 3-D sources.
    bool  Loop         = false;       ///< Loop playback continuously.
    bool  PlayOnStart  = false;       ///< Automatically start playback when the scene enters Play mode.
    bool  Is3D         = true;        ///< Use distance-based attenuation.
    bool  Playing      = false;       ///< Current playback state (read/write by @ref AudioEngine).
    bool  _startedOnce = false;       ///< Internal: prevents PlayOnStart from triggering more than once.
};

// ─────────────────────────────────────────────────────────────────────────────
//  Script component (Lua)
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Lua script attached to the entity.  Managed by @ref LuaScriptEngine.
struct ScriptComponent
{
    std::string scriptPath; ///< Asset-relative path (e.g. "scripts/player.lua").
    std::string lastError;  ///< Last compile or runtime error message.
    bool        loaded = false; ///< @c true when the script compiled without errors.
};

// ─────────────────────────────────────────────────────────────────────────────
//  MeshRenderer component
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Renders a 3-D mesh with PBR materials.  Loaded via @ref AssetLoader::loadModel().
struct MeshRendererComponent
{
    std::string               modelPath;            ///< Asset-relative (or absolute) path used to reload the model.
    std::shared_ptr<Model>    model;                ///< GPU-ready model; @c nullptr until loaded.
    std::shared_ptr<Animator> animator;             ///< Active animator; @c nullptr for static meshes.
    bool                      isPrimitive = false;  ///< @c true for procedural primitives (cube/sphere/plane).

    /// @return @c true when the model has been loaded and is ready to render.
    bool HasModel() const { return model != nullptr; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  SceneEntity — scene graph node
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief A node in the scene hierarchy.
 *
 * Every entity always has a @ref TagComponent and a @ref TransformComponent.
 * All other components are optional and stored as @c unique_ptr (nullptr = absent).
 * Children form a flat tree; they are not yet used for transform inheritance.
 */
struct SceneEntity
{
    uint32_t ID       = 0;     ///< Unique monotonically increasing ID assigned at construction.
    bool     Active   = true;  ///< Inactive entities are skipped by renderers and scripts.
    bool     Selected = false; ///< @c true when selected in the hierarchy panel.
    bool     Open     = false; ///< Tree-node expanded state in the hierarchy panel.

    TagComponent       Tag;
    TransformComponent Transform;

    /// @name Optional components
    /// @{
    std::unique_ptr<CameraComponent>           Camera;
    std::unique_ptr<MeshRendererComponent>     MeshRenderer;
    std::unique_ptr<DirectionalLightComponent> DirectionalLight;
    std::unique_ptr<ScriptComponent>           Script;
    std::unique_ptr<AudioListenerComponent>    AudioListener;
    std::unique_ptr<AudioSourceComponent>      AudioSource;
    /// @}

    bool HasCamera()           const { return Camera           != nullptr; } ///< @return @c true if a CameraComponent is attached.
    bool HasMeshRenderer()     const { return MeshRenderer     != nullptr; } ///< @return @c true if a MeshRendererComponent is attached.
    bool HasDirectionalLight() const { return DirectionalLight != nullptr; } ///< @return @c true if a DirectionalLightComponent is attached.
    bool HasScript()           const { return Script           != nullptr; } ///< @return @c true if a ScriptComponent is attached.
    bool HasAudioListener()    const { return AudioListener    != nullptr; } ///< @return @c true if an AudioListenerComponent is attached.
    bool HasAudioSource()      const { return AudioSource      != nullptr; } ///< @return @c true if an AudioSourceComponent is attached.

    /// Child entities (future support for transform hierarchy).
    std::vector<std::unique_ptr<SceneEntity>> Children;

    /**
     * @brief Constructs an entity with the given display name.
     * @param name  Initial value for @ref TagComponent::Name.
     */
    explicit SceneEntity(const std::string& name = "Entity")
    {
        static uint32_t s_NextID = 1;
        ID       = s_NextID++;
        Tag.Name = name;
    }
};