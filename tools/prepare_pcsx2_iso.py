#!/usr/bin/env python3
"""Prepare a verified, locally supplied SOTC ISO for PCSX2, without changing its layout."""

import argparse
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import shutil
import sys
import tempfile

SECTOR_SIZE = 2048
CHUNK_SIZE = 1024 * 1024
EXPECTED_SHA1 = {
    "SCPS_150.97;1": "c4d5576d1cae3721c411a746c7845f5c6f026dbb",
    "KERNEL.XFF;1": "ac00c937b84cbe2e283a97e59e09a11886118388",
}


class IsoError(ValueError):
    """An image or replacement does not satisfy the verified-layout contract."""


@dataclass(frozen=True)
class Record:
    path: str
    name: str
    sector: int
    size: int
    flags: int

    @property
    def offset(self):
        return self.sector * SECTOR_SIZE

    @property
    def directory(self):
        return bool(self.flags & 2)


def _both(data, offset, width, label):
    little = int.from_bytes(data[offset:offset + width], "little")
    big = int.from_bytes(data[offset + width:offset + 2 * width], "big")
    if little != big:
        raise IsoError(f"Inconsistent ISO9660 {label} byte order")
    return little


def _read_at(stream, offset, size):
    stream.seek(offset)
    data = stream.read(size)
    if len(data) != size:
        raise IsoError(f"Truncated ISO at byte {offset} (wanted {size} bytes)")
    return data


def _record(data, parent, volume_bytes):
    if len(data) < 34 or data[0] != len(data):
        raise IsoError("Malformed ISO9660 directory record")
    name_size = data[32]
    if not name_size or 33 + name_size > len(data):
        raise IsoError("Malformed ISO9660 file identifier")
    identifier = data[33:33 + name_size]
    if identifier in (b"\0", b"\1"):
        name = "." if identifier == b"\0" else ".."
    else:
        try:
            name = identifier.decode("ascii")
        except UnicodeDecodeError as exc:
            raise IsoError("Non-ASCII primary ISO9660 identifier") from exc
        if "/" in name or "\0" in name:
            raise IsoError("Invalid primary ISO9660 identifier")
    sector = _both(data, 2, 4, "extent")
    size = _both(data, 10, 4, "file length")
    if _both(data, 28, 2, "volume sequence") != 1:
        raise IsoError("Multi-volume ISO9660 images are unsupported")
    if data[1] or data[26] or data[27]:
        raise IsoError("Extended-attribute or interleaved extents are unsupported")
    if data[25] & 128:
        raise IsoError("Multi-extent ISO9660 files are unsupported")
    if sector * SECTOR_SIZE + size > volume_bytes:
        raise IsoError(f"Extent outside ISO volume: {name}")
    return Record(parent + "/" + name, name, sector, size, data[25])


def read_iso9660(stream):
    """Return primary-volume records and descriptor end offset, reading one sector at a time."""
    stream.seek(0, os.SEEK_END)
    image_size = stream.tell()
    if image_size % SECTOR_SIZE:
        raise IsoError("ISO image length must be a multiple of 2048 bytes")
    primary = None
    descriptor_end = None
    for sector in range(16, 80):
        data = _read_at(stream, sector * SECTOR_SIZE, SECTOR_SIZE)
        if data[1:6] != b"CD001" or data[6] != 1:
            raise IsoError("Invalid ISO9660 volume descriptor (expected a 2048-byte-sector ISO)")
        if data[0] == 1:
            if primary is not None:
                raise IsoError("Ambiguous primary volume descriptors")
            primary = data
        if data[0] == 255:
            descriptor_end = (sector + 1) * SECTOR_SIZE
            break
    if primary is None or descriptor_end is None:
        raise IsoError("Missing primary volume descriptor or descriptor terminator")
    if _both(primary, 128, 2, "logical block size") != SECTOR_SIZE:
        raise IsoError("Only 2048-byte ISO9660 logical blocks are supported")
    if _both(primary, 120, 2, "volume set size") != 1:
        raise IsoError("Multi-volume ISO9660 images are unsupported")
    if _both(primary, 124, 2, "volume sequence") != 1:
        raise IsoError("Unsupported ISO9660 volume sequence")
    volume_bytes = _both(primary, 80, 4, "volume space size") * SECTOR_SIZE
    if not descriptor_end <= volume_bytes <= image_size:
        raise IsoError("ISO9660 volume length is outside the image")
    root_length = primary[156]
    root = _record(primary[156:156 + root_length], "", volume_bytes)
    if not root.directory or root.name != "." or not root.size:
        raise IsoError("Invalid ISO9660 root directory")
    records = []
    directories = [root]
    seen_directories = set()
    while directories:
        directory = directories.pop()
        extent = (directory.sector, directory.size)
        if extent in seen_directories:
            raise IsoError("Repeated or cyclic ISO9660 directory extent")
        seen_directories.add(extent)
        if len(seen_directories) > 10000:
            raise IsoError("Too many ISO9660 directories")
        if directory.offset < descriptor_end:
            raise IsoError("Directory overlaps ISO9660 volume descriptors")
        records.append(directory)
        parent = "" if directory is root else directory.path
        for relative in range(0, directory.size, SECTOR_SIZE):
            block = _read_at(stream, directory.offset + relative,
                             min(SECTOR_SIZE, directory.size - relative))
            position = 0
            while position < len(block):
                length = block[position]
                if not length:
                    if any(block[position:]):
                        raise IsoError("Nonzero bytes after directory sector padding")
                    break
                if position + length > len(block):
                    raise IsoError("Directory record crosses a sector boundary")
                entry = _record(block[position:position + length], parent, volume_bytes)
                position += length
                if entry.name in (".", ".."):
                    continue
                if entry.directory:
                    if not entry.size:
                        raise IsoError("Empty ISO9660 directory extent")
                    directories.append(entry)
                else:
                    records.append(entry)
                if len(records) + len(directories) > 100000:
                    raise IsoError("Too many ISO9660 records")
    return records, descriptor_end


def locate_files(records, descriptor_end, names):
    selected = {}
    for name in names:
        matches = [record for record in records if record.name == name]
        if len(matches) != 1 or matches[0].directory:
            raise IsoError(f"Expected exactly one file {name}; found {len(matches)}")
        record = matches[0]
        if not record.size or record.offset < descriptor_end:
            raise IsoError(f"Invalid target extent: {record.path}")
        for other in records:
            if other is record or not other.size:
                continue
            if max(record.offset, other.offset) < min(record.offset + record.size,
                                                       other.offset + other.size):
                raise IsoError(f"Target extent overlaps {other.path}: {record.path}")
        selected[name] = record
    return selected


def _hash_region(stream, offset=0, size=None):
    stream.seek(offset)
    digest = hashlib.sha1()
    remaining = size
    while remaining is None or remaining:
        data = stream.read(CHUNK_SIZE if remaining is None else min(CHUNK_SIZE, remaining))
        if not data:
            if remaining:
                raise IsoError("Truncated file while hashing")
            break
        digest.update(data)
        if remaining is not None:
            remaining -= len(data)
    return digest.hexdigest()


def _identity(stream):
    stat = os.fstat(stream.fileno())
    return stat.st_dev, stat.st_ino, stat.st_size, stat.st_mtime_ns, stat.st_ctime_ns


def _publish(temp_path, destination):
    # Unlike replace(), link() never overwrites an existing destination, including a raced creation.
    os.link(temp_path, destination)
    os.unlink(temp_path)


def _write_report(report_path, report):
    with tempfile.TemporaryDirectory(prefix=".pcsx2-report-", dir=report_path.parent) as directory:
        temporary = Path(directory) / "report.json"
        with temporary.open("w", encoding="utf-8") as stream:
            json.dump(report, stream, indent=2, sort_keys=True)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
        _publish(temporary, report_path)


def prepare_iso(iso, loader, kernel, output, report_path=None, *, expected_sha1=None):
    """Verify and prepare an ISO. Hash injection is for synthetic API tests, never exposed by CLI."""
    expected = EXPECTED_SHA1 if expected_sha1 is None else expected_sha1
    if set(expected) != set(EXPECTED_SHA1):
        raise IsoError("Expected hashes must describe precisely the loader and kernel")
    inputs = [Path(path).resolve() for path in (iso, loader, kernel)]
    destination = Path(output).absolute()
    report_destination = Path(report_path).absolute() if report_path is not None else None
    for path in [destination] + ([report_destination] if report_destination else []):
        if path.resolve() in inputs:
            raise IsoError("Output/report must differ from every input path")
        if os.path.lexists(path):
            raise IsoError(f"Refusing to overwrite existing path: {path}")
        if not path.parent.is_dir():
            raise IsoError(f"Destination directory does not exist: {path.parent}")
    if report_destination and report_destination.resolve() == destination.resolve():
        raise IsoError("Report and ISO output paths must differ")
    with inputs[0].open("rb") as source, inputs[1].open("rb") as loader_stream, inputs[2].open("rb") as kernel_stream:
        source_identity = _identity(source)
        records, descriptor_end = read_iso9660(source)
        targets = locate_files(records, descriptor_end, EXPECTED_SHA1)
        replacements = dict(zip(EXPECTED_SHA1, (loader_stream, kernel_stream)))
        file_report = {}
        replacement_identities = {}
        for name, replacement in replacements.items():
            target = targets[name]
            identity = _identity(replacement)
            replacement_identities[name] = identity
            if identity[2] != target.size:
                raise IsoError(f"Replacement length differs for {name}: {identity[2]} != {target.size}")
            original_hash = _hash_region(source, target.offset, target.size)
            replacement_hash = _hash_region(replacement)
            if original_hash != expected[name]:
                raise IsoError(f"Original ISO {name} SHA-1 differs: {original_hash}")
            if replacement_hash != expected[name]:
                raise IsoError(f"Recompiled {name} SHA-1 differs: {replacement_hash}")
            file_report[name] = {"path": target.path, "offset": target.offset,
                                 "size": target.size, "expected_sha1": expected[name],
                                 "original_sha1": original_hash, "replacement_sha1": replacement_hash}
        system_files = [record for record in records if record.name == "SYSTEM.CNF;1" and not record.directory]
        boot2 = None
        if len(system_files) == 1:
            record = system_files[0]
            preview = _read_at(source, record.offset, min(record.size, 16384)).decode("ascii", "replace")
            boot2 = next((line.strip() for line in preview.splitlines()
                          if line.split("=", 1)[0].strip().upper() == "BOOT2"), None)
        with tempfile.TemporaryDirectory(prefix=".pcsx2-iso-", dir=destination.parent) as directory:
            temporary = Path(directory) / "prepared.iso"
            with temporary.open("w+b") as patched:
                source.seek(0)
                shutil.copyfileobj(source, patched, CHUNK_SIZE)
                source_hash = _hash_region(source)
                if patched.seek(0, os.SEEK_END) != source_identity[2] or _hash_region(patched) != source_hash:
                    raise IsoError("Copied ISO verification failed")
                if _identity(source) != source_identity:
                    raise IsoError("Source ISO changed during preparation")
                for name, replacement in replacements.items():
                    target = targets[name]
                    patched.seek(target.offset)
                    replacement.seek(0)
                    remaining = target.size
                    while remaining:
                        data = replacement.read(min(CHUNK_SIZE, remaining))
                        if not data:
                            raise IsoError(f"Replacement changed or was truncated: {name}")
                        patched.write(data)
                        remaining -= len(data)
                    patched.flush()
                    digest = _hash_region(patched, target.offset, target.size)
                    if digest != expected[name]:
                        raise IsoError(f"Patched extent verification failed: {name}")
                    if _identity(replacement) != replacement_identities[name]:
                        raise IsoError(f"Replacement changed during preparation: {name}")
                    file_report[name]["patched_sha1"] = digest
                output_hash = _hash_region(patched)
                if _identity(source) != source_identity:
                    raise IsoError("Source ISO changed during verification")
                patched.flush()
                os.fsync(patched.fileno())
            report = {"iso": str(inputs[0]), "output": str(destination),
                      "image_size": source_identity[2], "source_sha1": source_hash,
                      "output_sha1": output_hash, "boot2": boot2, "files": file_report}
            published_stat = temporary.stat()
            _publish(temporary, destination)
            if report_destination:
                try:
                    _write_report(report_destination, report)
                except Exception:
                    # Roll back only our published inode, preserving a concurrent replacement.
                    try:
                        current = destination.lstat()
                        if (current.st_dev, current.st_ino) == (published_stat.st_dev, published_stat.st_ino):
                            destination.unlink()
                    except FileNotFoundError:
                        pass
                    raise
    return report


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--iso", required=True, type=Path, help="Locally supplied original ISO9660 image")
    parser.add_argument("--loader", required=True, type=Path, help="Matching rebuilt SCPS_150.97")
    parser.add_argument("--kernel", required=True, type=Path, help="Matching rebuilt KERNEL.XFF")
    parser.add_argument("--output", required=True, type=Path, help="New ISO path; existing paths are refused")
    parser.add_argument("--report", type=Path, help="Optional new JSON report path")
    args = parser.parse_args(argv)
    try:
        report = prepare_iso(args.iso, args.loader, args.kernel, args.output, args.report)
    except (IsoError, OSError) as exc:
        parser.exit(1, f"prepare_pcsx2_iso: {exc}\n")
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    sys.exit(main())
