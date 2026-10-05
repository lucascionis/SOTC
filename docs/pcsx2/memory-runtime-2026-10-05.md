# PCSX2 test after completing memory.c — 2026-10-05

The rebuilt Preview passed the observed boot, menu, gameplay and stability
checkpoints on PCSX2 2.8.2. The overall pipeline result remains `runtime_pending`
because the combined graphics/audio checkpoint has not been fully observed.

## Build and execution

`python3 scripts/pcsx2_check.py prepare --keyboard` rebuilt the loader and kernel,
checked matching, verified the replacement files in a new disc image, and created
an isolated emulator profile. Kernel SHA1:
`ac00c937b84cbe2e283a97e59e09a11886118388`.
The unpatched linked ELF check found zero opcode/register differences over
52,419 words; 1,405 relocation destinations were checked and 4,730 remain
outside that verifier's coverage. The separate memory object validation is
documented in [MEMORY_DECOMP.md](../MEMORY_DECOMP.md).

The profile configuration check passed. The initial cold launch reached the
memory-card notice. Loading the native input movie reset the VM and used fast
boot; this was not a second cold BIOS boot. Cheats, external patches, save states
and memory cards were disabled. All test files stay local.

The movie was freshly generated from `runtime-preview-plan.json`. It completed
all 36,000 frames without amendments. Its SHA256 is
`1a0217e5093b7d8decc9f5aec246cbc330b9bdd6684dd8a6e5b94ca621800f6f`.
Source fingerprints for `memory.c`, its header, `isys/module.c`, and the input
plan were unchanged at the end of the run.

## Observations

| Checkpoint | Result | Evidence |
| --- | --- | --- |
| Boot | Pass | Memory-card menu appeared and Continue responded to recorded input. |
| Menu | Pass | The replay entered gameplay; map/options later responded to keyboard input. |
| Gameplay | Pass | Temple loaded; movement, camera rotation, sword raising, HUD and world map were observed. |
| Graphics/audio | Pending | Scene/UI rendered; stereo audio was recorded and decoded, but no subjective listening was performed. |
| Stability | Pass | Gameplay screenshots span 17:24:48–17:33:34 UTC (526 seconds); no crash or hang was observed. Normal shutdown was logged. |

Wander's local position changed from `(0, 11.3, -21.9)` to
`(-11.3, 10.1, -6.4)`, then through other temple positions. Agro was rendered,
but mounting/riding was not confirmed in this run. The fixed route reached
walls and columns, so the previous horse sequence is not a reproducible check
yet. At the end, the movie displayed the world map and paused at frame 36,000.
After stopping replay, the map/options accepted keyboard input; returning from
the map to the temple was not confirmed before normal emulator shutdown.

The 127.232-second native video contains H.264 at 640×480 and stereo AAC at
48 kHz. Full audio/video decoding completed without errors. The audio analysis
found 12,216,192 decoded samples, mean volume -28.3 dB and peak -6.4 dB.
These measurements establish an active audio signal, not perceptual correctness.

## Evidence and limits

The retained local run is `target/pcsx2/20261005T171925Z-3ad332/`:

- `result.json`: checkpoint results, pinned binary/disc hashes and hashed evidence.
- `runtime-review.json`: observations, source fingerprints and limitations.
- `evidence/`: copies of screenshots, native video, input movie and audio analysis.
- `emulog-observed.txt`: emulator log preserved after normal shutdown.

The log contains 18,787 repeated Metal uneven-page warnings. They were also
present in the earlier session; no controlled renderer comparison was performed.
Combat, colossi, an outdoor scene transition, saving/loading and a full
playthrough were not covered. This is a runtime smoke test after the memory
changes, with the audio checkpoint still pending.
