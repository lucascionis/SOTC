"""Binary fixtures for PCSX2 version 1 movies; no emulator, BIOS, or disc needed."""

import contextlib
from decimal import Decimal
import importlib.util
import io
import json
from pathlib import Path
import struct
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location(
    "pcsx2_input_movie", Path(__file__).resolve().parents[1] / "pcsx2_input_movie.py")
movie = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(movie)

NEUTRAL = bytes([255, 255, 127, 127, 127, 127] + [0] * 12)


class InputMovieTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.output = self.root / "fixture.p2m2"

    def plan(self, segments=None, frames=8, **kwargs):
        return {"frame_count": frames, "segments": segments or [], **kwargs}

    def segment(self, start=2, end=4, **kwargs):
        return {"start_frame": start, "end_frame": end, "buttons": ["Cross"], **kwargs}

    def pad_at(self, frame, port=0):
        start = 570 + 36 * frame + 18 * port
        return self.output.read_bytes()[start:start + 18]

    def test_header_layout_strings_and_little_endian_counters(self):
        plan = self.plan(frames=260, emulator_version="PCSX2-2.8.2",
                         author="Test é", game_name="Shadow of the Colossus")
        movie.create_movie(plan, self.output)
        data = self.output.read_bytes()
        self.assertEqual(len(data), 570 + 260 * 36)
        self.assertEqual(data[0], 1)
        self.assertEqual(data[1:51], b"PCSX2-2.8.2".ljust(50, b"\0"))
        self.assertEqual(data[51:306], "Test é".encode().ljust(255, b"\0"))
        self.assertEqual(data[306:561], b"Shadow of the Colossus".ljust(255, b"\0"))
        self.assertEqual(data[561:570], b"\x04\x01\0\0" + b"\0" * 5)
        self.assertEqual(data[570:], NEUTRAL * 2 * 260)

    def test_all_button_bit_and_pressure_positions(self):
        # These wire positions are taken directly from the official PadData format.
        group0 = [("select", None), ("l3", None), ("r3", None), ("start", None),
                  ("up", 8), ("right", 6), ("down", 9), ("left", 7)]
        group1 = [("l2", 16), ("r2", 17), ("l1", 14), ("r1", 15),
                  ("triangle", 10), ("circle", 11), ("cross", 12), ("square", 13)]
        for group, buttons in enumerate((group0, group1)):
            for bit, (name, pressure) in enumerate(buttons):
                with self.subTest(button=name):
                    expected = bytearray(NEUTRAL)
                    expected[group] = 255 ^ (1 << bit)
                    if pressure is not None:
                        expected[pressure] = 255
                    self.assertEqual(movie.pad_data({"buttons": [name.upper()]}), bytes(expected))
        self.assertEqual(movie.pad_data({"buttons": list(movie.BUTTONS)}),
                         bytes([0, 0, 127, 127, 127, 127] + [255] * 12))

    def test_frame_and_port_offsets_analog_order_and_end_exclusion(self):
        movie.create_movie(self.plan([self.segment(port=1, right=[11, 22], left=[33, 44])]), self.output)
        pressed = bytes([255, 191, 11, 22, 33, 44] + [0] * 6 + [255] + [0] * 5)
        for frame in range(8):
            self.assertEqual(self.pad_at(frame, 0), NEUTRAL)
            self.assertEqual(self.pad_at(frame, 1), pressed if 2 <= frame < 4 else NEUTRAL)

    def test_seconds_half_up_and_generic_time_form(self):
        segment = {"start_seconds": Decimal("0.05"), "end_seconds": Decimal("0.15"),
                   "buttons": ["start"]}
        movie.create_movie(self.plan([segment], fps=10), self.output)
        self.assertEqual(self.pad_at(0), NEUTRAL)
        self.assertEqual(self.pad_at(1)[0], 247)
        self.assertEqual(self.pad_at(2), NEUTRAL)
        plan = self.plan([{"start": 2, "end": 3, "unit": "frames", "buttons": []}])
        self.assertEqual(movie.validate_plan(plan)[2][0][:2], (2, 3))

    def test_adjacent_and_different_port_intervals_are_allowed(self):
        movie.create_movie(self.plan([self.segment(0, 2), self.segment(2, 3),
                                      self.segment(0, 3, port=1)]), self.output)
        self.assertEqual(self.pad_at(2, 0)[1], 191)
        self.assertEqual(self.pad_at(2, 1)[1], 191)

    def test_create_refuses_overwrite_or_symlink(self):
        movie.create_movie(self.plan(), self.output)
        before = self.output.read_bytes()
        with self.assertRaises(FileExistsError):
            movie.create_movie(self.plan([self.segment()]), self.output)
        self.assertEqual(self.output.read_bytes(), before)
        link = self.root / "link.p2m2"
        link.symlink_to(self.output)
        with self.assertRaises(FileExistsError):
            movie.create_movie(self.plan(), link)
        self.assertEqual(self.output.read_bytes(), before)

    def test_invalid_plan_is_rejected_before_creating_file(self):
        bad = [self.plan(frames=0), self.plan(frames=True), self.plan(frames=1 << 32),
               self.plan(fps=0), self.plan(fps=float("nan")), self.plan(fps=True),
               self.plan(typo="wrong"), self.plan([self.segment(-1, 2)]),
               self.plan([self.segment(2, 2)]), self.plan([self.segment(7, 9)]),
               self.plan([self.segment(port=2)]), self.plan([self.segment(left=[-1, 127])]),
               self.plan([self.segment(right=[127, 256])]), self.plan([self.segment(left=[True, 127])]),
               self.plan([self.segment(buttons=["unknown"])]),
               self.plan([self.segment(buttons=["cross", "Cross"])]),
               self.plan([self.segment(start_seconds=1)]),
               self.plan([self.segment(1, 4), self.segment(3, 5)]),
               self.plan(author="\0"), self.plan(emulator_version="é" * 25),
               self.plan(game_name="x" * 255)]
        for plan in bad:
            with self.subTest(plan=plan), self.assertRaises(movie.MovieError):
                movie.create_movie(plan, self.output)
            self.assertFalse(self.output.exists())

    def test_amend_only_future_selected_pad_and_keeps_inode_header_size(self):
        movie.create_movie(self.plan([self.segment(0, 8, port=1)]), self.output)
        # Existing v1 recordings may contain a nonzero undo count; preserve it.
        before = bytearray(self.output.read_bytes())
        struct.pack_into("<I", before, 565, 9)
        self.output.write_bytes(before)
        inode = self.output.stat().st_ino
        movie.amend_movie(self.plan([self.segment(5, 7, buttons=["up"], left=[0, 255])]),
                          self.output, current_frame=3, lead_frames=2)
        after = self.output.read_bytes()
        self.assertEqual(self.output.stat().st_ino, inode)
        self.assertEqual(len(after), len(before))
        self.assertEqual(after[:570 + 5 * 36], before[:570 + 5 * 36])
        self.assertEqual(after[570 + 7 * 36:], before[570 + 7 * 36:])
        for frame in range(8):
            self.assertEqual(self.pad_at(frame, 1)[1], 191)
        self.assertEqual(self.pad_at(5), bytes([239, 255, 127, 127, 0, 255, 0, 0, 255] + [0] * 9))
        self.assertEqual(after[:570], before[:570])

    def test_invalid_amendment_does_not_change_movie(self):
        movie.create_movie(self.plan(), self.output)
        before = self.output.read_bytes()
        bad = [(self.plan([self.segment(3, 5)]), 3, 1),
               (self.plan([self.segment(4, 5)]), 3, 2),
               (self.plan([self.segment(5, 6)]), -1, 1),
               (self.plan([self.segment(5, 6)]), 3, 0),
               (self.plan([self.segment(5, 6)], frames=9), 3, 1),
               (self.plan([self.segment(5, 6)], author="different"), 3, 1),
               (self.plan(), 3, 1)]
        for plan, current, lead in bad:
            with self.subTest(plan=plan), self.assertRaises(movie.MovieError):
                movie.amend_movie(plan, self.output, current, lead)
            self.assertEqual(self.output.read_bytes(), before)

    def test_invalid_headers_or_length_are_rejected_before_amendment(self):
        movie.create_movie(self.plan(), self.output)
        valid = self.output.read_bytes()
        fixtures = [valid[:569], bytes([2]) + valid[1:], valid[:-1], valid + b"extra"]
        for offset, value in ((569, 1), (569, 2), (561, 0)):
            changed = bytearray(valid)
            changed[offset] = value
            fixtures.append(bytes(changed))
        no_nul = bytearray(valid)
        no_nul[1:51] = b"x" * 50
        fixtures.append(bytes(no_nul))
        for data in fixtures:
            self.output.write_bytes(data)
            with self.subTest(length=len(data)), self.assertRaises(movie.MovieError):
                movie.amend_movie(self.plan([self.segment(5, 6)]), self.output, 3, 1)
            self.assertEqual(self.output.read_bytes(), data)

    def test_json_duplicate_keys_and_nonfinite_values_are_rejected(self):
        path = self.root / "bad.json"
        for text in ('{"frame_count":8,"frame_count":9}', '{"frame_count":8,"fps":NaN}'):
            path.write_text(text)
            with self.assertRaises(movie.MovieError):
                movie.load_plan(path)

    def test_cli_create_and_amend_reports_without_emulator(self):
        path = self.root / "plan.json"
        path.write_text(json.dumps(self.plan()))
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(movie.main(["create", str(path), str(self.output)]), 0)
        report = json.loads(output.getvalue())
        self.assertEqual(report["frame_count"], 8)
        self.assertEqual(report["plan_sha256"], movie.sha256(path))
        self.assertNotIn("runtime", report)
        path.write_text(json.dumps(self.plan([self.segment(5, 6)])))
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(movie.main(["amend", str(path), str(self.output),
                                         "--current-frame", "3", "--lead-frames", "2"]), 0)
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(movie.main(["create", str(path), str(self.output)]), 2)


if __name__ == "__main__":
    unittest.main()
