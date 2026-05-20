/**
 * @file AudioEngine.hpp
 * @brief Audio engine stub — full API surface, no-op backend until SDL3/miniaudio is wired in.
 *
 * All Play/Stop/Pause operations update the flags on @ref AudioSourceComponent but do not
 * produce actual sound yet.  The @ref Application calls @ref Update every frame so that
 * PlayOnStart logic and future positional audio updates work correctly once a real backend
 * is integrated.
 */
#pragma once
#include <string>
#include <vector>
#include <memory>

struct SceneEntity;

/**
 * @brief Central audio controller owned by @ref Application.
 *
 * Lifetime: Init() → Update() each frame → Shutdown().
 */
class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();

    /**
     * @brief Initialises the audio backend.
     * @return @c true on success (stub always returns @c true).
     */
    bool Init();

    /// Releases all audio resources.
    void Shutdown();

    /**
     * @brief Per-frame update — handles PlayOnStart and listener repositioning.
     * @param entities All entities in the active scene.
     */
    void Update(std::vector<std::unique_ptr<SceneEntity>>& entities);

    /**
     * @brief Starts playback of the audio source on @p e.
     * @param e Entity that owns an @ref AudioSourceComponent.
     */
    void Play(SceneEntity& e);

    /**
     * @brief Stops playback and resets playback position on @p e.
     * @param e Entity that owns an @ref AudioSourceComponent.
     */
    void Stop(SceneEntity& e);

    /**
     * @brief Pauses playback without resetting position on @p e.
     * @param e Entity that owns an @ref AudioSourceComponent.
     */
    void Pause(SceneEntity& e);

    /**
     * @brief Sets the master output volume.
     * @param v Volume scalar in [0, 1].
     */
    void SetMasterVolume(float v);

    /// @return Current master volume scalar.
    float GetMasterVolume() const { return m_MasterVolume; }

    /// @return @c true if the audio backend initialised successfully.
    bool IsReady() const { return m_Ready; }

    /// @return Human-readable name of the active backend (e.g. "None (stub)").
    const std::string& GetBackendName() const { return m_BackendName; }

    /// @return Last error message, or an empty string if none.
    const std::string& GetLastError()   const { return m_LastError;   }

private:
    bool        m_Ready        = false;
    float       m_MasterVolume = 1.0f;
    std::string m_BackendName  = "None (stub)";
    std::string m_LastError;
};
