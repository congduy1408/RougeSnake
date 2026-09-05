#include "include/generated_audio_data.h"

namespace generated_audio {
namespace {
constexpr AudioNoteEvent test_music_events[] = {
    {0,     static_cast<std::uint8_t>(AudioEventType::ProgramChange), 0, 0, 80},
    {0,     static_cast<std::uint8_t>(AudioEventType::NoteOn),  0, 60, 96},
    {4410,  static_cast<std::uint8_t>(AudioEventType::NoteOff), 0, 60, 0},
    {5512,  static_cast<std::uint8_t>(AudioEventType::NoteOn),  0, 64, 92},
    {9922,  static_cast<std::uint8_t>(AudioEventType::NoteOff), 0, 64, 0},
    {11025, static_cast<std::uint8_t>(AudioEventType::NoteOn),  0, 67, 92},
    {15435, static_cast<std::uint8_t>(AudioEventType::NoteOff), 0, 67, 0},
    {16537, static_cast<std::uint8_t>(AudioEventType::NoteOn),  0, 72, 88},
    {20947, static_cast<std::uint8_t>(AudioEventType::NoteOff), 0, 72, 0},
    {22050, static_cast<std::uint8_t>(AudioEventType::NoteOn),  0, 67, 92},
    {26460, static_cast<std::uint8_t>(AudioEventType::NoteOff), 0, 67, 0},
    {27562, static_cast<std::uint8_t>(AudioEventType::NoteOn),  0, 64, 92},
    {31972, static_cast<std::uint8_t>(AudioEventType::NoteOff), 0, 64, 0},
    {33075, static_cast<std::uint8_t>(AudioEventType::NoteOn),  0, 62, 90},
    {37485, static_cast<std::uint8_t>(AudioEventType::NoteOff), 0, 62, 0},
    {38587, static_cast<std::uint8_t>(AudioEventType::NoteOn),  0, 59, 86},
    {41895, static_cast<std::uint8_t>(AudioEventType::NoteOff), 0, 59, 0}
};
}

const AudioClip test_music = {
    test_music_events,
    sizeof(test_music_events) / sizeof(test_music_events[0]),
    44100
};
}
