#pragma once

#include "Common.h"

#include "FileManager.h"
#include <SDL2/SDL.h>

namespace NGame {

class TAudioManager {
public:
    TAudioManager(TFileManager& fileManager);
    DELETE_COPY(TAudioManager)

    void Run();
    void Play(const std::string& sound);
    void Paint(SDL_AudioStream* stream, std::uint32_t ticks);
    void SetVolume(float volume);
    float Volume() const;

private:
    struct TSound {
        SDL_AudioSpec Spec;
        std::uint8_t* Data;
        std::uint32_t Size;
    };

    struct TSoundChannel {
        bool InUse;
        TSound* Sound;
        std::uint32_t Position;
    };

private:
    TSound* Load(const std::string& sound);

private:
    TFileManager& FileManager_;
    float Volume_ = 1.0;
    TSoundChannel Channels_[16];
    std::unordered_map<std::string, TSound*> Sounds_;
    SDL_AudioDeviceID Device_;
    SDL_AudioStream* Stream_;
    std::uint32_t LastTick_;
};

} // namespace NGame
