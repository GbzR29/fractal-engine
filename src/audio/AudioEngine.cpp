#include "AudioEngine.hpp"
#include "SceneEntity.hpp"
#include <iostream>

AudioEngine::AudioEngine()  = default;
AudioEngine::~AudioEngine() { Shutdown(); }

bool AudioEngine::Init()
{
    m_Ready = false;
    std::cout << "[Audio] Engine inicializado (stub — sem backend ativo).\n";
    return true;
}

void AudioEngine::Shutdown()
{
    m_Ready = false;
}

void AudioEngine::Update(std::vector<std::unique_ptr<SceneEntity>>& entities)
{
    if (!m_Ready) return;
    for (auto& e : entities) {
        if (!e->HasAudioListener() || !e->AudioListener->Enabled) continue;
        // stub: listener position/orientation would be sent to audio backend here
        break;
    }
    for (auto& e : entities) {
        if (!e->HasAudioSource()) continue;
        auto& src = *e->AudioSource;
        if (src.PlayOnStart && !src.Playing && !src._startedOnce) {
            src.Playing = true;
            src._startedOnce = true;
            std::cout << "[Audio] PlayOnStart: " << src.soundPath << " (stub)\n";
        }
    }
}

void AudioEngine::Play(SceneEntity& e)
{
    if (!e.HasAudioSource()) return;
    e.AudioSource->Playing = true;
    std::cout << "[Audio] Play: " << e.AudioSource->soundPath << " (stub)\n";
}

void AudioEngine::Stop(SceneEntity& e)
{
    if (!e.HasAudioSource()) return;
    e.AudioSource->Playing = false;
}

void AudioEngine::Pause(SceneEntity& e)
{
    if (!e.HasAudioSource()) return;
    e.AudioSource->Playing = false;
}

void AudioEngine::SetMasterVolume(float v)
{
    m_MasterVolume = v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v;
}
