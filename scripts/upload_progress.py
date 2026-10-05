"""
Script for uploading progress to the frogress.

Adapted from https://github.com/zeldaret/af/blob/aeb01dcb95e8281f89f355604dbeba519ef073d5/tools/progress.py
MIT License: https://opensource.org/license/mit
"""
from pathlib import Path
import argparse
import mapfile_parser

ROOT = Path(__file__).resolve().parents[1]
ASMPATH = ROOT / "asm"
NONMATCHINGS = "nonmatchings"
BASE_URL = "https://progress.deco.mp"
SLUG = "sotc"
VERSION = "preview"
MAP_FILES = [
    ("build/SCPS_150.97/SCPS_150.97.map", "SCPS_150.97", "loader"),
    ("build/KERNEL.XFF/KERNEL.XFF.map", "KERNEL.XFF", "kernel"),
]

def getProgressFromMapFile(mapFile: mapfile_parser.MapFile, asmPath: Path, nonmatchings: Path, aliases: dict[str, str] | None = None) -> tuple[mapfile_parser.ProgressStats, dict[str, mapfile_parser.ProgressStats]]:
    for directory in (asmPath, nonmatchings):
        if not directory.is_dir():
            raise FileNotFoundError(f"Missing assembly directory: {directory}. Run make first.")
    aliases = aliases or {}
    totalStats = mapfile_parser.ProgressStats()
    progressPerFolder: dict[str, mapfile_parser.ProgressStats] = dict()

    for segment in mapFile:
        for file in segment:
            if len(file) == 0:
                continue

            # Objects are build/<module>/{src,asm}/<module>/<source>.{c,s}.o.
            # Keep only the source path within the module, including dotted names.
            objectPath = Path(file.filepath)
            for kind in ("src", "asm"):
                prefix = Path("build") / asmPath.name / kind / asmPath.name
                if objectPath.is_relative_to(prefix):
                    originalFilePath = objectPath.relative_to(prefix)
                    break
            else:
                raise ValueError(f"Unexpected object path in map: {objectPath}")
            if originalFilePath.suffixes[-2:] not in ([".c", ".o"], [".s", ".o"]):
                raise ValueError(f"Unexpected object suffix in map: {objectPath}")
            extensionlessFilePath = originalFilePath.with_suffix("").with_suffix("")
            folder = extensionlessFilePath.parts[0]

            if ".a" in folder:
                folder = folder.split('.a')[0]

            if folder in aliases:
                folder = aliases[folder]

            if folder not in progressPerFolder:
                progressPerFolder[folder] = mapfile_parser.ProgressStats()

            # Startup assembly is handwritten and intentionally considered complete.
            wholeFileIsUndecomped = kind == "asm" and extensionlessFilePath != Path("sdk/crt0")

            for func in file:
                funcAsmPath = nonmatchings / extensionlessFilePath / f"{func.name}.s"

                symSize = 0
                if func.size is not None:
                    symSize = func.size

                if wholeFileIsUndecomped:
                    totalStats.undecompedSize += symSize
                    progressPerFolder[folder].undecompedSize += symSize
                elif funcAsmPath.exists():
                    totalStats.undecompedSize += symSize
                    progressPerFolder[folder].undecompedSize += symSize
                else:
                    totalStats.decompedSize += symSize
                    progressPerFolder[folder].decompedSize += symSize

    return totalStats, progressPerFolder

def getProgress(mapPath: str, asmPath: str) -> tuple[mapfile_parser.ProgressStats, dict[str, mapfile_parser.ProgressStats]]:
    """
    Gets the progress of the project using the mapfile parser.
    """
    mapPath = ROOT / mapPath
    if not mapPath.is_file():
        raise FileNotFoundError(f"Missing map file: {mapPath}. Run make first.")
    mapFile = mapfile_parser.MapFile()
    mapFile.readMapFile(mapPath)

    for segment in mapFile:
        for file in segment:
            if len(file) == 0:
                continue

            filepathParts = list(file.filepath.parts)
            file.filepath = Path(*filepathParts)

    asmPath = ASMPATH / Path(asmPath)

    nonMatchingsPath = asmPath / NONMATCHINGS

    print(f"ASM path: {asmPath}")
    print(f"Nonmatchings path: {nonMatchingsPath}")

    progress = getProgressFromMapFile(mapFile.filterBySectionType(".text"), asmPath, nonMatchingsPath)
    if progress[0].total == 0:
        raise ValueError(f"No code symbols found in map: {mapPath}")

    return progress

def processMapFiles(mapFiles: list[tuple[str, str, str]], frogress_api_key: str | None, dry_run: bool = False) -> None:
    """
    Processes a list of map files and uploads their progress to frogress.
    """

    # Validate every module before publishing any result.
    results = [(category, getProgress(mapPath, asmDir))
               for mapPath, asmDir, category in mapFiles]
    for category, (codeTotalStats, codeProgressPerFolder) in results:
        print(f"Progress: {category}")
        codeEntries = mapfile_parser.frontends.upload_frogress.getFrogressEntriesFromStats(
            codeTotalStats, codeProgressPerFolder, verbose=True
        )

        url = mapfile_parser.utils.generateFrogressEndpointUrl(BASE_URL, SLUG, VERSION)

        # Service categories are independent of the on-disk assembly directories.
        if not dry_run:
            mapfile_parser.frontends.upload_frogress.uploadEntriesToFrogress(codeEntries, category, url, apikey=frogress_api_key, verbose=True)

def main(args: argparse.Namespace) -> None:
    """
    Main function, calculates the progress and uploads it to frogress.
    """
    frogress_api_key = args.frogress_api_key
    if not frogress_api_key and not args.dry_run:
        raise ValueError("Missing frogress API key.")

    processMapFiles(MAP_FILES, frogress_api_key, dry_run=args.dry_run)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Upload progress to the frogress")
    parser.add_argument("--frogress_api_key", help="API key for the frogress")
    parser.add_argument("--dry-run", action="store_true", help="Print local progress without uploading or requiring an API key")

    args = parser.parse_args()
    main(args)
