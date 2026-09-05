#!/usr/bin/env python3

import pathlib
import struct
import tempfile
import unittest

import midi_to_events


def variable_length(value):
    encoded = [value & 0x7F]
    value >>= 7
    while value:
        encoded.append((value & 0x7F) | 0x80)
        value >>= 7
    return bytes(reversed(encoded))


def track(payload):
    return b"MTrk" + struct.pack(">I", len(payload)) + payload


def midi_file(midi_format, division, tracks):
    header = b"MThd" + struct.pack(">IHHH", 6, midi_format, len(tracks), division)
    return header + b"".join(track(payload) for payload in tracks)


class MidiToEventsTests(unittest.TestCase):
    def parse(self, data):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "test.mid"
            path.write_bytes(data)
            return midi_to_events.parse_midi(path, 22050)

    def test_format_zero_running_status(self):
        payload = (
            b"\x00\xc0\x28"
            b"\x00\x90\x3c\x64"
            + variable_length(96) + b"\x3c\x00"
            + b"\x00\xff\x2f\x00"
        )
        events, length = self.parse(midi_file(0, 96, [payload]))
        self.assertEqual(events[0], (0, midi_to_events.PROGRAM_CHANGE, 0, 0, 40))
        self.assertEqual(events[1], (0, midi_to_events.NOTE_ON, 0, 60, 100))
        self.assertEqual(events[2], (11025, midi_to_events.NOTE_OFF, 0, 60, 0))
        self.assertEqual(length, 11025)

    def test_format_one_merges_tempo_and_note_tracks(self):
        tempo_track = (
            b"\x00\xff\x51\x03\x03\xd0\x90"
            + variable_length(96) + b"\xff\x2f\x00"
        )
        note_track = (
            b"\x00\x90\x40\x60"
            + variable_length(96) + b"\x80\x40\x00"
            + b"\x00\xff\x2f\x00"
        )
        events, length = self.parse(midi_file(1, 96, [tempo_track, note_track]))
        self.assertEqual(events[0], (0, midi_to_events.NOTE_ON, 0, 64, 96))
        self.assertEqual(events[1], (5513, midi_to_events.NOTE_OFF, 0, 64, 0))
        self.assertEqual(length, 5513)


if __name__ == "__main__":
    unittest.main()
