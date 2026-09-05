#include "include/audio_system.h"

#include "include/generated_audio_data.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {
constexpr float pi = 3.14159265358979323846f;

float ClampUnit(float value) {
    return std::max(-1.0f, std::min(value, 1.0f));
}

float MidiFrequency(std::uint8_t note) {
    return 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
}

float ShortEnvelope(int sample, int note_samples, int attack_samples, int release_samples) {
    if (sample < attack_samples) {
        return static_cast<float>(sample) / std::max(attack_samples, 1);
    }
    int release_start = note_samples - release_samples;
    if (sample >= release_start) {
        return static_cast<float>(note_samples - sample) / std::max(release_samples, 1);
    }
    return 1.0f;
}

Sound LoadPcmSound(std::vector<short>& samples, unsigned int sample_rate) {
    Wave wave = {};
    wave.frameCount = static_cast<unsigned int>(samples.size());
    wave.sampleRate = sample_rate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples.data();
    return LoadSoundFromWave(wave);
}

Sound GenerateFoodSound(unsigned int sample_rate) {
    constexpr float frequencies[] = {783.99f, 659.25f, 523.25f, 1046.50f};
    constexpr float durations[] = {0.055f, 0.055f, 0.065f, 0.055f};
    int total_samples = static_cast<int>(0.230f * sample_rate);
    std::vector<short> samples(total_samples, 0);
    int cursor = 0;
    for (int note = 0; note < 4; note++) {
        int note_samples = static_cast<int>(durations[note] * sample_rate);
        int edge_samples = static_cast<int>(0.003f * sample_rate);
        for (int i = 0; i < note_samples && cursor + i < total_samples; i++) {
            float phase = frequencies[note] * i / sample_rate;
            float square = std::fmod(phase, 1.0f) < 0.5f ? 1.0f : -1.0f;
            float value = square * ShortEnvelope(i, note_samples, edge_samples, edge_samples);
            samples[cursor + i] = static_cast<short>(value * 15000.0f);
        }
        cursor += note_samples;
    }
    return LoadPcmSound(samples, sample_rate);
}

Sound GenerateItemSound(unsigned int sample_rate) {
    constexpr float frequencies[] = {523.25f, 659.25f, 783.99f};
    int note_samples = static_cast<int>(0.055f * sample_rate);
    std::vector<short> samples(note_samples * 3, 0);
    int edge_samples = static_cast<int>(0.004f * sample_rate);
    for (int note = 0; note < 3; note++) {
        for (int i = 0; i < note_samples; i++) {
            float phase = std::fmod(frequencies[note] * i / sample_rate, 1.0f);
            float triangle = 1.0f - 4.0f * std::fabs(phase - 0.5f);
            float value = triangle * ShortEnvelope(i, note_samples, edge_samples, edge_samples);
            samples[note * note_samples + i] = static_cast<short>(value * 17000.0f);
        }
    }
    return LoadPcmSound(samples, sample_rate);
}

Sound GenerateRockSound(unsigned int sample_rate) {
    int total_samples = static_cast<int>(0.24f * sample_rate);
    std::vector<short> samples(total_samples, 0);
    std::uint32_t noise_state = 0x31415926u;
    float phase = 0.0f;
    for (int i = 0; i < total_samples; i++) {
        float progress = static_cast<float>(i) / total_samples;
        noise_state ^= noise_state << 13;
        noise_state ^= noise_state >> 17;
        noise_state ^= noise_state << 5;
        float noise = (static_cast<int>(noise_state & 0xffffu) - 32768) / 32768.0f;
        float frequency = 115.0f - 60.0f * progress;
        phase = std::fmod(phase + frequency / sample_rate, 1.0f);
        float triangle = 1.0f - 4.0f * std::fabs(phase - 0.5f);
        float envelope = (1.0f - progress) * (1.0f - progress);
        float value = (noise * 0.68f + triangle * 0.55f) * envelope;
        samples[i] = static_cast<short>(ClampUnit(value) * 23000.0f);
    }
    return LoadPcmSound(samples, sample_rate);
}
}

constexpr unsigned int AudioSystem::sample_rate;
constexpr std::size_t AudioSystem::voice_count;
constexpr std::size_t AudioSystem::sound_voice_count;
std::atomic<AudioSystem*> AudioSystem::callback_instance{nullptr};

bool AudioSystem::Initialize() {
    if (initialized || !IsAudioDeviceReady()) {
        return initialized;
    }

    GenerateSoundEffects();
    SetAudioStreamBufferSizeDefault(1024);
    music_stream = LoadAudioStream(sample_rate, 16, 1);
    if (!IsAudioStreamValid(music_stream)) {
        for (std::size_t effect = 0; effect < sound_voices.size(); effect++) {
            for (Sound& voice : sound_voices[effect]) {
                if (voice.frameCount > 0) {
                    UnloadSoundAlias(voice);
                    voice = {};
                }
            }
        }
        for (Sound& sound : sounds) {
            if (sound.frameCount > 0) {
                UnloadSound(sound);
                sound = {};
            }
        }
        return false;
    }
    callback_instance.store(this);
    SetAudioStreamCallback(music_stream, MusicCallback);
    SetAudioStreamVolume(music_stream, music_volume);
    requested_music_enabled.store(music_enabled);
    initialized = true;
    PlayAudioStream(music_stream);
    return true;
}

void AudioSystem::Shutdown() {
    if (!initialized) {
        return;
    }

    requested_music_enabled.store(false);
    StopAudioStream(music_stream);
    callback_instance.store(nullptr);
    UnloadAudioStream(music_stream);
    StopSoundEffects();
    for (std::size_t effect = 0; effect < sound_voices.size(); effect++) {
        for (Sound& voice : sound_voices[effect]) {
            if (voice.frameCount > 0) {
                UnloadSoundAlias(voice);
                voice = {};
            }
        }
    }
    for (Sound& sound : sounds) {
        if (sound.frameCount > 0) {
            UnloadSound(sound);
            sound = {};
        }
    }
    ReleaseAllVoices();
    initialized = false;
}

void AudioSystem::PlayMusic(MusicId music, bool loop) {
    requested_music.store(static_cast<int>(music));
    requested_loop.store(loop);
    request_version.fetch_add(1);
}

void AudioSystem::StopMusic() {
    PlayMusic(MusicId::None, false);
}

void AudioSystem::SetMusicEnabled(bool enabled) {
    music_enabled = enabled;
    requested_music_enabled.store(enabled);
    request_version.fetch_add(1);
}

void AudioSystem::SetSoundEffectsEnabled(bool enabled) {
    sound_effects_enabled = enabled;
    if (!enabled) {
        StopSoundEffects();
    }
}

void AudioSystem::SetMusicVolume(float volume) {
    music_volume = std::max(0.0f, std::min(volume, 1.0f));
    if (initialized) {
        SetAudioStreamVolume(music_stream, music_volume);
    }
}

void AudioSystem::SetSoundEffectVolume(float volume) {
    sound_effects_volume = std::max(0.0f, std::min(volume, 1.0f));
    for (auto& effect_voices : sound_voices) {
        for (Sound& voice : effect_voices) {
            if (voice.frameCount > 0) {
                SetSoundVolume(voice, sound_effects_volume);
            }
        }
    }
}

void AudioSystem::PlaySoundEffect(SoundEffectId effect) {
    std::size_t index = static_cast<std::size_t>(effect);
    if (!initialized || !sound_effects_enabled || index >= sounds.size() ||
        sounds[index].frameCount == 0) {
        return;
    }
    std::size_t selected = next_sound_voice[index];
    for (std::size_t offset = 0; offset < sound_voice_count; offset++) {
        std::size_t candidate = (selected + offset) % sound_voice_count;
        if (!IsSoundPlaying(sound_voices[index][candidate])) {
            selected = candidate;
            break;
        }
    }
    StopSound(sound_voices[index][selected]);
    PlaySound(sound_voices[index][selected]);
    next_sound_voice[index] = (selected + 1) % sound_voice_count;
}

void AudioSystem::MusicCallback(void* buffer_data, unsigned int frames) {
    short* samples = static_cast<short*>(buffer_data);
    AudioSystem* instance = callback_instance.load();
    if (instance == nullptr) {
        std::fill(samples, samples + frames, 0);
        return;
    }
    instance->RenderMusic(samples, frames);
}

void AudioSystem::RenderMusic(short* samples, unsigned int frames) {
    ApplyPendingMusicRequest();
    for (unsigned int frame = 0; frame < frames; frame++) {
        if (current_clip != nullptr && requested_music_enabled.load()) {
            while (next_event_index < current_clip->event_count &&
                   current_clip->events[next_event_index].sample_position <= music_sample_position) {
                ApplyEvent(current_clip->events[next_event_index]);
                next_event_index++;
            }
        }

        samples[frame] = static_cast<short>(ClampUnit(RenderVoices() * 0.28f) * 32767.0f);
        if (current_clip == nullptr || !requested_music_enabled.load()) {
            continue;
        }

        music_sample_position++;
        if (music_sample_position >= current_clip->length_samples) {
            ReleaseAllVoices();
            if (current_loop) {
                music_sample_position = 0;
                next_event_index = 0;
            } else {
                current_clip = nullptr;
            }
        }
    }
}

void AudioSystem::ApplyPendingMusicRequest() {
    std::uint32_t version = request_version.load();
    if (version == applied_request_version) {
        return;
    }

    ReleaseAllVoices();
    MusicId music = static_cast<MusicId>(requested_music.load());
    current_clip = requested_music_enabled.load() ? GetClip(music) : nullptr;
    current_loop = requested_loop.load();
    music_sample_position = 0;
    next_event_index = 0;
    applied_request_version = version;
}

void AudioSystem::ApplyEvent(const AudioNoteEvent& event) {
    AudioEventType type = static_cast<AudioEventType>(event.type);
    if (type == AudioEventType::NoteOn && event.value > 0) {
        StartVoice(event.channel, event.note, event.value);
    } else if (type == AudioEventType::NoteOff ||
               (type == AudioEventType::NoteOn && event.value == 0)) {
        ReleaseVoice(event.channel, event.note);
    } else if (type == AudioEventType::ProgramChange && event.channel < channel_waveforms.size()) {
        if (event.value >= 112) {
            channel_waveforms[event.channel] = Waveform::Noise;
        } else if (event.value >= 80) {
            channel_waveforms[event.channel] = Waveform::Pulse;
        } else if (event.value >= 40) {
            channel_waveforms[event.channel] = Waveform::Triangle;
        } else {
            channel_waveforms[event.channel] = Waveform::Square;
        }
    }
}

void AudioSystem::StartVoice(std::uint8_t channel, std::uint8_t note, std::uint8_t velocity) {
    Voice* selected = nullptr;
    for (Voice& voice : voices) {
        if (!voice.active) {
            selected = &voice;
            break;
        }
    }
    if (selected == nullptr) {
        selected = &voices[0];
        for (Voice& voice : voices) {
            if (voice.envelope < selected->envelope) {
                selected = &voice;
            }
        }
    }

    selected->active = true;
    selected->releasing = false;
    selected->note = note;
    selected->channel = channel;
    selected->waveform = channel < channel_waveforms.size()
        ? channel_waveforms[channel]
        : Waveform::Square;
    selected->phase = 0.0f;
    selected->frequency = MidiFrequency(note);
    selected->volume = static_cast<float>(velocity) / 127.0f;
    selected->envelope = 0.0f;
    selected->noise_state = 0x9e3779b9u ^ (note << 8) ^ channel;
}

void AudioSystem::ReleaseVoice(std::uint8_t channel, std::uint8_t note) {
    for (Voice& voice : voices) {
        if (voice.active && voice.channel == channel && voice.note == note) {
            voice.releasing = true;
        }
    }
}

void AudioSystem::ReleaseAllVoices() {
    for (Voice& voice : voices) {
        if (voice.active) {
            voice.releasing = true;
        }
    }
}

float AudioSystem::RenderVoices() {
    float mixed = 0.0f;
    constexpr float attack_step = 1.0f / (sample_rate * 0.004f);
    constexpr float release_step = 1.0f / (sample_rate * 0.018f);
    for (Voice& voice : voices) {
        if (!voice.active) {
            continue;
        }

        if (voice.releasing) {
            voice.envelope -= release_step;
            if (voice.envelope <= 0.0f) {
                voice = Voice{};
                continue;
            }
        } else {
            voice.envelope = std::min(1.0f, voice.envelope + attack_step);
        }

        float wave = 0.0f;
        if (voice.waveform == Waveform::Square) {
            wave = voice.phase < 0.5f ? 1.0f : -1.0f;
        } else if (voice.waveform == Waveform::Triangle) {
            wave = 1.0f - 4.0f * std::fabs(voice.phase - 0.5f);
        } else if (voice.waveform == Waveform::Pulse) {
            wave = voice.phase < 0.25f ? 1.0f : -1.0f;
        } else {
            voice.noise_state ^= voice.noise_state << 13;
            voice.noise_state ^= voice.noise_state >> 17;
            voice.noise_state ^= voice.noise_state << 5;
            wave = (static_cast<int>(voice.noise_state & 0xffffu) - 32768) / 32768.0f;
        }

        mixed += wave * voice.volume * voice.envelope;
        voice.phase = std::fmod(voice.phase + voice.frequency / sample_rate, 1.0f);
    }
    return mixed;
}

void AudioSystem::GenerateSoundEffects() {
    sounds[static_cast<std::size_t>(SoundEffectId::FoodEaten)] = GenerateFoodSound(sample_rate);
    sounds[static_cast<std::size_t>(SoundEffectId::ItemCollected)] = GenerateItemSound(sample_rate);
    sounds[static_cast<std::size_t>(SoundEffectId::RockImpact)] = GenerateRockSound(sample_rate);
    for (std::size_t effect = 0; effect < sounds.size(); effect++) {
        if (sounds[effect].frameCount == 0) {
            continue;
        }
        for (Sound& voice : sound_voices[effect]) {
            voice = LoadSoundAlias(sounds[effect]);
        }
    }
    SetSoundEffectVolume(sound_effects_volume);
}

void AudioSystem::StopSoundEffects() {
    for (auto& effect_voices : sound_voices) {
        for (Sound& voice : effect_voices) {
            if (voice.frameCount > 0) {
                StopSound(voice);
            }
        }
    }
}

const AudioClip* AudioSystem::GetClip(MusicId music) const {
    return music == MusicId::None ? nullptr : &generated_audio::test_music;
}
