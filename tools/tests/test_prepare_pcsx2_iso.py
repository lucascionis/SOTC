"""Small synthetic ISO9660 fixtures; no game image or BIOS is required."""

import hashlib
import importlib.util
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

MODULE_PATH = Path(__file__).resolve().parents[1] / "prepare_pcsx2_iso.py"
SPEC = importlib.util.spec_from_file_location("prepare_pcsx2_iso", MODULE_PATH)
iso_tool = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = iso_tool
SPEC.loader.exec_module(iso_tool)

BLOCK = 2048
LOADER = b"synthetic matching loader\0" * 3
KERNEL = b"synthetic matching kernel\xff" * 5
HASHES = {"SCPS_150.97;1": hashlib.sha1(LOADER).hexdigest(),
          "KERNEL.XFF;1": hashlib.sha1(KERNEL).hexdigest()}


def both(value, width):
    return value.to_bytes(width, "little") + value.to_bytes(width, "big")


def record(name, sector, size, flags=0):
    name = name if isinstance(name, bytes) else name.encode("ascii")
    length = 33 + len(name) + (len(name) % 2 == 0)
    data = bytearray(length)
    data[0] = length
    data[2:10] = both(sector, 4)
    data[10:18] = both(size, 4)
    data[25] = flags
    data[28:32] = both(1, 2)
    data[32] = len(name)
    data[33:33 + len(name)] = name
    return bytes(data)


def fixture(extra=(), nested=False):
    image = bytearray(32 * BLOCK)
    primary = bytearray(BLOCK)
    primary[:7] = b"\x01CD001\x01"
    primary[80:88] = both(32, 4)
    primary[120:124] = both(1, 2)
    primary[124:128] = both(1, 2)
    primary[128:132] = both(BLOCK, 2)
    root_record = record(b"\0", 20, BLOCK, 2)
    primary[156:156 + len(root_record)] = root_record
    image[16 * BLOCK:17 * BLOCK] = primary
    image[17 * BLOCK:17 * BLOCK + 7] = b"\xffCD001\x01"
    directory = [record(b"\0", 20, BLOCK, 2), record(b"\1", 20, BLOCK, 2)]
    targets = [record("SCPS_150.97;1", 22, len(LOADER)),
               record("KERNEL.XFF;1", 24, len(KERNEL))]
    if nested:
        directory.append(record("BIN", 21, BLOCK, 2))
        child = record(b"\0", 21, BLOCK, 2) + record(b"\1", 20, BLOCK, 2) + b"".join(targets)
        image[21 * BLOCK:21 * BLOCK + len(child)] = child
    else:
        directory.extend(targets)
    cnf = b"BOOT2 = cdrom0:\\SCPS_150.97;1\r\nVER = 1.00\r\n"
    directory.append(record("SYSTEM.CNF;1", 26, len(cnf)))
    directory.extend(extra)
    directory = b"".join(directory)
    image[20 * BLOCK:20 * BLOCK + len(directory)] = directory
    image[22 * BLOCK:22 * BLOCK + len(LOADER)] = LOADER
    image[24 * BLOCK:24 * BLOCK + len(KERNEL)] = KERNEL
    image[26 * BLOCK:26 * BLOCK + len(cnf)] = cnf
    return image


class PrepareIsoTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.iso = self.root / "original.iso"
        self.loader = self.root / "SCPS_150.97"
        self.kernel = self.root / "KERNEL.XFF"
        self.output = self.root / "prepared.iso"
        self.report = self.root / "report.json"
        self.iso.write_bytes(fixture())
        self.loader.write_bytes(LOADER)
        self.kernel.write_bytes(KERNEL)

    def prepare(self, **kwargs):
        return iso_tool.prepare_iso(self.iso, self.loader, self.kernel, self.output,
                                    expected_sha1=HASHES, **kwargs)

    def reject(self, message):
        with self.assertRaisesRegex(iso_tool.IsoError, message):
            self.prepare()
        self.assertFalse(self.output.exists())
        self.assertEqual(list(self.root.glob(".pcsx2-*")), [])

    def test_valid_streamed_copy_and_json_report(self):
        result = self.prepare(report_path=self.report)
        self.assertEqual(self.output.read_bytes(), self.iso.read_bytes())
        self.assertEqual(result, json.loads(self.report.read_text()))
        self.assertEqual(result["source_sha1"], result["output_sha1"])
        self.assertEqual(result["boot2"], "BOOT2 = cdrom0:\\SCPS_150.97;1")
        for name, digest in HASHES.items():
            self.assertEqual(result["files"][name]["patched_sha1"], digest)
        self.assertEqual(list(self.root.glob(".pcsx2-*")), [])

    def test_nested_directory(self):
        self.iso.write_bytes(fixture(nested=True))
        result = self.prepare()
        self.assertEqual(result["files"]["KERNEL.XFF;1"]["path"], "/BIN/KERNEL.XFF;1")

    def test_different_replacement_same_length(self):
        self.loader.write_bytes(b"!" + LOADER[1:])
        self.reject("Recompiled.*SHA-1 differs")

    def test_different_replacement_length(self):
        self.kernel.write_bytes(KERNEL + b"!")
        self.reject("Replacement length differs")

    def test_different_original_hash(self):
        data = fixture()
        data[24 * BLOCK] ^= 1
        self.iso.write_bytes(data)
        self.reject("Original ISO.*SHA-1 differs")

    def test_cli_production_hashes_not_fixture_hashes(self):
        with self.assertRaisesRegex(iso_tool.IsoError, "Original ISO"):
            iso_tool.prepare_iso(self.iso, self.loader, self.kernel, self.output)

    def test_output_already_exists(self):
        self.output.write_bytes(b"keep")
        with self.assertRaisesRegex(iso_tool.IsoError, "Refusing to overwrite"):
            self.prepare()
        self.assertEqual(self.output.read_bytes(), b"keep")

    def test_output_broken_symlink_already_exists(self):
        self.output.symlink_to(self.root / "missing")
        with self.assertRaisesRegex(iso_tool.IsoError, "Refusing to overwrite"):
            self.prepare()
        self.assertTrue(self.output.is_symlink())

    def test_same_input_output(self):
        with self.assertRaisesRegex(iso_tool.IsoError, "differ from every input"):
            iso_tool.prepare_iso(self.iso, self.loader, self.kernel, self.iso, expected_sha1=HASHES)

    def test_report_same_as_output_or_input(self):
        for path in (self.iso, self.output):
            with self.assertRaises(iso_tool.IsoError):
                self.prepare(report_path=path)
        self.assertFalse(self.output.exists())

    def test_existing_report_preserved(self):
        self.report.write_text("keep")
        with self.assertRaisesRegex(iso_tool.IsoError, "Refusing to overwrite"):
            self.prepare(report_path=self.report)
        self.assertFalse(self.output.exists())
        self.assertEqual(self.report.read_text(), "keep")

    def test_raced_output_creation_preserved(self):
        link = iso_tool.os.link
        def race(source, destination):
            Path(destination).write_bytes(b"concurrent")
            return link(source, destination)
        with mock.patch.object(iso_tool.os, "link", side_effect=race):
            with self.assertRaises(FileExistsError):
                self.prepare()
        self.assertEqual(self.output.read_bytes(), b"concurrent")
        self.assertEqual(list(self.root.glob(".pcsx2-*")), [])

    def test_report_failure_rolls_back_output(self):
        with mock.patch.object(iso_tool, "_write_report", side_effect=OSError("report failed")):
            with self.assertRaisesRegex(OSError, "report failed"):
                self.prepare(report_path=self.report)
        self.assertFalse(self.output.exists())

    def test_report_race_preserves_concurrent_file_and_rolls_back_iso(self):
        link = iso_tool.os.link
        def race(source, destination):
            if Path(destination) == self.report:
                self.report.write_text("concurrent report")
            return link(source, destination)
        with mock.patch.object(iso_tool.os, "link", side_effect=race):
            with self.assertRaises(FileExistsError):
                self.prepare(report_path=self.report)
        self.assertFalse(self.output.exists())
        self.assertEqual(self.report.read_text(), "concurrent report")
        self.assertEqual(list(self.root.glob(".pcsx2-*")), [])

    def test_report_failure_preserves_concurrently_replaced_iso(self):
        def fail(report_path, report):
            replacement = self.root / "concurrent.iso"
            replacement.write_bytes(b"concurrent ISO")
            replacement.replace(self.output)
            raise OSError("report failed")
        with mock.patch.object(iso_tool, "_write_report", side_effect=fail):
            with self.assertRaises(OSError):
                self.prepare(report_path=self.report)
        self.assertEqual(self.output.read_bytes(), b"concurrent ISO")

    def test_source_mutation_rejected(self):
        copy = iso_tool.shutil.copyfileobj
        def mutate(source, target, length):
            copy(source, target, length)
            with self.iso.open("r+b") as stream:
                stream.write(b"changed")
        with mock.patch.object(iso_tool.shutil, "copyfileobj", side_effect=mutate):
            self.reject("Copied ISO verification failed")

    def test_non_sector_aligned_image_rejected(self):
        self.iso.write_bytes(fixture() + b"!")
        self.reject("multiple of 2048")

    def test_interleaving_rejected(self):
        data = fixture()
        data[20 * BLOCK + 26] = 1
        self.iso.write_bytes(data)
        self.reject("interleaved")

    def test_ambiguous_targets(self):
        self.iso.write_bytes(fixture(extra=[record("KERNEL.XFF;1", 28, len(KERNEL))]))
        self.reject("exactly one file KERNEL")

    def test_missing_target(self):
        self.iso.write_bytes(bytes(fixture()).replace(b"KERNEL.XFF;1", b"OTHER_.XFF;1"))
        self.reject("exactly one file KERNEL")

    def test_overlapping_target(self):
        self.iso.write_bytes(fixture(extra=[record("ALIAS;1", 22, 1)]))
        self.reject("overlaps")

    def test_multiextent(self):
        self.iso.write_bytes(fixture(extra=[record("PART;1", 28, 1, 128)]))
        self.reject("Multi-extent")

    def test_directory_cycle(self):
        self.iso.write_bytes(fixture(extra=[record("LOOP", 20, BLOCK, 2)]))
        self.reject("cyclic")

    def test_out_of_bounds_extent(self):
        self.iso.write_bytes(fixture(extra=[record("BAD;1", 32, 1)]))
        self.reject("outside ISO")

    def test_mismatched_endian_field(self):
        data = fixture()
        data[16 * BLOCK + 84] ^= 1
        self.iso.write_bytes(data)
        self.reject("byte order")

    def test_non2048_block(self):
        data = fixture()
        data[16 * BLOCK + 128:16 * BLOCK + 132] = both(1024, 2)
        self.iso.write_bytes(data)
        self.reject("Only 2048")

    def test_raw2352_image(self):
        cooked = fixture()
        raw = b"".join(b"\0" * 24 + cooked[offset:offset + BLOCK] + b"\0" * 280
                       for offset in range(0, len(cooked), BLOCK))
        self.iso.write_bytes(raw + b"\0" * (-len(raw) % BLOCK))
        self.reject("volume descriptor")

    def test_truncated_image(self):
        self.iso.write_bytes(fixture()[:25 * BLOCK])
        self.reject("volume length")

    def test_primary_descriptor_required(self):
        data = fixture()
        data[16 * BLOCK] = 2
        self.iso.write_bytes(data)
        self.reject("Missing primary")

    def test_record_crossing_sector(self):
        data = fixture()
        data[20 * BLOCK + BLOCK - 1] = 34
        # Reach the last byte through a chain of otherwise parsable records.
        filler = record("X" * 180, 28, 0)
        at = 20 * BLOCK
        used = 0
        while used + len(filler) <= BLOCK - 35:
            data[at + used:at + used + len(filler)] = filler
            used += len(filler)
        tail_size = BLOCK - 1 - used
        tail = bytearray(record("Y" * (tail_size - 34), 28, 0))
        tail[0] = tail_size
        tail.extend(b"\0" * (tail_size - len(tail)))
        data[at + used:at + BLOCK - 1] = tail
        self.iso.write_bytes(data)
        self.reject("crosses a sector")

    def test_parser_standalone(self):
        records, end = iso_tool.read_iso9660(io.BytesIO(fixture()))
        self.assertEqual(end, 18 * BLOCK)
        targets = iso_tool.locate_files(records, end, HASHES)
        self.assertEqual(targets["SCPS_150.97;1"].offset, 22 * BLOCK)


if __name__ == "__main__":
    unittest.main()
