"""Regression checks for progress accounting; no original binaries needed."""

import importlib.util
import argparse
from pathlib import Path
import tempfile
import unittest
import xml.etree.ElementTree as ET
from unittest.mock import patch

import mapfile_parser


SCRIPT = Path(__file__).resolve().parents[2] / "scripts/upload_progress.py"
spec = importlib.util.spec_from_file_location("upload_progress", SCRIPT)
progress = importlib.util.module_from_spec(spec)
spec.loader.exec_module(progress)


class UploadProgressTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.asm = self.root / "asm/KERNEL.XFF"
        self.nonmatchings = self.asm / "nonmatchings"
        self.nonmatchings.mkdir(parents=True)

    def section(self, source, symbols, kind="src"):
        return mapfile_parser.Section(
            filepath=Path(f"build/KERNEL.XFF/{kind}/KERNEL.XFF/{source}.o"),
            vram=0x1000, size=sum(size for _, size in symbols), sectionType=".text",
            _symbols=[mapfile_parser.Symbol(name, 0x1000, size) for name, size in symbols],
        )

    def test_byte_weighted_mixed_sources_and_handwritten_startup(self):
        # Include a dotted source name: only .c.o should be stripped.
        stub = self.nonmatchings / "ios/name.with.dot/pending.s"
        stub.parent.mkdir(parents=True)
        stub.touch()
        sections = [
            self.section("ios/name.with.dot.c", [("done", 16), ("pending", 48)]),
            self.section("sdk/libc/memcpy.s", [("memcpy", 80)], kind="asm"),
            self.section("sdk/crt0.s", [("_start", 8)], kind="asm"),
        ]
        total, folders = progress.getProgressFromMapFile([sections], self.asm, self.nonmatchings)
        self.assertEqual((total.decompedSize, total.undecompedSize), (24, 128))
        self.assertEqual(folders["ios"].decompedSize, 16)
        self.assertEqual(folders["sdk"].undecompedSize, 80)

    def test_missing_assembly_directory_fails_instead_of_reporting_complete(self):
        with self.assertRaises(FileNotFoundError):
            progress.getProgressFromMapFile([], self.root / "asm/kernel", self.nonmatchings)
        self.nonmatchings.rmdir()
        with self.assertRaises(FileNotFoundError):
            progress.getProgressFromMapFile([], self.asm, self.nonmatchings)

    def test_unexpected_object_layout_is_rejected(self):
        section = self.section("ios/cd.c", [("done", 16)])
        section.filepath = Path("build/kernel/ios/cd.c.o")
        with self.assertRaises(ValueError):
            progress.getProgressFromMapFile([[section]], self.asm, self.nonmatchings)

    def test_empty_map_is_rejected(self):
        map_path = self.root / "empty.map"
        map_path.touch()
        with patch.object(progress, "ROOT", self.root), patch.object(progress, "ASMPATH", self.root / "asm"):
            with self.assertRaisesRegex(ValueError, "No code symbols"):
                progress.getProgress("empty.map", "KERNEL.XFF")

    def test_service_categories_are_separate_from_module_paths(self):
        stats = (mapfile_parser.ProgressStats(decompedSize=16, undecompedSize=48), {})
        with patch.object(progress, "getProgress", return_value=stats) as get_progress, patch.object(
            mapfile_parser.frontends.upload_frogress, "uploadEntriesToFrogress"
        ) as upload:
            progress.processMapFiles(progress.MAP_FILES, "test-key")
            self.assertEqual(get_progress.call_args_list[0].args,
                             ("build/SCPS_150.97/SCPS_150.97.map", "SCPS_150.97"))
            self.assertEqual(get_progress.call_args_list[1].args,
                             ("build/KERNEL.XFF/KERNEL.XFF.map", "KERNEL.XFF"))
            self.assertEqual([call.args[1] for call in upload.call_args_list], ["loader", "kernel"])
            upload.reset_mock()
            progress.processMapFiles(progress.MAP_FILES, None, dry_run=True)
            upload.assert_not_called()

    def test_all_modules_are_validated_before_uploading(self):
        stats = (mapfile_parser.ProgressStats(decompedSize=16), {})
        with patch.object(progress, "getProgress", side_effect=[stats, FileNotFoundError("missing kernel")]), patch.object(
            mapfile_parser.frontends.upload_frogress, "uploadEntriesToFrogress"
        ) as upload:
            with self.assertRaises(FileNotFoundError):
                progress.processMapFiles(progress.MAP_FILES, "test-key")
            upload.assert_not_called()

    def test_badges_render_measured_percentages_and_byte_counts(self):
        results = [
            ("loader", (mapfile_parser.ProgressStats(decompedSize=16, undecompedSize=48), {})),
            ("kernel", (mapfile_parser.ProgressStats(decompedSize=32, undecompedSize=32), {})),
        ]
        with patch.object(progress, "ROOT", self.root):
            progress.writeProgressBadges(results)
        for category, label, percentage in [("loader", "SCPS_150.97", "25.00%"),
                                             ("kernel", "KERNEL.XFF", "50.00%")]:
            svg = ET.parse(self.root / f"docs/progress/{category}.svg").getroot()
            self.assertIn(f"{label}: {percentage}", svg.attrib["aria-label"])
            self.assertIn("/ 64 bytes reconstructed", svg.attrib["aria-label"])
            self.assertIn(percentage, "".join(svg.itertext()))

    def test_invalid_statistics_do_not_replace_existing_badges(self):
        directory = self.root / "docs/progress"
        directory.mkdir(parents=True)
        badge = directory / "loader.svg"
        badge.write_text("previous counter")
        results = [("loader", (mapfile_parser.ProgressStats(decompedSize=16), {})),
                   ("kernel", (mapfile_parser.ProgressStats(), {}))]
        with patch.object(progress, "ROOT", self.root):
            with self.assertRaises(ValueError):
                progress.writeProgressBadges(results)
        self.assertEqual(badge.read_text(), "previous counter")

    def test_update_badges_does_not_upload_or_require_a_key(self):
        args = argparse.Namespace(frogress_api_key=None, dry_run=False, update_badges=True)
        with patch.object(progress, "processMapFiles", return_value=[]) as calculate, patch.object(
            progress, "writeProgressBadges"
        ) as write:
            progress.main(args)
            calculate.assert_called_once_with(progress.MAP_FILES, None, dry_run=True)
            write.assert_called_once_with([])


if __name__ == "__main__":
    unittest.main()
