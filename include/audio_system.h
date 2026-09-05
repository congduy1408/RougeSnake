#pragma once

#include <raylib.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

enum class MusicId {
    None,
    MainMenu,
    Gameplay,
    Boss
};

enum class SoundEffectId {
    FoodEaten,
    ItemCollected,
    RockImpact,
    Count
};

enum class AudioEventType : std::uint8_t {
    NoteOff,
    NoteOn,
    ProgramChange
};

struct AudioNoteEvent {
    std::uint32_t sample_position;
    std::uint8_t type;
    std::uint8_t channel;
    std::uint8_t note;
    std::uint8_t value;
};

struct AudioClip {
    const AudioNoteEvent* events;
    std::size_t event_count;
    std::uint32_t length_samples;
};

class AudioSystem {
    public:
        bool music_enabled = true;
        bool sound_effects_enabled = true;
        float music_volume = 0.18f;
        float sound_effects_volume = 0.65f;

        bool Initialize();
        void Shutdown();
        void PlayMusic(MusicId music, bool loop = true);
        void StopMusic();
        void SetMusicEnabled(bool enabled);
        void SetSoundEffectsEnabled(bool enabled);
        void SetMusicVolume(float volume);
        void SetSoundEffectVolume(float volume);
        void PlaySoundEffect(SoundEffectId effect);

    private:
        enum class Waveform : std::uint8_t {
            Square,
            Triangle,
            Pulse,
            Noise
        };

        struct Voice {
            bool active = false;
            bool releasing = false;
            std::uint8_t note = 0;
            std::uint8_t channel = 0;
            Waveform waveform = Waveform::Square;
            float phase = 0.0f;
            float frequency = 0.0f;
            float volume = 0.0f;
            float envelope = 0.0f;
            std::uint32_t noise_state = 1;
        };

        static constexpr unsigned int sample_rate = 22050;
        static constexpr std::size_t voice_count = 8;
        static constexpr std::size_t sound_voice_count = 4;

        AudioStream music_stream = {};
        std::array<Sound, static_cast<std::size_t>(SoundEffectId::Count)> sounds = {};
        std::array<std::array<Sound, sound_voice_count>,
                   static_cast<std::size_t>(SoundEffectId::Count)> sound_voices = {};
        std::array<std::size_t, static_cast<std::size_t>(SoundEffectId::Count)>
            next_sound_voice = {};
        std::array<Voice, voice_count> voices = {};
        std::array<Waveform, 16> channel_waveforms = {};
        const AudioClip* current_clip = nullptr;
        std::size_t next_event_index = 0;
        std::uint32_t music_sample_position = 0;
        std::atomic<int> requested_music{static_cast<int>(MusicId::None)};
        std::atomic<bool> requested_loop{true};
        std::atomic<bool> requested_music_enabled{true};
        std::atomic<std::uint32_t> request_version{0};
        std::uint32_t applied_request_version = 0;
        bool current_loop = true;
        bool initialized = false;

        static std::atomic<AudioSystem*> callback_instance;
        static void MusicCallback(void* buffer_data, unsigned int frames);
        void RenderMusic(short* samples, unsigned int frames);
        void ApplyPendingMusicRequest();
        void ApplyEvent(const AudioNoteEvent& event);
        void StartVoice(std::uint8_t channel, std::uint8_t note, std::uint8_t velocity);
        void ReleaseVoice(std::uint8_t channel, std::uint8_t note);
        void ReleaseAllVoices();
        float RenderVoices();
        void GenerateSoundEffects();
        void StopSoundEffects();
        const AudioClip* GetClip(MusicId music) const;
};
