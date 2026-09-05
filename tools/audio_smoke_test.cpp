#include "include/audio_system.h"

#include <raylib.h>

int main() {
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(64, 64, "Rouge Snake audio smoke test");
    InitAudioDevice();

    AudioSystem audio;
    if (!audio.Initialize()) {
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }

    audio.SetMusicVolume(0.1f);
    audio.SetSoundEffectVolume(0.4f);
    audio.PlayMusic(MusicId::Gameplay, true);
    audio.PlaySoundEffect(SoundEffectId::FoodEaten);
    audio.PlaySoundEffect(SoundEffectId::FoodEaten);
    audio.PlaySoundEffect(SoundEffectId::ItemCollected);
    audio.PlaySoundEffect(SoundEffectId::RockImpact);
    WaitTime(0.35);

    audio.StopMusic();
    audio.SetMusicEnabled(false);
    audio.SetSoundEffectsEnabled(false);
    WaitTime(0.05);
    audio.Shutdown();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
