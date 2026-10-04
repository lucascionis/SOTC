# Shadow of the Colossus Decompilation Project
[loader-progress-badge]: https://img.shields.io/endpoint?url=https%3A%2F%2Fprogress.deco.mp%2Fdata%2Fsotc%2Fpreview%2Floader%2F%3Fmode%3Dshield%26measure%3Dloader&style=flat&label=SCPS_150.97
[kernel-progress-badge]: https://img.shields.io/endpoint?url=https%3A%2F%2Fprogress.deco.mp%2Fdata%2Fsotc%2Fpreview%2Fkernel%2F%3Fmode%3Dshield%26measure%3Dkernel&style=flat&label=KERNEL.XFF

<!-- Contributors shield -->
[contributors-url]: https://github.com/Fantaskink/SOTC/graphs/contributors
[contributors-badge]: https://img.shields.io/github/contributors/Fantaskink/SOTC?color=green

<!-- Shields -->
![loader-progress-badge] ![kernel-progress-badge] [![Contributors][contributors-badge]][contributors-url]

We are currently targeting the Preview Version's main ELF (``SCPS_150.97``, sha1: ``c4d5576d1cae3721c411a746c7845f5c6f026dbb``).

<a href="https://discord.gg/WGSnPQFtHU" target="_blank">
  <img src="https://discord.com/api/guilds/465610776762384394/widget.png?style=banner2" alt="Discord Banner">
</a>

## Getting Started
### Requirements
**Disclaimer**: Linux or WSL2 required
1. ``sudo apt install gcc-mipsel-linux-gnu binutils-mips-linux-gnu ninja-build``
2. ``sudo apt install python3-pip python3-venv``
3. ``python3 -m venv ./.python3``
4. ``source .python3/bin/activate``
3. ``python3 -m pip install -U -r requirements.txt``
4. ``scripts/setup_compiler.sh``

(NOTE: If you have problems when compiling try installing ``gcc-multilib``)

### Setup
1. Create an ``iso`` directory in the root directory
2. From a Shadow of the Colossus Preview Version ISO, extract ``SCPS_150.97`` and ``KERNEL.XFF`` into the ``iso`` directory
3. Just run ``make`` to build all modules (``make loader`` or ``make kernel`` to build just one module. ``make clean`` also works)

### Build and test in PCSX2

The local pipeline performs a clean build, checks matching before XFF patching,
and creates a separate Preview ISO containing the rebuilt loader and kernel.
It launches PCSX2 with isolated settings and keeps logs and an observed test
result. See [the PCSX2 test guide](docs/PCSX2_TESTING.md).

```sh
export SOTC_PREVIEW_ISO="/path/to/SotC Preview.iso"
export PCSX2_BIN="/path/to/PCSX2.app" # or pcsx2-qt executable
make pcsx2-prepare
make pcsx2-check-config
make pcsx2-run
```

`make pcsx2` prepares and launches in one command. A successful build or launch
does not mark the runtime test as passed; checkpoints must be observed and
recorded. The existing GitHub workflow checks the build and matching; PCSX2
execution runs locally with your configured BIOS and Preview disc.

## Contributing
Use our [contribution guide](docs/CONTRIBUTING.md).
