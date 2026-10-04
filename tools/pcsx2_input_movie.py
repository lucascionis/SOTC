#!/usr/bin/env python3
"""Create or amend boot-based PCSX2 version 1 input movies from a JSON plan."""

import argparse
from decimal import Decimal, InvalidOperation, ROUND_HALF_UP
import hashlib
import json
import os
from pathlib import Path
import stat
import struct
import sys
from typing import NamedTuple


# PCSX2 v2.8.2 Recording/InputRecordingFile.{h,cpp} and Recording/PadData.cpp.
HEADER = struct.Struct("<B50s255s255sIIB")
HEADER_SIZE = 570
PAD_SIZE = 18
FRAME_SIZE = 36
NEUTRAL_PAD = bytes([255, 255, 127, 127, 127, 127] + [0] * 12)
UINT32_MAX = (1 << 32) - 1
BUTTONS = {
    "select": (0, 0, None), "l3": (0, 1, None), "r3": (0, 2, None),
    "start": (0, 3, None), "up": (0, 4, 8), "right": (0, 5, 6),
    "down": (0, 6, 9), "left": (0, 7, 7),
    "l2": (1, 0, 16), "r2": (1, 1, 17), "l1": (1, 2, 14),
    "r1": (1, 3, 15), "triangle": (1, 4, 10), "circle": (1, 5, 11),
    "cross": (1, 6, 12), "square": (1, 7, 13),
}
PLAN_KEYS = {"frame_count", "fps", "segments", "emulator_version", "author", "game_name"}
SEGMENT_KEYS = {"start_frame", "end_frame", "start_seconds", "end_seconds",
                "start", "end", "unit", "port", "buttons", "left", "right"}
METADATA = (("emulator_version", 50, "PCSX2-2.8.2"),
            ("author", 255, ""), ("game_name", 255, ""))
assert HEADER.size == HEADER_SIZE


class MovieError(ValueError):
    pass


class Segment(NamedTuple):
    start: int
    end: int
    port: int
    pad: bytes


def integer(value, label, minimum, maximum):
    if isinstance(value, bool) or not isinstance(value, int) or not minimum <= value <= maximum:
        raise MovieError(f"{label} must be an integer from {minimum} to {maximum}")
    return value


def number(value, label, positive=False):
    if isinstance(value, bool) or not isinstance(value, (int, float, Decimal)):
        raise MovieError(f"{label} must be a number")
    try:
        result = Decimal(str(value))
    except InvalidOperation as exc:
        raise MovieError(f"Invalid {label}") from exc
    if not result.is_finite() or result < 0 or (positive and result == 0):
        raise MovieError(f"{label} must be finite and {'positive' if positive else 'nonnegative'}")
    return result


def c_string(value, length, label):
    if not isinstance(value, str) or "\0" in value:
        raise MovieError(f"{label} must be a string without NUL bytes")
    encoded = value.encode("utf-8")
    if len(encoded) >= length:
        raise MovieError(f"{label} exceeds {length - 1} UTF-8 bytes")
    return encoded.ljust(length, b"\0")


def pad_data(segment):
    pad = bytearray(NEUTRAL_PAD)
    buttons = segment.get("buttons", [])
    if not isinstance(buttons, list):
        raise MovieError("buttons must be a list of button names")
    seen = set()
    for name in buttons:
        if not isinstance(name, str) or name.lower() not in BUTTONS:
            raise MovieError(f"Unknown button: {name!r}")
        name = name.lower()
        if name in seen:
            raise MovieError(f"Duplicate button: {name}")
        seen.add(name)
        group, bit, pressure = BUTTONS[name]
        pad[group] &= ~(1 << bit)
        if pressure is not None:
            pad[pressure] = 255
    for name, offset in (("right", 2), ("left", 4)):
        value = segment.get(name, [127, 127])
        if not isinstance(value, list) or len(value) != 2:
            raise MovieError(f"{name} must be [x, y] with two byte values")
        pad[offset:offset + 2] = bytes(integer(v, name, 0, 255) for v in value)
    return bytes(pad)


def segment_bounds(segment, fps):
    forms = sum(bool(set(segment) & fields) for fields in (
        {"start_frame", "end_frame"}, {"start_seconds", "end_seconds"}, {"start", "end", "unit"}))
    if forms != 1:
        raise MovieError("Use one timing form: start_frame/end_frame, start_seconds/end_seconds, or start/end/unit")
    if "start_frame" in segment or "end_frame" in segment:
        start_key, end_key, unit = "start_frame", "end_frame", "frames"
    elif "start_seconds" in segment or "end_seconds" in segment:
        start_key, end_key, unit = "start_seconds", "end_seconds", "seconds"
    else:
        start_key, end_key, unit = "start", "end", segment.get("unit", "frames")
    if start_key not in segment or end_key not in segment:
        raise MovieError(f"Both {start_key} and {end_key} are required")
    if unit == "frames":
        return tuple(integer(segment[k], k, 0, UINT32_MAX) for k in (start_key, end_key))
    if unit == "seconds":
        return tuple(int((number(segment[k], k) * fps).to_integral_value(rounding=ROUND_HALF_UP))
                     for k in (start_key, end_key))
    raise MovieError("unit must be frames or seconds")


def validate_plan(plan):
    if not isinstance(plan, dict) or set(plan) - PLAN_KEYS:
        raise MovieError("Plan must be an object containing only documented fields")
    frames = integer(plan.get("frame_count"), "frame_count", 1, UINT32_MAX)
    fps = number(plan.get("fps", 60), "fps", positive=True)
    metadata = [c_string(plan.get(key, default), length, key) for key, length, default in METADATA]
    raw_segments = plan.get("segments", [])
    if not isinstance(raw_segments, list):
        raise MovieError("segments must be a list")
    segments = []
    for index, item in enumerate(raw_segments):
        if not isinstance(item, dict) or set(item) - SEGMENT_KEYS:
            raise MovieError(f"Segment {index} must contain only documented fields")
        start, end = segment_bounds(item, fps)
        if not 0 <= start < end <= frames:
            raise MovieError(f"Segment {index} must have 0 <= start < end <= frame_count")
        port = integer(item.get("port", 0), "port", 0, 1)
        segments.append(Segment(start, end, port, pad_data(item)))
    segments.sort(key=lambda s: (s.port, s.start, s.end))
    for previous, current in zip(segments, segments[1:]):
        if previous.port == current.port and current.start < previous.end:
            raise MovieError(f"Overlapping segments on port {current.port}")
    return frames, metadata, segments


def load_plan(path):
    def unique_keys(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise MovieError(f"Duplicate JSON key: {key}")
            result[key] = value
        return result

    def invalid_constant(value):
        raise MovieError(f"Nonfinite JSON number: {value}")

    return json.loads(Path(path).read_text(encoding="utf-8"), parse_float=Decimal,
                      object_pairs_hook=unique_keys, parse_constant=invalid_constant)


def apply_segments(stream, segments):
    # Do not replace the inode: PCSX2 can hold the movie open during replay.
    # Only the selected controller's 18 bytes are replaced in each frame.
    for segment in segments:
        for frame in range(segment.start, segment.end):
            stream.seek(HEADER_SIZE + frame * FRAME_SIZE + segment.port * PAD_SIZE)
            if stream.write(segment.pad) != PAD_SIZE:
                raise OSError("Short write while updating input movie")


def create_movie(plan, output):
    frames, metadata, segments = validate_plan(plan)
    header = HEADER.pack(1, *metadata, frames, 0, 0)
    output = Path(output)
    # Exclusive creation rejects existing files and symlinks before writing.
    with output.open("xb", buffering=0) as stream:
        try:
            if stream.write(header) != HEADER_SIZE:
                raise OSError("Short write while creating input movie header")
            neutral_frame = NEUTRAL_PAD * 2
            block_frames = 4096
            block = neutral_frame * block_frames
            remaining = frames
            while remaining:
                count = min(remaining, block_frames)
                data = block if count == block_frames else neutral_frame * count
                if stream.write(data) != len(data):
                    raise OSError("Short write while creating input movie")
                remaining -= count
            apply_segments(stream, segments)
            os.fsync(stream.fileno())
        except BaseException:
            output.unlink()
            raise
    return frames


def read_header(stream):
    if not stat.S_ISREG(os.fstat(stream.fileno()).st_mode):
        raise MovieError("Input movie must be a regular file")
    stream.seek(0)
    raw = stream.read(HEADER_SIZE)
    if len(raw) != HEADER_SIZE:
        raise MovieError("Truncated input movie header")
    version, emulator, author, game, frames, undo, from_state = HEADER.unpack(raw)
    if version != 1:
        raise MovieError("Only input movie version 1 is supported")
    if from_state != 0:
        raise MovieError("Only movies starting from boot, without a save state, are supported")
    for value in (emulator, author, game):
        if b"\0" not in value:
            raise MovieError("Input movie metadata must be NUL terminated")
    if not frames or os.fstat(stream.fileno()).st_size != HEADER_SIZE + frames * FRAME_SIZE:
        raise MovieError("Movie size does not match header frame count")
    return raw, frames, (emulator, author, game)


def amend_movie(plan, movie, current_frame, lead_frames=120):
    frames, metadata, segments = validate_plan(plan)
    current_frame = integer(current_frame, "current_frame", 0, UINT32_MAX)
    lead_frames = integer(lead_frames, "lead_frames", 1, UINT32_MAX)
    if not segments:
        raise MovieError("An amendment requires at least one segment")
    if any(segment.start < current_frame + lead_frames for segment in segments):
        raise MovieError("Amendment would touch a past frame or the future safety margin")
    with Path(movie).open("r+b", buffering=0) as stream:
        original_header, original_frames, original_metadata = read_header(stream)
        if frames != original_frames:
            raise MovieError("Amendment frame_count must match the existing movie")
        for (key, _, _), wanted, existing in zip(METADATA, metadata, original_metadata):
            if key in plan and wanted.split(b"\0", 1)[0] != existing.split(b"\0", 1)[0]:
                raise MovieError(f"Amendment cannot change header field {key}")
        apply_segments(stream, segments)
        os.fsync(stream.fileno())
        # Header and total length are never updated by this tool.
        current_header, _, _ = read_header(stream)
        if current_header != original_header:
            raise MovieError("Movie header changed concurrently during amendment")
    return frames


def sha256(path):
    value = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    create = sub.add_parser("create", help="Create a new movie without overwriting an existing file")
    create.add_argument("plan", type=Path)
    create.add_argument("movie", type=Path)
    amend = sub.add_parser("amend", help="Replace only future segments in an existing movie, in place")
    amend.add_argument("plan", type=Path)
    amend.add_argument("movie", type=Path)
    amend.add_argument("--current-frame", type=int, required=True,
                       help="Last consumed replay frame, observed by the caller")
    amend.add_argument("--lead-frames", type=int, default=120,
                       help="Minimum future margin; default: 120 frames")
    args = parser.parse_args(argv)
    try:
        plan = load_plan(args.plan)
        if args.command == "create":
            frames = create_movie(plan, args.movie)
        else:
            frames = amend_movie(plan, args.movie, args.current_frame, args.lead_frames)
        report = {"operation": args.command, "movie": str(args.movie.resolve()),
                  "frame_count": frames, "movie_sha256": sha256(args.movie),
                  "plan_sha256": sha256(args.plan)}
        if args.command == "amend":
            report.update(current_frame=args.current_frame, lead_frames=args.lead_frames)
        print(json.dumps(report, indent=2))
        return 0
    except (OSError, ValueError, ArithmeticError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
