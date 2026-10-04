"""Synthetic XFF/ELF checks: no ISO, compiler, or ignored baseline required."""

import contextlib
import importlib.util
import io
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest


SCRIPT = Path(__file__).resolve().parents[1] / "verify_kernel.py"
spec = importlib.util.spec_from_file_location("verify_kernel", SCRIPT)
kernel = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = kernel
spec.loader.exec_module(kernel)

BASE = 0x40012000
TEXT_OFFSET = 0x200


def put(data, offset, fmt, *values):
    struct.pack_into(fmt, data, offset, *values)


def xff(words=None, relocs=None):
    words = [0x3C040000, 0x24840008, 0x0C000000, 0] if words is None else words
    relocs = [(0, 5, 1), (4, 6, 1), (8, 4, 2)] if relocs is None else relocs
    data = bytearray(TEXT_OFFSET + len(words) * 4)
    data[:4] = b"xff2"
    for offset, value in [(20, len(data)), (36, 3), (56, 1), (64, 2),
                          (84, 0x100), (88, 0x140), (92, 0x80),
                          (100, 0x160), (104, 0xC0), (108, 0xD0)]:
        put(data, offset, "<I", value)
    put(data, 0xA0, "<8I", 0, BASE, len(words) * 4, 4, 1, 0, 0, TEXT_OFFSET)
    put(data, 0xC0, "<2I", 0, 1)
    data[0xD0:0xD7] = b"\0.text\0"
    put(data, 0x110, "<3IBBH", 0, 0, 0, 3, 0, 1)
    put(data, 0x120, "<3IBBH", 1, 12, 0, 18, 0, 1)
    data[0x140:0x148] = b"\0target\0"
    put(data, 0x160, "<7I", 9, len(relocs), 1, 0, 0, 0x180, 0x1A0)
    for i, (offset, kind, symbol) in enumerate(relocs):
        put(data, 0x180 + i * 8, "<2I", offset, kind | symbol << 8)
        saved = words[offset // 4] if offset % 4 == 0 and offset // 4 < len(words) else 0
        put(data, 0x1A0 + i * 8, "<2I", saved, 0)
    for i, instruction in enumerate(words):
        put(data, TEXT_OFFSET + i * 4, "<I", instruction)
    return data


def elf(words=None, relocs=None, section_value=0, target_value=12):
    words = [0x3C040000, 0x24840008, 0x0C000000, 0] if words is None else words
    relocs = [(0, 5, 1), (4, 6, 1), (8, 4, 2)] if relocs is None else relocs
    sections = ["", ".text", ".rel.text", ".symtab", ".strtab", ".shstrtab"]
    names = bytearray()
    name_offsets = []
    for name in sections:
        name_offsets.append(len(names))
        names += name.encode() + b"\0"
    data = bytearray(0x300 + len(sections) * 40)
    ident = b"\x7fELF\x01\x01\x01" + bytes(9)
    put(data, 0, "<16sHHIIIIIHHHHHH", ident, 1, 8, 1, 0, 0, 0x300, 0,
        52, 0, 0, 40, len(sections), 5)
    for i, instruction in enumerate(words):
        put(data, 0x80 + i * 4, "<I", instruction)
    for i, (offset, kind, symbol) in enumerate(relocs):
        put(data, 0xA0 + i * 8, "<2I", offset, kind | symbol << 8)
    put(data, 0x110, "<3IBBH", 0, section_value, 0, 3, 0, 1)
    put(data, 0x120, "<3IBBH", 1, target_value, 0, 18, 0, 1)
    data[0x140:0x148] = b"\0target\0"
    data[0x180:0x180 + len(names)] = names
    entries = [
        (0,) * 10,
        (name_offsets[1], 1, 6, BASE, 0x80, len(words) * 4, 0, 0, 4, 0),
        (name_offsets[2], 9, 0, 0, 0xA0, len(relocs) * 8, 3, 1, 4, 8),
        (name_offsets[3], 2, 0, 0, 0x100, 48, 4, 1, 4, 16),
        (name_offsets[4], 3, 0, 0, 0x140, 8, 0, 0, 1, 0),
        (name_offsets[5], 3, 0, 0, 0x180, len(names), 0, 0, 1, 0),
    ]
    for i, entry in enumerate(entries):
        put(data, 0x300 + i * 40, "<10I", *entry)
    return data


class VerifyKernelTests(unittest.TestCase):
    def test_success_and_full_synthetic_destination_coverage(self):
        # Changed immediate bits are legal only when effective destinations
        # remain identical after symbol/section resolution.
        compiled = elf([0x3C040000, 0x24840000, 0x0C000001, 0],
                       section_value=8, target_value=8)
        report = kernel.verify(xff(), compiled)
        self.assertTrue(report["match"])
        self.assertEqual(report["words_checked"], 4)
        self.assertEqual(report["relocation_destinations_checked"], 3)
        self.assertTrue(report["all_relocation_destinations_verified"])

    def test_opcode_changed_at_hi16_is_not_hidden_by_relocation(self):
        report = kernel.verify(xff(), elf([0x34040000, 0x24840008, 0x0C000000, 0]))
        self.assertFalse(report["match"])
        self.assertEqual(report["opcode_and_register_differences"], 1)

    def test_register_changed_at_lo16_is_not_hidden(self):
        report = kernel.verify(xff(), elf([0x3C040000, 0x24850008, 0x0C000000, 0]))
        self.assertFalse(report["match"])
        self.assertEqual(report["opcode_and_register_differences"], 1)

    def test_nonrelocated_word_is_checked_in_full(self):
        report = kernel.verify(xff(), elf([0x3C040000, 0x24840008, 0x0C000000, 1]))
        self.assertFalse(report["match"])
        self.assertEqual(report["opcode_and_register_differences"], 1)

    def test_jump_destination_mismatch(self):
        report = kernel.verify(xff(), elf([0x3C040000, 0x24840008, 0x0C000001, 0]))
        self.assertEqual(report["opcode_and_register_differences"], 0)
        self.assertEqual(report["relocation_destination_differences"], 1)
        self.assertFalse(report["match"])

    def test_hi_lo_destination_mismatch(self):
        report = kernel.verify(xff(), elf([0x3C040000, 0x24840009, 0x0C000000, 0]))
        self.assertEqual(report["opcode_and_register_differences"], 0)
        self.assertEqual(report["relocation_destination_differences"], 1)
        self.assertFalse(report["match"])

    def test_address_literal_destination_mismatch(self):
        original = xff([4, 0, 0, 0], [(0, 2, 2)])
        compiled = elf([8, 0, 0, 0], [(0, 2, 2)])
        report = kernel.verify(original, compiled)
        self.assertFalse(report["match"])
        self.assertEqual(report["opcode_and_register_differences"], 0)
        self.assertEqual(report["relocation_destination_differences"], 1)

    def test_unresolved_symbol_is_explicitly_unverified(self):
        compiled = elf()
        put(compiled, 0x12E, "<H", 0)  # target is SHN_UNDEF
        report = kernel.verify(xff(), compiled)
        self.assertTrue(report["match"])
        self.assertEqual(report["relocation_destinations_unverified"], 1)
        self.assertFalse(report["all_relocation_destinations_verified"])
        self.assertTrue(report["unverified_destination_reasons"])

    def test_compiled_relocation_cannot_expand_original_mask(self):
        original = xff(relocs=[])
        compiled = elf([0x3C040123, 0x24840008, 0x0C000000, 0], [(0, 5, 1)])
        self.assertFalse(kernel.verify(original, compiled)["match"])

    def test_extra_text_runtime_mapping_is_explicitly_unverified(self):
        original = xff()
        put(original, 0x114, "<I", BASE + 0x1000)
        put(original, 0x11E, "<H", 0xFFF1)  # external absolute data symbol
        compiled = elf([0x3C040000, 0x24840010, 0x0C000000, 0, 0, 0, 0, 0])
        report = kernel.verify(original, compiled)
        self.assertTrue(report["match"])
        self.assertEqual(report["relocation_destinations_unverified"], 2)
        self.assertEqual(len(report["first_unmapped_destinations"]), 1)
        self.assertFalse(report["all_relocation_destinations_verified"])

    def test_wrong_text_destination_in_extra_suffix_still_fails(self):
        compiled = elf([0x3C040000, 0x24840008, 0x0C000001, 0, 0, 0, 0, 0])
        report = kernel.verify(xff(), compiled)
        self.assertFalse(report["match"])
        self.assertEqual(report["relocation_destination_differences"], 1)

    def test_resolved_executable_destination_checks(self):
        compiled = elf([0x3C044001, 0x24842008, 0x0C000000 | ((BASE + 12) >> 2 & 0x3FFFFFF), 0],
                       relocs=[], section_value=BASE, target_value=BASE + 12)
        put(compiled, 16, "<H", 2)  # ET_EXEC with already resolved addresses
        report = kernel.verify(xff(), compiled)
        self.assertTrue(report["match"])
        self.assertEqual(report["relocation_destinations_checked"], 3)

    def test_executable_with_retained_relocations_does_not_double_add_symbols(self):
        compiled = elf([0x3C044001, 0x24842008, 0x0C000000 | ((BASE + 12) >> 2 & 0x3FFFFFF), 0],
                       section_value=BASE, target_value=BASE + 12)
        put(compiled, 16, "<H", 2)
        for i, offset in enumerate((0, 4, 8)):
            put(compiled, 0xA0 + i * 8, "<I", BASE + offset)
        self.assertTrue(kernel.verify(xff(), compiled)["match"])

    def test_partial_link_missing_metadata_is_explicitly_unverified(self):
        report = kernel.verify(xff(), elf(relocs=[]))
        self.assertTrue(report["match"])
        self.assertEqual(report["relocation_destinations_unverified"], 3)

    def test_signed_low_carry(self):
        original = xff([0x3C040001, 0x24848008, 0x0C000000, 0])
        compiled = elf([0x3C040000, 0x24840008, 0x0C000000, 0], section_value=0x8000)
        report = kernel.verify(original, compiled)
        self.assertTrue(report["match"])
        self.assertEqual(report["relocation_destinations_checked"], 3)

    def test_incompatible_duplicates_rejected(self):
        cases = [
            (xff(relocs=[(8, 4, 1), (8, 4, 2)]), elf()),
            (xff(), elf(relocs=[(8, 4, 1), (8, 4, 2)])),
            (xff(relocs=[(0, 5, 1), (0, 6, 1)]), elf()),
        ]
        for original, compiled in cases:
            with self.subTest(original=original[0x180:0x190]):
                with self.assertRaises(kernel.InputError):
                    kernel.verify(original, compiled)

    def test_identical_duplicates_deduplicated(self):
        original = xff(relocs=[(0, 5, 1), (0, 5, 1), (4, 6, 1)])
        report = kernel.verify(original, elf())
        self.assertTrue(report["match"])
        self.assertEqual(report["relocation_sites_checked"], 2)

    def test_malformed_inputs_rejected(self):
        mutations = [
            ("xff truncated header", lambda x, e: (x[:10], e)),
            ("elf truncated header", lambda x, e: (x, e[:10])),
            ("xff wrong magic", lambda x, e: (b"bad!" + x[4:], e)),
            ("elf wrong endian", lambda x, e: (x, e[:5] + b"\x02" + e[6:])),
        ]
        for name, mutate in mutations:
            with self.subTest(name=name), self.assertRaises(kernel.InputError):
                kernel.verify(*mutate(xff(), elf()))
        field_mutations = [
            ("xff section table", "x", 92, "<I", 0xFFFFFFF0),
            ("xff section data", "x", 0xBC, "<I", 0xFFFFFFF0),
            ("xff saved table", "x", 0x178, "<I", 0xFFFFFFF0),
            ("xff relocation site", "x", 0x180, "<I", 16),
            ("xff relocation unaligned", "x", 0x180, "<I", 1),
            ("xff relocation symbol", "x", 0x184, "<I", 0xFFFF05),
            ("xff symbol name", "x", 0x120, "<I", 0xFFFFFFFF),
            ("xff unsupported relocation", "x", 0x184, "<I", 0x1FF),
            ("xff saved word", "x", 0x1A0, "<I", 0),
            ("elf text range", "e", 0x338, "<I", 0xFFFFFFFF),
            ("elf string index", "e", 0x120, "<I", 1000),
            ("elf relocation symbol", "e", 0xA4, "<I", 0xFFFF05),
            ("elf relocation stride", "e", 0x374, "<I", 4),
            ("elf relocation link", "e", 0x368, "<I", 1),
            ("elf relocation target", "e", 0x36C, "<I", 999),
        ]
        for name, source, offset, fmt, value in field_mutations:
            original, compiled = xff(), elf()
            put(original if source == "x" else compiled, offset, fmt, value)
            with self.subTest(name=name), self.assertRaises(kernel.InputError):
                kernel.verify(original, compiled)

    def test_cli_report_and_exit_codes(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            original, compiled, report = root / "original.xff", root / "compiled.elf", root / "nested/report.json"
            original.write_bytes(xff())
            args = ["--original", str(original), "--compiled", str(compiled), "--report", str(report)]
            for data, expected in [(elf(), 0),
                                   (elf([0x3C040000, 0x24840008, 0x0C000000, 1]), 1),
                                   (b"bad", 2)]:
                compiled.write_bytes(data)
                with contextlib.redirect_stdout(io.StringIO()) as output:
                    self.assertEqual(kernel.main(args), expected)
                self.assertEqual(json.loads(output.getvalue()), json.loads(report.read_text()))
                self.assertEqual(json.loads(report.read_text())["match"], expected == 0)

    def test_report_cannot_overwrite_binary_inputs(self):
        with tempfile.TemporaryDirectory() as tmp:
            original, compiled = Path(tmp) / "original.xff", Path(tmp) / "compiled.elf"
            original.write_bytes(xff())
            compiled.write_bytes(elf())
            for report in (original, compiled):
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(kernel.main(["--original", str(original), "--compiled", str(compiled),
                                                  "--report", str(report)]), 2)
                self.assertEqual(original.read_bytes(), xff())
                self.assertEqual(compiled.read_bytes(), elf())


if __name__ == "__main__":
    unittest.main()
