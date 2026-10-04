import argparse
import configparser
import contextlib
import io
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import subprocess

spec = importlib.util.spec_from_file_location("pcsx2_check", Path(__file__).resolve().parents[2] / "scripts/pcsx2_check.py")
pipeline = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pipeline)


class PipelineTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def source_config(self):
        source = self.root / "user"
        (source / "inis").mkdir(parents=True)
        (source / "bios").mkdir()
        (source / "bios/ps2.bin").write_bytes(b"test bios")
        (source / "inis/PCSX2.ini").write_text(
            "[Filenames]\nBIOS=ps2.bin\n[Folders]\nBios=bios\n"
            "MemoryCards=/existing/cards\n[EmuCore]\nEnableCheats=true\n"
            "EnablePatches=true\n[Achievements]\nToken=private\n")
        return source

    def prepared_run(self):
        directory = self.root / "run"
        directory.mkdir()
        settings = pipeline.isolated_settings(self.source_config(), directory / "profile")
        disc = directory / "game.iso"
        disc.write_bytes(b"prepared test disc")
        emulator = directory / "pcsx2"
        emulator.write_bytes(b"test emulator")
        manifest = {"status": "ready", "disc": {"path": str(disc), "sha256": pipeline.digest(disc)},
                    "emulator": {"executable": str(emulator),
                                 "executable_sha256": pipeline.digest(emulator), "settings": settings,
                                 "settings_args": []},
                    "runtime": {"status": "pending", "checks": {c: "pending" for c in pipeline.CHECKS}}}
        pipeline.save(directory / "result.json", manifest)
        return directory, manifest

    def arguments(self, directory, value="pass", evidence=None):
        args = argparse.Namespace(run_dir=str(directory), notes="Observed test", evidence=evidence)
        for check in pipeline.CHECKS:
            setattr(args, check, value)
        return args

    def test_isolated_config_preserves_user_and_disables_modifications(self):
        source = self.source_config()
        original = (source / "inis/PCSX2.ini").read_bytes()
        destination = self.root / "private"
        result = pipeline.isolated_settings(source, destination)
        config = configparser.ConfigParser()
        config.read(destination / "inis/PCSX2.ini")
        for key in pipeline.CONTROLLED:
            self.assertFalse(config.getboolean("EmuCore", key))
        self.assertFalse(config.getboolean("MemoryCards", "Slot1_Enable"))
        self.assertFalse(config.has_option("Achievements", "Token"))
        self.assertEqual(config.get("Folders", "MemoryCards"), "memcards")
        self.assertEqual((source / "inis/PCSX2.ini").read_bytes(), original)
        self.assertEqual(result["bios_sha256"], pipeline.digest(source / "bios/ps2.bin"))

    def test_missing_bios_rejected(self):
        with self.assertRaisesRegex(ValueError, "BIOS"):
            pipeline.isolated_settings(self.root / "missing", self.root / "private")

    def test_keyboard_profile_keeps_user_config_unchanged(self):
        source = self.source_config()
        original = (source / "inis/PCSX2.ini").read_bytes()
        destination = self.root / "keyboard-profile"
        result = pipeline.isolated_settings(source, destination, keyboard=True)
        config = configparser.ConfigParser()
        config.read(destination / "inis/PCSX2.ini")
        self.assertEqual(config.get("Pad1", "Cross"), "Keyboard/K")
        self.assertEqual(config.get("Pad1", "LUp"), "Keyboard/W")
        self.assertEqual(config.get("Pad1", "RRight"), "Keyboard/H")
        self.assertTrue(result["keyboard"])
        self.assertEqual((source / "inis/PCSX2.ini").read_bytes(), original)

    def test_modern_datapath_uses_parent_of_pcsx2_profile(self):
        run = self.root / "modern-run"
        run.mkdir()
        executable = self.root / "pcsx2"
        executable.write_bytes(b"emulator")
        response = subprocess.CompletedProcess([], 1, "-batch -logfile -datapath", "")
        with patch.object(pipeline.subprocess, "run", return_value=response):
            result = pipeline.emulator_setup(executable, run, self.source_config())
        datapath = Path(result["settings_args"][1])
        self.assertEqual(datapath / "PCSX2", Path(result["settings"]["path"]))
        self.assertTrue((datapath / "PCSX2/inis/PCSX2.ini").is_file())

    def test_portable_marker_rejected_for_custom_datapath(self):
        executable = self.root / "pcsx2"
        (self.root / "portable.txt").write_text("user-profile")
        with self.assertRaisesRegex(ValueError, "Portable marker"):
            pipeline.reject_portable_markers(executable)

    def test_changed_disc_rejected(self):
        directory, manifest = self.prepared_run()
        Path(manifest["disc"]["path"]).write_bytes(b"modified")
        with self.assertRaisesRegex(ValueError, "disc changed"):
            pipeline.check_pinned(directory, manifest)

    def test_enabled_patch_rejected(self):
        directory, manifest = self.prepared_run()
        path = Path(manifest["emulator"]["settings"]["path"]) / "inis/PCSX2.ini"
        path.write_text(path.read_text().replace("EnablePatches = false", "EnablePatches = true"))
        with self.assertRaisesRegex(ValueError, "EnablePatches"):
            pipeline.check_pinned(directory, manifest)

    def test_no_pass_without_launch(self):
        directory, _ = self.prepared_run()
        with self.assertRaisesRegex(ValueError, "Launch"):
            pipeline.record(self.arguments(directory))

    def test_print_command_includes_linux_profile_environment(self):
        directory, manifest = self.prepared_run()
        manifest["emulator"]["settings"]["xdg_config_home"] = "/path with spaces/profile"
        pipeline.save(directory / "result.json", manifest)
        args = argparse.Namespace(run_dir=str(directory), check_config=False, print_command=True)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            pipeline.launch(args)
        self.assertIn("'XDG_CONFIG_HOME=/path with spaces/profile'", output.getvalue())

    def test_no_pass_without_evidence(self):
        directory, manifest = self.prepared_run()
        manifest["runtime"]["launched_at"] = pipeline.now()
        pipeline.save(directory / "result.json", manifest)
        with self.assertRaisesRegex(ValueError, "evidence"):
            pipeline.record(self.arguments(directory))

    def test_launch_does_not_imply_runtime_pass(self):
        directory, manifest = self.prepared_run()
        manifest["runtime"]["launched_at"] = pipeline.now()
        pipeline.save(directory / "result.json", manifest)
        with contextlib.redirect_stdout(io.StringIO()):
            pipeline.record(self.arguments(directory, "pending"))
        result = json.loads((directory / "result.json").read_text())
        self.assertEqual(result["status"], "runtime_pending")

    def test_manual_pass_records_evidence_and_log_hash(self):
        directory, manifest = self.prepared_run()
        manifest["runtime"]["launched_at"] = pipeline.now()
        pipeline.save(directory / "result.json", manifest)
        (directory / "emulog.txt").write_text("test log")
        evidence = self.root / "screenshot.png"
        evidence.write_bytes(b"test evidence")
        with contextlib.redirect_stdout(io.StringIO()):
            pipeline.record(self.arguments(directory, evidence=[str(evidence)]))
        result = json.loads((directory / "result.json").read_text())
        self.assertEqual(result["runtime"]["status"], "manual_pass")
        self.assertEqual(result["runtime"]["evidence"][0]["sha256"], pipeline.digest(evidence))
        self.assertEqual(result["runtime"]["log_sha256"], pipeline.digest(directory / "emulog-observed.txt"))


if __name__ == "__main__":
    unittest.main()
