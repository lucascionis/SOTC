# Testing the rebuilt Preview in PCSX2

The pipeline checks the reconstructed PS2 binaries, prepares a disc containing
them, and launches that disc in PCSX2. It combines automated matching checks
with an observed runtime check. It does not automatically declare gameplay
correct because the emulator process started or exited successfully.

## Requirements

- Docker Desktop (macOS) or Docker Engine (Linux), running.
- The compiler and build dependencies described in the project README. The
  Docker wrapper can provision them on its first image build.
- Python 3.9 or later on the host.
- PCSX2 with a BIOS already configured in your own installation.
- The **Preview** ISO, plus the original `SCPS_150.97` and `KERNEL.XFF` files in
  `iso/`. A PAL retail disc cannot substitute for the Preview.
- A graphical desktop for the runtime check, and enough free space for a copy
  of the disc and the test emulator profile.

The pipeline never downloads a game image or BIOS. The source ISO, normal
PCSX2 installation, memory cards and save states are preserved.

## Prepare and launch

```sh
export SOTC_PREVIEW_ISO="/path/to/SotC Preview.iso"
export PCSX2_BIN="/path/to/PCSX2.app" # macOS bundle or executable
make pcsx2-prepare
make pcsx2-check-config
make pcsx2-run
```

Alternatively, `make pcsx2` prepares and launches in one command.
`PCSX2_SETTINGS_DIR` can select an existing PCSX2 data directory containing
`inis/PCSX2.ini` and your BIOS. The wrapper discovers standard macOS/Linux
settings locations and can discover a single PCSX2 application on this Mac.
It also finds a single `.iso`/`.ISO` in `target/downloads` when no ISO is given.
When a single downloaded macOS app exists under `target/pcsx2-downloads/VERSION`,
it is preferred over installed apps unless `PCSX2_BIN` is explicitly set.
Explicit paths are preferable when several installations or discs exist.

The reproducible Linux build environment is in `scripts/Dockerfile`:

```sh
scripts/linux.sh --build-image
scripts/linux.sh make all
```

`SOTC_BUILD_IMAGE` selects an already prepared image. On the current development
Mac, `SOTC_BUILD_IMAGE=sotc-getting-started:local` reuses the working local image.

## What preparation checks

1. Original build inputs have the expected Preview SHA-1 hashes.
2. Loader and kernel are rebuilt from clean generated outputs. This matters
   because the current Ninja rules do not track every header dependency.
3. `tools/verify_kernel.py` compares the original kernel text with the linked
   ELF **before** `fix_xff.py` restores relocation words. Its JSON report records
   instruction and relocation checks, including any unsupported address cases.
4. Both final binaries have the original SHA-1 hashes.
5. `tools/prepare_pcsx2_iso.py` finds the files in the ISO9660 directory, checks
   their hashes and sizes, and replaces their extents in a new disc copy. It
   keeps file locations and directory records intact, rereads the replaced
   extents, and verifies the copy. Unsupported layouts and overwrites are
   rejected. The source disc is never edited.
6. The runner pins the rebuilt binaries and prepared disc hashes and creates
   a private emulator profile with BIOS, logging, and controller settings.
   Cheats, external patches and save-state loading are excluded. Memory cards
   are disabled for this initial test.

Modern PCSX2 uses `-datapath` for isolation. PCSX2 2.2 on macOS uses a private
copy of the app and its supported `portable.txt` mechanism. Linux 2.2 uses
`XDG_CONFIG_HOME`. The runner uses documented batch/log options and starts the
prepared disc directly, so the loader reads the rebuilt kernel from the disc.
See the [official PCSX2 CLI documentation](https://pcsx2.net/docs/advanced/cli/)
and [2.2 folder handling](https://github.com/PCSX2/pcsx2/blob/v2.2.0/pcsx2/Pcsx2Config.cpp).

Every preparation creates a directory under `target/pcsx2/`; `latest.txt` points
to the latest successful preparation. The directory contains `result.json`,
build/verification logs, the prepared ISO, pinned binaries and emulator profile.
`run --run-dir DIRECTORY` selects an earlier run; runs can only be launched
once so their original logs/evidence are not overwritten.

## Observe and record the runtime check

Boot from the disc, without a save state. For the initial acceptance check:

The isolated profile has no memory card attached. At the game's memory-card
notice, select **Continue** to proceed without saving. Use your configured
controller (bindings are copied from your PCSX2 profile).

For a keyboard-only test, prepare with `python3 scripts/pcsx2_check.py prepare --keyboard`, then `make pcsx2-run`. Controls: arrows for menus, Enter for Start,
K for Cross, J for Square, I for Triangle, L for Circle, WASD for movement,
TFGH for camera, Q/E for L1/R1, and 1/3 for L2/R2. This changes only the private
test profile.

- **boot:** the game boots without an error or crash.
- **menu:** the title/menu responds to input and starts a new game.
- **gameplay:** the playable scene loads; movement and camera respond.
- **graphics_audio:** scene/UI rendering and audio work without obvious regressions.
- **stability:** play for at least five minutes, including a scene transition
  or other loading event where available, without a hang or crash.

Capture screenshots with PCSX2's screenshot command (normally F8), or record
a short video. Close the emulation normally to flush its log, then record your
observations with actual evidence and specific notes:

```sh
python3 scripts/pcsx2_check.py record \
  --boot pass --menu pass --gameplay pass \
  --graphics-audio pass --stability pass \
  --notes "Played 5 minutes; movement, camera and loading checked." \
  --evidence "/path/to/gameplay-screenshot.png"
```

Use `fail` for a failed checkpoint and `pending` for anything not observed.
All checkpoints are required. A passing result requires evidence and a nonempty
PCSX2 log; it is recorded explicitly as `manual_pass`. Process start, a
configuration check or absence of log errors is never sufficient for a pass.
The record identifies the prepared disc, binary hashes, emulator executable,
BIOS, configuration, logs and observations. This is a reproducible smoke test,
not exhaustive coverage of the whole game or a replacement for binary matching.

## CI

GitHub Actions builds the two binaries, checks their final hashes, tests the
pipeline tools, and runs the kernel verification before XFF patching. Its
verification JSON is uploaded as an artifact. Disc/BIOS files and emulator data
are not uploaded. The graphical PCSX2 check is local; CI does not claim a
runtime pass.

Run the tools' tests with `make test-pipeline`.

## Observed local run: 2026-10-04

The clean rebuilt Preview was tested on PCSX2 2.8.2 with an isolated profile,
without cheats, external patches or save states. Boot, menu input, playable
temple loading, movement, camera rotation, mounting/riding Agro and opening the
world map were observed. Gameplay screenshots span 8 minutes 31 seconds; no
hang or crash was observed, and the emulator was closed normally. All 75
pipeline tests passed.

The run is retained at `target/pcsx2/20261004T145000Z-f9b33f/`. Its `result.json`
records boot, menu, gameplay and stability as passing, with `graphics_audio`
pending. The 115.52-second video contains a successfully decoded, nonzero stereo
audio signal; subjective listening and comparison against the original game
were not performed. Repeated Metal uneven-page warnings remain in the log.
Combat, colossi, outdoor areas and a full playthrough were not covered.
`runtime-review.json` records the observations and limitations, and `evidence/`
contains hashed copies of the video, screenshots, input movie and analysis.

The durable input plan is `docs/pcsx2/runtime-preview-plan.json`:

```sh
python3 tools/pcsx2_input_movie.py create \
  docs/pcsx2/runtime-preview-plan.json /tmp/sotc-preview-replay.p2m2
```

The generated movie matches the tested header and all 36,000 declared input
frames byte for byte. The tested file also contains 36 bytes beyond those
declared frames; the generated file omits that trailing data. This run amended
future inputs during playback; a fresh complete playback of the regenerated
movie still needs observation before claiming repeatability of the outcome.

## Native input replay

For repeatable controller input, use `tools/pcsx2_input_movie.py` with a durable JSON plan such as `{"frame_count":18000,"fps":60,"game_name":"Shadow of the Colossus","segments":[{"start_frame":600,"end_frame":606,"buttons":["Cross"]},{"start_seconds":20,"end_seconds":25,"left":[127,0],"right":[255,127]}]}`. Run `python3 tools/pcsx2_input_movie.py create plan.json replay.p2m2`; creation refuses to overwrite an existing file. PCSX2 2.8.2 requires the `.p2m2` extension in its file picker even though the binary movie header is version 1. Load the movie with **Tools → Input Recording → Play**, then toggle **System → Pause** to resume its initially paused replay; controller bindings need no changes. Frame indices and `port` are zero-based (`port` defaults to 0), segment ends are exclusive, and seconds are rounded to the nearest frame with half-frame ties rounded up. Analog `left`/`right` values are `[x,y]` bytes from 0 to 255, centered at `[127,127]`; named buttons are case-insensitive. A segment describes the complete pad state, and gaps are neutral; overlapping segments on the same port are rejected. To amend a movie already open for replay, supply a plan with the same `frame_count` and only future segments: `python3 tools/pcsx2_input_movie.py amend future.json replay.p2m2 --current-frame 900 --lead-frames 120`. The caller must observe the last consumed frame accurately and allow enough time for writes and emulator buffering; pause replay before updating if uncertain. The default margin is 120 frames. Amendments retain the file inode, header, total size, other controller, and every frame outside the supplied segments; they do not change metadata or load a save state. Both commands print movie and plan SHA-256 hashes for the test record. The wire layout follows the official [InputRecordingFile](https://github.com/PCSX2/pcsx2/blob/v2.8.2/pcsx2/Recording/InputRecordingFile.cpp) and [PadData](https://github.com/PCSX2/pcsx2/blob/v2.8.2/pcsx2/Recording/PadData.cpp) sources. Observe each replay visually and record the checkpoints above: successful file generation or replay commands never establish a runtime pass.
