#!/usr/bin/env python3
"""Convert a small Standard MIDI File into sample-positioned C++ note events."""

import argparse
import pathlib
import re
import struct
import sys


NOTE_OFF = 0
NOTE_ON = 1
PROGRAM_CHANGE = 2


def read_variable_length(data, offset):
    value = 0
    for _ in range(4):
        if offset >= len(data):
            raise ValueError("truncated variable-length value")
        byte = data[offset]
        offset += 1
        value = (value << 7) | (byte & 0x7F)
        if byte & 0x80 == 0:
            return value, offset
    raise ValueError("variable-length value exceeds four bytes")


def parse_track(data, sequence_start):
    events = []
    offset = 0
    tick = 0
    running_status = None
    sequence = sequence_start

    while offset < len(data):
        delta, offset = read_variable_length(data, offset)
        tick += delta
        if offset >= len(data):
            raise ValueError("truncated MIDI event")

        first = data[offset]
        if first & 0x80:
            status = first
            offset += 1
            if status < 0xF0:
                running_status = status
        elif running_status is not None:
            status = running_status
        else:
            raise ValueError("running status used before a channel status byte")

        if status == 0xFF:
            running_status = None
            if offset >= len(data):
                raise ValueError("truncated meta event")
            meta_type = data[offset]
            offset += 1
            length, offset = read_variable_length(data, offset)
            payload = data[offset:offset + length]
            if len(payload) != length:
                raise ValueError("truncated meta-event payload")
            offset += length
            if meta_type == 0x51 and length == 3:
                tempo = int.from_bytes(payload, "big")
                events.append((tick, 0, sequence, "tempo", 0, 0, tempo))
                sequence += 1
            if meta_type == 0x2F:
                break
            continue

        if status in (0xF0, 0xF7):
            running_status = None
            length, offset = read_variable_length(data, offset)
            offset += length
            if offset > len(data):
                raise ValueError("truncated SysEx payload")
            continue

        message = status & 0xF0
        channel = status & 0x0F
        data_length = 1 if message in (0xC0, 0xD0) else 2
        if offset + data_length > len(data):
            raise ValueError("truncated channel event")
        first_data = data[offset]
        second_data = data[offset + 1] if data_length == 2 else 0
        offset += data_length

        if message == 0x80:
            events.append((tick, 1, sequence, "note_off", channel, first_data, second_data))
            sequence += 1
        elif message == 0x90:
            event_type = "note_on" if second_data > 0 else "note_off"
            events.append((tick, 1, sequence, event_type, channel, first_data, second_data))
            sequence += 1
        elif message == 0xC0:
            events.append((tick, 1, sequence, "program", channel, 0, first_data))
            sequence += 1

    return events, sequence, tick


def parse_midi(path, sample_rate):
    data = path.read_bytes()
    if len(data) < 14 or data[:4] != b"MThd":
        raise ValueError("not a Standard MIDI File")
    header_length = struct.unpack_from(">I", data, 4)[0]
    if header_length < 6 or 8 + header_length > len(data):
        raise ValueError("invalid MIDI header")
    midi_format, track_count, division = struct.unpack_from(">HHH", data, 8)
    if midi_format not in (0, 1):
        raise ValueError("only MIDI formats 0 and 1 are supported")
    if division & 0x8000:
        raise ValueError("SMPTE time division is not supported")

    offset = 8 + header_length
    merged = []
    sequence = 0
    final_tick = 0
    for _ in range(track_count):
        if offset + 8 > len(data) or data[offset:offset + 4] != b"MTrk":
            raise ValueError("missing MIDI track chunk")
        length = struct.unpack_from(">I", data, offset + 4)[0]
        offset += 8
        track = data[offset:offset + length]
        if len(track) != length:
            raise ValueError("truncated MIDI track")
        offset += length
        track_events, sequence, track_end = parse_track(track, sequence)
        merged.extend(track_events)
        final_tick = max(final_tick, track_end)

    merged.sort(key=lambda event: (event[0], event[1], event[2]))
    tempo = 500000
    previous_tick = 0
    elapsed_numerator = 0
    denominator = division * 1000000
    converted = []
    for tick, _, _, kind, channel, note, value in merged:
        elapsed_numerator += (tick - previous_tick) * tempo * sample_rate
        previous_tick = tick
        sample_position = (elapsed_numerator + denominator // 2) // denominator
        if kind == "tempo":
            tempo = value
            continue
        event_type = {
            "note_off": NOTE_OFF,
            "note_on": NOTE_ON,
            "program": PROGRAM_CHANGE,
        }[kind]
        converted.append((sample_position, event_type, channel, note, value))

    elapsed_numerator += (final_tick - previous_tick) * tempo * sample_rate
    length_samples = (elapsed_numerator + denominator // 2) // denominator
    return converted, max(length_samples, 1)


def cpp_identifier(value):
    identifier = re.sub(r"[^A-Za-z0-9_]", "_", value)
    if not identifier or identifier[0].isdigit():
        identifier = "clip_" + identifier
    return identifier


def write_cpp(path, namespace, symbol, events, length_samples, tail_samples):
    symbol = cpp_identifier(symbol)
    namespace = cpp_identifier(namespace)
    lines = [
        '#include "include/audio_system.h"',
        "",
        f"namespace {namespace} {{",
        "namespace {",
        f"constexpr AudioNoteEvent {symbol}_events[] = {{",
    ]
    for sample, event_type, channel, note, value in events:
        lines.append(f"    {{{sample}, {event_type}, {channel}, {note}, {value}}},")
    lines.extend([
        "};",
        "}",
        "",
        f"const AudioClip {symbol} = {{",
        f"    {symbol}_events,",
        f"    sizeof({symbol}_events) / sizeof({symbol}_events[0]),",
        f"    {length_samples + tail_samples}",
        "};",
        "}",
        "",
    ])
    path.write_text("\n".join(lines), encoding="ascii")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=pathlib.Path)
    parser.add_argument("output", type=pathlib.Path)
    parser.add_argument("--symbol", default="generated_clip")
    parser.add_argument("--namespace", default="generated_audio")
    parser.add_argument("--sample-rate", type=int, default=22050)
    parser.add_argument("--tail-ms", type=int, default=50)
    args = parser.parse_args()
    if args.sample_rate <= 0 or args.tail_ms < 0:
        parser.error("sample rate must be positive and tail must not be negative")

    try:
        events, length_samples = parse_midi(args.input, args.sample_rate)
        tail_samples = args.sample_rate * args.tail_ms // 1000
        write_cpp(args.output, args.namespace, args.symbol, events, length_samples, tail_samples)
    except (OSError, ValueError) as error:
        print(f"midi_to_events: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
