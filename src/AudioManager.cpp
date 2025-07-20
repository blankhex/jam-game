#include "AudioManager.h"
#include <cmath>

namespace NGame {

TAudioManager::TAudioManager(TFileManager& fileManager) 
    : FileManager_(fileManager) {

    SDL_AudioSpec desired;
    desired.freq = 48000;
    desired.format = AUDIO_F32;
    desired.channels = 2;
    desired.samples = 2048;
    desired.callback = NULL;

    Device_ = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);
    if (!Device_) {
        throw std::runtime_error("no audio device");
    }

    Stream_ = SDL_NewAudioStream(AUDIO_U8, 1, 22050, AUDIO_F32, 2, 48000);
    if (!Stream_) {
        throw std::runtime_error("can't create audio stream");
    }

    for (std::size_t i = 0; i < 16; ++i) {
        Channels_[i].InUse = false;
    }
    SDL_PauseAudioDevice(Device_, SDL_FALSE);

    LastTick_ = SDL_GetTicks();
}

void TAudioManager::Run() {
    std::uint32_t currentTick = SDL_GetTicks();

    Paint(Stream_, currentTick - LastTick_);
    if (SDL_GetQueuedAudioSize(Device_) < 8192) {
        const int bytesRemaining = SDL_AudioStreamAvailable(Stream_);
        if (bytesRemaining > 0) {
            const int samplesRemaining = SDL_min(bytesRemaining, 32 * 1024);
            static Uint8 convertedBuffer[32 * 1024];
            auto bytesRead = SDL_AudioStreamGet(Stream_, convertedBuffer, samplesRemaining);
            SDL_QueueAudio(Device_, convertedBuffer, bytesRead);
        }
    }

    LastTick_ = currentTick;
}

void TAudioManager::Play(const std::string& path) {
    TSound* sound = Load(path);

    for (std::size_t i = 0; i < 16; i++) {
        if (Channels_[i].InUse) {
            continue;
        }

        Channels_[i].InUse = true;
        Channels_[i].Position = 0;
        Channels_[i].Sound = sound;
        break;
    }
}

void TAudioManager::Paint(SDL_AudioStream* stream, std::uint32_t ticks) {
    std::uint8_t temporary[1024];
    std::size_t samples = std::min((ticks * 22050 + 999) / 1000, 1024u);

    // Mix
    for (size_t i = 0; i < samples; ++i) {
        int16_t accumulator = 0;

        for (size_t j = 0; j < 16; j++) {
            auto& channel = Channels_[j];
            if (!channel.InUse) {
                continue;
            }
            
            accumulator += channel.Sound->Data[channel.Position];
            accumulator -= 128;
            channel.Position++;

            if (channel.Position > channel.Sound->Size) {
                channel.InUse = false;
            }
        }

        accumulator *= Volume_;

        accumulator += 128;
        if (accumulator < 0) {
            accumulator = 0;
        } 
        if (accumulator > 255) {
            accumulator = 255;
        }

        temporary[i] = accumulator;
    }

    // Output
    if (samples > 0) {
        SDL_AudioStreamPut(stream, temporary, samples * sizeof (std::uint8_t));
    }
}

void TAudioManager::SetVolume(float volume) {
    if (volume > 1) {
        volume = 1;
    } else if (volume < 0) {
        volume = 0;
    }
    Volume_ = volume;
}

float TAudioManager::Volume() const {
    return Volume_;
}

TAudioManager::TSound* TAudioManager::Load(const std::string& path) {
    auto it = Sounds_.find(path);
    if (it != Sounds_.end()) {
        return it->second;
    }

    TSound* newSound = new TSound;
    auto data = FileManager_.Get(path);
    SDL_RWops* rw = SDL_RWFromConstMem(data.data(), data.size());
    if (!SDL_LoadWAV_RW(rw, 1, &newSound->Spec, &newSound->Data, &newSound->Size)) {
        throw std::runtime_error("can't read WAV file " + path);
    }

    if (newSound->Spec.channels != 1) {
        throw std::runtime_error("not mono WAV file " + path);
    }

    if (newSound->Spec.format != AUDIO_U8) {
        throw std::runtime_error("not 8bit pcm WAV file " + path);
    }

    if (newSound->Spec.freq != 22050) {
        throw std::runtime_error("not 22050hz WAV file " + path);
    }

    Sounds_[path] = newSound;
    return newSound;
}

} // namespace NGame