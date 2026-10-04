#!/usr/bin/env python3
"""Build a matching Preview disc and keep an auditable PCSX2 test record."""
import argparse
import configparser
import datetime
import hashlib
import json
import os
from pathlib import Path
import plistlib
import shlex
import shutil
import subprocess
import sys
import uuid

ROOT = Path(__file__).resolve().parents[1]
RUNS = ROOT / "target" / "pcsx2"
HASHES = {"SCPS_150.97": "c4d5576d1cae3721c411a746c7845f5c6f026dbb",
          "KERNEL.XFF": "ac00c937b84cbe2e283a97e59e09a11886118388"}
CHECKS = ("boot", "menu", "gameplay", "graphics_audio", "stability")
CONTROLLED = {"EnableCheats": "false", "EnablePatches": "false",
              "EnableWideScreenPatches": "false", "EnableNoInterlacingPatches": "false"}


def now():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def digest(path, algorithm="sha256"):
    value = hashlib.new(algorithm)
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def save(path, value):
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, indent=2) + "\n")
    temporary.replace(path)


def run_logged(command, log, timeout=None):
    print("+ " + shlex.join(map(str, command)), flush=True)
    with log.open("wb") as output:
        result = subprocess.run(list(map(str, command)), cwd=ROOT, stdout=output,
                                stderr=subprocess.STDOUT, timeout=timeout)
    if result.returncode:
        raise RuntimeError(f"Command failed ({result.returncode}); see {log}")


def preview_iso(explicit):
    if explicit:
        path = Path(explicit).expanduser().resolve()
        if not path.is_file():
            raise ValueError(f"ISO not found: {path}")
        return path
    candidates = list((ROOT / "target" / "downloads").glob("*.iso"))
    candidates += list((ROOT / "target" / "downloads").glob("*.ISO"))
    if len(candidates) != 1:
        raise ValueError("Specify the Preview disc with --iso or SOTC_PREVIEW_ISO.")
    return candidates[0].resolve()


def find_emulator(explicit):
    if explicit:
        candidate = Path(explicit).expanduser().resolve()
    else:
        candidate = next((Path(p) for name in ("pcsx2-qt", "PCSX2", "pcsx2")
                          if (p := shutil.which(name))), None)
        if candidate is None and sys.platform == "darwin":
            apps = []
            for directory in (Path("/Applications"), Path.home() / "Applications"):
                apps.extend(directory.glob("PCSX2*.app"))
                apps.extend(directory.glob("Emulatori/PCSX2*.app"))
            if len(apps) == 1:
                candidate = apps[0]
        # A locally downloaded emulator can be pinned without replacing an installed app.
        local_apps = list((ROOT / "target/pcsx2-downloads").glob("*/PCSX2*.app"))
        if not explicit and len(local_apps) == 1:
            candidate = local_apps[0]
        if candidate is None:
            raise ValueError("Specify PCSX2 with --pcsx2 or PCSX2_BIN.")
    if candidate.suffix == ".app":
        info = plistlib.loads((candidate / "Contents/Info.plist").read_bytes())
        candidate = candidate / "Contents/MacOS" / info["CFBundleExecutable"]
    if not candidate.is_file() or not os.access(candidate, os.X_OK):
        raise ValueError(f"PCSX2 executable not found: {candidate}")
    return candidate


def settings_root(explicit):
    if explicit:
        return Path(explicit).expanduser().resolve()
    if sys.platform == "darwin":
        return Path.home() / "Library/Application Support/PCSX2"
    return Path(os.environ.get("XDG_CONFIG_HOME", Path.home() / ".config")) / "PCSX2"


def isolated_settings(source, destination, keyboard=False):
    """Copy selected BIOS/config, never the user's cards, states, or patches."""
    config = configparser.ConfigParser(interpolation=None, strict=False)
    config.optionxform = str
    config.read(source / "inis/PCSX2.ini")
    config.remove_section("Achievements")
    bios_name = config.get("Filenames", "BIOS", fallback="")
    bios_directory = Path(config.get("Folders", "Bios", fallback="bios"))
    if not bios_directory.is_absolute():
        bios_directory = source / bios_directory
    bios = bios_directory / bios_name
    if not bios_name or not bios.is_file():
        raise ValueError(f"Configure your BIOS in PCSX2 first (settings: {source}).")
    if Path(bios_name).name != bios_name:
        raise ValueError("PCSX2 BIOS setting must be a filename.")
    (destination / "inis").mkdir(parents=True)
    (destination / "bios").mkdir()
    shutil.copy2(bios, destination / "bios" / bios_name)
    # Optional region NVM/MEC files accompany the selected BIOS.
    for suffix in (".NVM", ".nvm", ".mec", ".MEC"):
        extra = bios.with_suffix(suffix)
        if extra.is_file():
            shutil.copy2(extra, destination / "bios" / extra.name)
    overrides = {
        "UI": {"SetupWizardIncomplete": "false", "SettingsVersion": "1",
               "StartPaused": "false", "ConfirmShutdown": "false"},
        "Folders": {key: value for key, value in {
            "Bios": "bios", "Logs": "logs", "MemoryCards": "memcards",
            "Savestates": "sstates", "Snapshots": "snaps", "Patches": "patches",
            "Cheats": "cheats", "Cache": "cache", "Textures": "textures",
            "GameSettings": "gamesettings", "InputProfiles": "inputprofiles",
            "UserResources": "resources", "Videos": "videos", "Covers": "covers"
        }.items()},
        "Logging": {"EnableFileLogging": "true", "EnableEEConsole": "true",
                    "EnableIOPConsole": "true", "EnableTimestamps": "true"},
        "EmuCore": {"EnableCheats": "false", "EnablePatches": "false",
                    "EnableWideScreenPatches": "false", "EnableNoInterlacingPatches": "false",
                    "SaveStateOnShutdown": "false", "EnableFastBoot": "true"},
        "EmuCore/Speedhacks": {"EECycleRate": "0", "EECycleSkip": "0"},
        "Framerate": {"NominalScalar": "1"},
        "Achievements": {"Enabled": "false"},
        "MemoryCards": {"Slot1_Enable": "false", "Slot2_Enable": "false"},
        "GameList": {"Paths": "", "RecursivePaths": ""},
    }
    if keyboard:
        overrides["Pad1"] = {
            "Type": "DualShock2", "Up": "Keyboard/Up", "Down": "Keyboard/Down",
            "Left": "Keyboard/Left", "Right": "Keyboard/Right",
            "Cross": "Keyboard/K", "Square": "Keyboard/J", "Triangle": "Keyboard/I",
            "Circle": "Keyboard/L", "Start": "Keyboard/Return", "Select": "Keyboard/Backspace",
            "L1": "Keyboard/Q", "L2": "Keyboard/1", "R1": "Keyboard/E", "R2": "Keyboard/3",
            "L3": "Keyboard/2", "R3": "Keyboard/4", "LUp": "Keyboard/W",
            "LDown": "Keyboard/S", "LLeft": "Keyboard/A", "LRight": "Keyboard/D",
            "RUp": "Keyboard/T", "RDown": "Keyboard/G", "RLeft": "Keyboard/F",
            "RRight": "Keyboard/H", "LargeMotor": "", "SmallMotor": "", "Analog": "",
        }
    for section, values in overrides.items():
        if not config.has_section(section):
            config.add_section(section)
        for key, value in values.items():
            config.set(section, key, value)
    path = destination / "inis/PCSX2.ini"
    with path.open("w") as stream:
        config.write(stream)
    return {"source": str(source), "path": str(destination),
            "config_sha256": digest(path), "bios_sha256": digest(bios),
            "bios_filename": bios_name, "keyboard": keyboard}


def emulator_setup(executable, run_dir, source, keyboard=False):
    result = subprocess.run([str(executable), "-help"], capture_output=True,
                            text=True, timeout=30)
    help_text = result.stdout + result.stderr  # PCSX2 2.2 exits 1 for -help.
    (run_dir / "pcsx2-help.txt").write_text(help_text)
    if "-batch" not in help_text or "-logfile" not in help_text:
        raise ValueError("PCSX2 does not advertise the required batch/log options.")
    config_dir = run_dir / "emulator/PCSX2"
    settings = isolated_settings(source, config_dir, keyboard=keyboard)
    settings_args = []
    if "-datapath" in help_text:
        reject_portable_markers(executable)
        settings_args = ["-datapath", str(config_dir.parent)]
        isolated = executable
    elif sys.platform == "darwin" and executable.parent.name == "MacOS":
        # 2.2 predates -datapath. Portable mode uses the containing bundle's
        # directory; use a private copy, preserving the installed application.
        app = executable.parents[2]
        private_app = run_dir / "emulator" / app.name
        shutil.copytree(app, private_app, symlinks=True)
        isolated = private_app / executable.relative_to(app)
        (private_app.parent / "portable.txt").write_text(str(config_dir))
    elif sys.platform.startswith("linux"):
        reject_portable_markers(executable)
        isolated = executable
        settings["xdg_config_home"] = str(run_dir / "emulator")
    else:
        raise ValueError("This PCSX2 needs -datapath for isolated testing on this platform.")
    return {"source_executable": str(executable), "executable": str(isolated),
            "executable_sha256": digest(isolated), "settings_args": settings_args,
            "settings": settings}


def reject_portable_markers(executable):
    roots = [executable.parent]
    if sys.platform == "darwin" and executable.parent.name == "MacOS":
        roots.append(executable.parents[2].parent)
    for directory in roots:
        for name in ("portable.txt", "portable.ini"):
            if (directory / name).exists():
                raise ValueError(f"Portable marker overrides isolated settings: {directory / name}. "
                                 "Use a separate non-portable PCSX2 installation.")


def load_run(directory):
    if directory:
        run_dir = Path(directory).expanduser().resolve()
    else:
        run_dir = Path((RUNS / "latest.txt").read_text().strip())
    return run_dir, json.loads((run_dir / "result.json").read_text())


def prepare(args):
    iso = preview_iso(args.iso)
    executable = find_emulator(args.pcsx2)
    run_dir = RUNS / (datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
                      + "-" + uuid.uuid4().hex[:6])
    run_dir.mkdir(parents=True)
    manifest = {"schema_version": 1, "created_at": now(), "status": "preparing",
                "runtime": {"status": "pending", "checks": {c: "pending" for c in CHECKS}},
                "run_dir": str(run_dir)}
    save(run_dir / "result.json", manifest)
    try:
        for name, expected in HASHES.items():
            if digest(ROOT / "iso" / name, "sha1") != expected:
                raise ValueError(f"Wrong Preview input: iso/{name}")
        # Clean generated outputs: Ninja does not currently track C headers.
        command = [ROOT / "scripts/linux.sh", "bash", "-c",
                   "set -euo pipefail; ./configure.py -c -t loader; "
                   "./configure.py -c -t kernel; make all"]
        run_logged(command, run_dir / "build.log")
        run_logged([sys.executable, ROOT / "tools/verify_kernel.py", "--original",
                    ROOT / "iso/KERNEL.XFF", "--compiled", ROOT / "build/KERNEL.XFF/KERNEL.XFF.elf",
                    "--report", run_dir / "kernel-verification.json"], run_dir / "verification.log")
        manifest["matching"] = {}
        for name, expected in HASHES.items():
            built = ROOT / "build" / name / name
            if digest(built, "sha1") != expected:
                raise ValueError(f"Rebuilt binary does not match: {built}")
            (run_dir / "binaries").mkdir(exist_ok=True)
            pinned = run_dir / "binaries" / name
            shutil.copy2(built, pinned)
            manifest["matching"][name] = {"sha1": expected, "sha256": digest(pinned)}
        unpatched = run_dir / "binaries/KERNEL-unpatched.elf"
        shutil.copy2(ROOT / "build/KERNEL.XFF/KERNEL.XFF.elf", unpatched)
        manifest["unpatched_kernel_sha256"] = digest(unpatched)
        output = run_dir / "SOTC-Preview-rebuilt.iso"
        run_logged([sys.executable, ROOT / "tools/prepare_pcsx2_iso.py", "--iso", iso,
                    "--loader", run_dir / "binaries/SCPS_150.97",
                    "--kernel", run_dir / "binaries/KERNEL.XFF", "--output", output,
                    "--report", run_dir / "disc-verification.json"], run_dir / "disc.log")
        manifest["disc"] = {"source": str(iso), "path": str(output), "sha256": digest(output)}
        manifest["emulator"] = emulator_setup(executable, run_dir, settings_root(args.settings),
                                               keyboard=args.keyboard)
        manifest["status"] = "ready"
        save(run_dir / "result.json", manifest)
        (RUNS / "latest.txt").write_text(str(run_dir) + "\n")
        print(f"Prepared: {run_dir}\nMatching passed; PCSX2 runtime test remains pending.")
    except Exception as exc:
        manifest.update(status="preparation_failed", error=str(exc))
        save(run_dir / "result.json", manifest)
        raise


def check_pinned(run_dir, manifest):
    if manifest.get("status") not in {"ready", "runtime_pending", "runtime_passed", "runtime_failed"}:
        raise ValueError("This run did not finish preparation.")
    if digest(manifest["disc"]["path"]) != manifest["disc"]["sha256"]:
        raise ValueError("Prepared disc changed; prepare a new run.")
    emulator = manifest["emulator"]
    if digest(emulator["executable"]) != emulator["executable_sha256"]:
        raise ValueError("Emulator executable changed; prepare a new run.")
    settings = Path(emulator["settings"]["path"])
    if digest(settings / "bios" / emulator["settings"]["bios_filename"]) != emulator["settings"]["bios_sha256"]:
        raise ValueError("Test BIOS changed; prepare a new run.")
    config = configparser.ConfigParser(interpolation=None, strict=False)
    config.read(settings / "inis/PCSX2.ini")
    for key, expected in CONTROLLED.items():
        if config.get("EmuCore", key, fallback="").lower() != expected:
            raise ValueError(f"Test configuration changed: {key} must be {expected}.")


def launch(args):
    run_dir, manifest = load_run(args.run_dir)
    check_pinned(run_dir, manifest)
    emulator = manifest["emulator"]
    command = [emulator["executable"], *emulator["settings_args"], "-batch", "-nofullscreen",
               "-logfile", str(run_dir / "emulog.txt"), "--", manifest["disc"]["path"]]
    if args.check_config:
        command = [emulator["executable"], *emulator["settings_args"], "-testconfig",
                   "-logfile", str(run_dir / "config-check.log")]
    env = os.environ.copy()
    if "xdg_config_home" in emulator["settings"]:
        env["XDG_CONFIG_HOME"] = emulator["settings"]["xdg_config_home"]
    if args.print_command:
        prefix = []
        if "xdg_config_home" in emulator["settings"]:
            prefix = ["env", "XDG_CONFIG_HOME=" + emulator["settings"]["xdg_config_home"]]
        print(shlex.join(prefix + command))
        return
    if args.check_config:
        result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, timeout=30)
        (run_dir / "config-check-console.log").write_bytes(result.stdout + result.stderr)
        if result.returncode:
            raise RuntimeError(f"PCSX2 configuration check failed: {run_dir / 'config-check-console.log'}")
        logs = result.stdout + result.stderr
        if (run_dir / "config-check.log").is_file():
            logs += (run_dir / "config-check.log").read_bytes()
        expected = str(Path(emulator["settings"]["path"]) / "inis/PCSX2.ini").encode()
        if expected not in logs:
            raise RuntimeError("PCSX2 did not confirm the isolated configuration path; see config-check logs.")
        manifest["emulator"]["config_check"] = {"at": now(), "exit_code": result.returncode}
        save(run_dir / "result.json", manifest)
        print("PCSX2 configuration initialized. Runtime check remains pending.")
        return
    if not emulator.get("config_check"):
        args.check_config = True
        launch(args)
        args.check_config = False
        run_dir, manifest = load_run(str(run_dir))
        emulator = manifest["emulator"]
    if manifest["runtime"].get("launched_at"):
        raise ValueError("This run was already launched. Prepare a new run to preserve its evidence.")
    manifest["runtime"] = {"status": "pending", "launched_at": now(), "command": command,
                           "checks": {c: "pending" for c in CHECKS}}
    manifest["status"] = "runtime_pending"
    settings_file = Path(emulator["settings"]["path"]) / "inis/PCSX2.ini"
    shutil.copy2(settings_file, run_dir / "PCSX2-before-run.ini")
    with (run_dir / "pcsx2-console.log").open("wb") as stream:
        child = subprocess.Popen(command, cwd=ROOT, env=env, stdout=stream, stderr=subprocess.STDOUT,
                                 start_new_session=True)
    manifest["runtime"]["pid"] = child.pid
    save(run_dir / "result.json", manifest)
    print(f"PCSX2 launched (PID {child.pid}). Test outcome is pending.\nLogs: {run_dir}")


def record(args):
    run_dir, manifest = load_run(args.run_dir)
    check_pinned(run_dir, manifest)
    if not manifest["runtime"].get("launched_at"):
        raise ValueError("Launch this run before recording observations.")
    checks = {check: getattr(args, check) for check in CHECKS}
    all_pass = all(value == "pass" for value in checks.values())
    if all_pass and not args.evidence:
        raise ValueError("A passing check needs screenshot/video evidence (--evidence FILE).")
    evidence = []
    evidence_dir = run_dir / "evidence"
    evidence_dir.mkdir(exist_ok=True)
    for index, filename in enumerate(args.evidence or []):
        source = Path(filename).expanduser().resolve()
        if not source.is_file():
            raise ValueError(f"Evidence file not found: {source}")
        destination = evidence_dir / f"{index:02d}-{source.name}"
        shutil.copy2(source, destination)
        evidence.append({"path": str(destination), "sha256": digest(destination)})
    log = run_dir / "emulog.txt"
    if all_pass and (not log.is_file() or not log.stat().st_size):
        raise ValueError("PCSX2 log is missing; cannot record a passing check.")
    status = "manual_pass" if all_pass else ("fail" if "fail" in checks.values() else "pending")
    manifest["runtime"].update(status=status, observed_at=now(), checks=checks,
                               notes=args.notes, evidence=evidence)
    if log.is_file():
        shutil.copy2(log, run_dir / "emulog-observed.txt")
        manifest["runtime"]["log_sha256"] = digest(run_dir / "emulog-observed.txt")
    manifest["status"] = {"manual_pass": "runtime_passed", "fail": "runtime_failed",
                          "pending": "runtime_pending"}[status]
    save(run_dir / "result.json", manifest)
    print(f"Recorded runtime outcome: {status}\n{run_dir / 'result.json'}")
    return 1 if status == "fail" else 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    p = sub.add_parser("prepare", help="Clean build, verify, copy Preview disc, prepare isolated PCSX2.")
    p.add_argument("--iso", default=os.environ.get("SOTC_PREVIEW_ISO"))
    p.add_argument("--pcsx2", default=os.environ.get("PCSX2_BIN"))
    p.add_argument("--settings", default=os.environ.get("PCSX2_SETTINGS_DIR"))
    p.add_argument("--keyboard", action="store_true", help="Use keyboard controls in the isolated test profile.")
    p.set_defaults(function=prepare)
    p = sub.add_parser("run", help="Launch the last prepared run; it remains pending until observed.")
    p.add_argument("--run-dir")
    p.add_argument("--print-command", action="store_true")
    p.add_argument("--check-config", action="store_true", help="Initialize config without booting a game.")
    p.set_defaults(function=launch)
    p = sub.add_parser("record", help="Record observed checkpoints with evidence, never auto-pass on launch.")
    p.add_argument("--run-dir")
    for check in CHECKS:
        p.add_argument("--" + check.replace("_", "-"), choices=("pass", "fail", "pending"), required=True)
    p.add_argument("--notes", required=True)
    p.add_argument("--evidence", action="append")
    p.set_defaults(function=record)
    args = parser.parse_args(argv)
    try:
        return args.function(args) or 0
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError, configparser.Error) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
