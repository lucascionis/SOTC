#!/usr/bin/env python3
"""Compare original XFF2 text with a MIPS ELF before fix_xff runs.

Only the address bits of ORIGINAL relocations may differ. Relocation metadata
from the compiled ELF never enlarges those masks. Destination checks resolve
section/absolute symbols and unambiguous adjacent HI16/LO16 pairs; the JSON
report explicitly counts every site whose destination cannot be established.
No third-party packages or generated baseline files are required.
"""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import dataclass
import json
from pathlib import Path
import struct
from typing import Sequence


class InputError(ValueError):
    """Unsupported or malformed binary input."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise InputError(message)


def region(data: bytes, offset: int, size: int, label: str) -> bytes:
    require(0 <= offset <= len(data) and 0 <= size <= len(data) - offset,
            f"{label}: range {offset:#x}+{size:#x} exceeds file size {len(data):#x}")
    return data[offset:offset + size]


def unpack(data: bytes, offset: int, fmt: str, label: str) -> tuple:
    return struct.unpack(fmt, region(data, offset, struct.calcsize(fmt), label))


def word(data: bytes, offset: int) -> int:
    return unpack(data, offset, "<I", "word")[0]


def string(data: bytes, offset: int, label: str) -> str:
    require(0 <= offset < len(data), f"{label}: string offset out of bounds")
    end = data.find(b"\0", offset)
    require(end != -1, f"{label}: unterminated string")
    try:
        return data[offset:end].decode("utf-8")
    except UnicodeDecodeError as exc:
        raise InputError(f"{label}: invalid UTF-8 string") from exc


@dataclass(frozen=True)
class Section:
    name: str
    address: int
    size: int
    offset: int
    kind: int


@dataclass(frozen=True)
class Symbol:
    name: str
    value: int
    section: int


@dataclass(frozen=True)
class Relocation:
    offset: int
    kind: int
    symbol: int
    # The XFF records preserve the entire original instruction, not merely
    # its address field. Keeping this also detects incompatible duplicates.
    saved_word: int | None = None
    extra: int = 0


MASKS = {0: 0xFFFFFFFF, 2: 0, 4: 0xFC000000,
         5: 0xFFFF0000, 6: 0xFFFF0000}


def add_relocation(result: dict[int, Relocation], relocation: Relocation,
                   label: str) -> None:
    previous = result.get(relocation.offset)
    require(previous is None or previous == relocation,
            f"{label}: incompatible duplicate relocation at {relocation.offset:#x}")
    result[relocation.offset] = relocation


def validate_site(relocation: Relocation, section: Section, label: str) -> None:
    require(relocation.kind in MASKS,
            f"{label}: unsupported MIPS relocation type {relocation.kind}")
    require(relocation.offset % 4 == 0 and relocation.offset + 4 <= section.size,
            f"{label}: relocation site outside section or not word aligned")


def parse_xff(data: bytes) -> tuple[Section, dict[int, Relocation], list[Symbol], list[Section]]:
    region(data, 0, 112, "XFF header")
    require(data[:4] == b"xff2", "expected an XFF2 original")
    require(word(data, 20) == len(data), "XFF: declared file size differs from input size")
    count, table = word(data, 64), word(data, 92)
    require(count > 0, "XFF: no sections")
    region(data, table, count * 32, "XFF section table")
    names_table, names_base = word(data, 104), word(data, 108)
    region(data, names_table, count * 4, "XFF section name offsets")
    region(data, names_base, 1, "XFF section names")
    raw = [unpack(data, table + i * 32, "<8I", "XFF section") for i in range(count)]
    name_offsets = [word(data, names_table + i * 4) for i in range(count)]
    names = [string(data[names_base:], offset, "XFF section name") for offset in name_offsets]
    # XFF metadata sections use separate header offsets. Their offs_Rel can
    # legitimately be zero even when their size is nonzero.
    for i, name in enumerate(names):
        if name == ".shstrtab":
            name_bytes = region(data, names_base, raw[i][2], "XFF section names")
            names = [string(name_bytes, off, "XFF section name") for off in name_offsets]
            break
    sections = [Section(names[i], s[1], s[2], s[7], s[4]) for i, s in enumerate(raw)]
    for section, s in zip(sections, raw):
        require(not s[3] or s[3] & (s[3] - 1) == 0, "XFF: invalid section alignment")
        require(section.address + section.size <= 1 << 32, "XFF: section address overflow")
        if section.kind == 1:
            region(data, section.offset, section.size, f"XFF {section.name}")
    text_indices = [i for i, section in enumerate(sections) if section.name == ".text"]
    require(len(text_indices) == 1, "XFF: expected exactly one .text section")
    text_index = text_indices[0]
    text = sections[text_index]
    require(text.kind == 1 and text.size > 0 and text.size % 4 == 0,
            "XFF: .text must contain complete instruction words")
    require(text.address % 4 == 0 and text.offset % 4 == 0, "XFF: unaligned .text")

    symbol_count, symbol_table, strings_base = word(data, 36), word(data, 84), word(data, 88)
    region(data, symbol_table, symbol_count * 16, "XFF symbol table")
    region(data, strings_base, 1, "XFF symbol strings")
    strings_size = next((section.size for section in sections if section.name == ".strtab"),
                        len(data) - strings_base)
    strings = region(data, strings_base, strings_size, "XFF symbol strings")
    symbols = []
    for i in range(symbol_count):
        name, value, _, _, _, index = unpack(data, symbol_table + i * 16, "<3IBBH", "XFF symbol")
        symbols.append(Symbol(string(strings, name, "XFF symbol name"), value, index))

    heads_count, heads = word(data, 56), word(data, 100)
    region(data, heads, heads_count * 28, "XFF relocation headers")
    result: dict[int, Relocation] = {}
    # Validate all relocation tables, including non-text section ranges.
    all_sites: dict[tuple[int, int], Relocation] = {}
    for i in range(heads_count):
        _, nr, index, _, _, addresses, saved = unpack(data, heads + i * 28, "<7I", "XFF relocation header")
        require(index < count, "XFF: relocation section index out of bounds")
        region(data, addresses, nr * 8, "XFF relocation addresses")
        region(data, saved, nr * 8, "XFF saved relocation words")
        for j in range(nr):
            offset, info = unpack(data, addresses + j * 8, "<2I", "XFF relocation")
            saved_word, extra = unpack(data, saved + j * 8, "<2I", "XFF saved relocation")
            rel = Relocation(offset, info & 255, info >> 8, saved_word, extra)
            validate_site(rel, sections[index], "XFF")
            require(rel.symbol < len(symbols), "XFF: relocation symbol index out of bounds")
            sym_section = symbols[rel.symbol].section
            require(sym_section < count or sym_section in (0xFFF1, 0xFFF2),
                    "XFF: referenced symbol section index out of bounds")
            key = (index, offset)
            require(key not in all_sites or all_sites[key] == rel,
                    f"XFF: incompatible duplicate relocation at section {index}+{offset:#x}")
            all_sites[key] = rel
            if index == text_index:
                require(word(data, text.offset + offset) == saved_word,
                        "XFF: saved relocation word disagrees with original .text")
                if rel.kind == 5:
                    require(saved_word >> 26 == 15, "XFF: HI16 relocation is not a LUI")
                if rel.kind == 4:
                    require(saved_word >> 26 in (2, 3), "XFF: MIPS26 relocation is not J/JAL")
                add_relocation(result, rel, "XFF")
    return text, result, symbols, sections


def parse_elf(data: bytes) -> tuple[Section, dict[int, Relocation], dict[int, int | None], int, dict[int, str | None]]:
    header = unpack(data, 0, "<16sHHIIIIIHHHHHH", "ELF header")
    ident, elf_type, machine, version, _, phoff, shoff, _, ehsize, phstride, phcount, stride, count, names_index = header
    require(ident[:7] == b"\x7fELF\x01\x01\x01", "expected a little-endian ELF32")
    require(machine == 8 and version == 1 and elf_type in (1, 2),
            "expected a MIPS relocatable or executable ELF")
    require(ehsize == 52 and stride == 40 and count > 0 and names_index < count,
            "ELF: invalid or unsupported section header layout")
    if phcount:
        require(phstride == 32, "ELF: invalid program header stride")
        region(data, phoff, phcount * phstride, "ELF program headers")
        for i in range(phcount):
            _, offset, address, _, file_size, memory_size, _, alignment = unpack(
                data, phoff + i * phstride, "<8I", "ELF program header")
            region(data, offset, file_size, "ELF program contents")
            require(memory_size >= file_size and address + memory_size <= 1 << 32,
                    "ELF: invalid program memory range")
            require(not alignment or alignment & (alignment - 1) == 0,
                    "ELF: invalid program alignment")
    region(data, shoff, count * stride, "ELF section table")
    raw = [unpack(data, shoff + i * stride, "<10I", "ELF section") for i in range(count)]
    require(raw[0] == (0,) * 10, "ELF: invalid null section")
    require(raw[names_index][1] == 3, "ELF: section name table is not STRTAB")
    names = region(data, raw[names_index][4], raw[names_index][5], "ELF section names")
    sections = []
    for s in raw:
        name = string(names, s[0], "ELF section name")
        require(not s[8] or s[8] & (s[8] - 1) == 0, "ELF: invalid section alignment")
        require(s[3] + s[5] <= 1 << 32, "ELF: section address overflow")
        if s[1] != 8:
            region(data, s[4], s[5], f"ELF {name}")
        sections.append(Section(name, s[3], s[5], s[4], s[1]))
    text_indices = [i for i, section in enumerate(sections) if section.name == ".text"]
    require(len(text_indices) == 1, "ELF: expected exactly one .text section")
    text_index = text_indices[0]
    text = sections[text_index]
    require(text.kind == 1 and raw[text_index][2] & 4 and text.size % 4 == 0,
            "ELF: .text must be executable PROGBITS with complete words")
    require(text.address % 4 == 0 and text.offset % 4 == 0, "ELF: unaligned .text")

    symbol_tables: dict[int, list[Symbol]] = {}
    for i, s in enumerate(raw):
        if s[1] not in (2, 11):
            continue
        require(s[9] == 16 and s[5] % 16 == 0 and s[6] < count,
                "ELF: invalid symbol table layout")
        strings_header = raw[s[6]]
        require(strings_header[1] == 3, "ELF: symbol table link is not STRTAB")
        strings = region(data, strings_header[4], strings_header[5], "ELF symbol strings")
        entries = []
        for j in range(s[5] // 16):
            name, value, _, _, _, index = unpack(data, s[4] + j * 16, "<3IBBH", "ELF symbol")
            require(index < count or index in (0xFFF1, 0xFFF2),
                    "ELF: invalid or unsupported symbol section index")
            entries.append(Symbol(string(strings, name, "ELF symbol name"), value, index))
        symbol_tables[i] = entries

    result: dict[int, Relocation] = {}
    bases: dict[int, int | None] = {}
    destination_sections: dict[int, str | None] = {}
    for s in raw:
        if s[1] not in (4, 9):
            continue
        require(s[6] in symbol_tables and s[7] < count,
                "ELF: invalid relocation section links")
        entry_size = 12 if s[1] == 4 else 8
        require(s[9] == entry_size and s[5] % entry_size == 0,
                "ELF: invalid relocation entry size")
        require(s[1] == 9 or s[7] != text_index, "ELF: text RELA relocations are unsupported")
        for j in range(s[5] // entry_size):
            offset, info = unpack(data, s[4] + j * entry_size, "<2I", "ELF relocation")
            index = info >> 8
            entries = symbol_tables[s[6]]
            require(index < len(entries), "ELF: relocation symbol index out of bounds")
            if elf_type == 2:
                offset -= sections[s[7]].address
            require(offset >= 0 and offset + 4 <= sections[s[7]].size and offset % 4 == 0,
                    "ELF: relocation site outside section or not word aligned")
            if s[7] != text_index:
                continue
            rel = Relocation(offset, info & 255, index)
            validate_site(rel, text, "ELF")
            sym = entries[index]
            base = None
            if sym.section == 0xFFF1:
                base = sym.value
            elif sym.section not in (0, 0xFFF2):
                base = sym.value + (sections[sym.section].address if elf_type == 1 else 0)
            require(base is None or base < 1 << 32, "ELF: symbol address overflow")
            # A symbol index is scoped to its own symbol table. Duplicates
            # across distinct tables must still have identical destinations.
            if offset in result:
                require(result[offset].kind == rel.kind and bases[offset] == base,
                        f"ELF: incompatible duplicate relocation at {offset:#x}")
                require(result[offset].symbol == index and result[offset] == rel,
                        f"ELF: ambiguous duplicate relocation at {offset:#x}")
            add_relocation(result, rel, "ELF")
            bases[offset] = base
            destination_sections[offset] = sections[sym.section].name if 0 < sym.section < count else None
    return text, result, bases, elf_type, destination_sections


def sign16(value: int) -> int:
    return (value & 0x7FFF) - (value & 0x8000)


def verify(original: bytes, compiled: bytes) -> dict:
    """Return a JSON-compatible report, or raise InputError for invalid files."""
    source_text, source_rels, symbols, sections = parse_xff(original)
    target_text, target_rels, target_bases, elf_type, target_sections = parse_elf(compiled)
    require(source_text.address == target_text.address, "compiled .text base address changed")
    require(target_text.size >= source_text.size, "compiled .text is shorter than original")

    differences = []
    source_words = [word(original, source_text.offset + off) for off in range(0, source_text.size, 4)]
    target_words = [word(compiled, target_text.offset + off) for off in range(0, source_text.size, 4)]
    for i, (a, b) in enumerate(zip(source_words, target_words)):
        off = i * 4
        mask = MASKS[source_rels[off].kind] if off in source_rels else 0xFFFFFFFF
        if a & mask != b & mask:
            differences.append(dict(address=f"0x{source_text.address + off:08x}",
                                    original=f"{a:08x}", compiled=f"{b:08x}",
                                    retained_mask=f"{mask:08x}"))

    def source_base(rel: Relocation) -> int | None:
        sym = symbols[rel.symbol]
        if sym.section == 0xFFF1:
            return sym.value
        if sym.section and sym.section < len(sections) and sections[sym.section].address:
            return sections[sym.section].address + sym.value
        return None  # Undefined/common symbols and runtime-allocated XFF BSS.

    failures = []
    unmapped_destinations = []
    statuses: dict[int, str] = {}

    def check(offsets: tuple[int, ...], a: int, b: int) -> None:
        # A partial link can append orphan C data/string literals to .text.
        # That suffix has no counterpart in the original XFF text extent.
        # Do not equate its temporary ELF VMA with a proven XFF destination.
        # Ordinary in-range destination
        # mismatches still fail, including wrong jumps into the suffix.
        original_text_end = source_text.address + source_text.size
        if (elf_type == 1 and (a & 0xFFFFFFFF) != (b & 0xFFFFFFFF)
                and not source_text.address <= a < original_text_end
                and original_text_end <= b < target_text.address + target_text.size
                and all(target_sections.get(off) == ".text" for off in offsets)):
            for off in offsets:
                statuses[off] = "compiled destination lies in extra .text without XFF runtime mapping"
            unmapped_destinations.append(dict(address=f"0x{source_text.address + offsets[0]:08x}",
                                             original_destination=f"0x{a & 0xFFFFFFFF:08x}",
                                             temporary_elf_destination=f"0x{b & 0xFFFFFFFF:08x}"))
            return
        for off in offsets:
            statuses[off] = "checked"
        if (a & 0xFFFFFFFF) != (b & 0xFFFFFFFF):
            failures.append(dict(address=f"0x{source_text.address + offsets[0]:08x}",
                                 original_destination=f"0x{a & 0xFFFFFFFF:08x}",
                                 compiled_destination=f"0x{b & 0xFFFFFFFF:08x}"))

    for off, rel in sorted(source_rels.items()):
        if rel.kind == 0:
            statuses[off] = "no address relocation"
            continue
        target_rel = target_rels.get(off)
        if target_rel and target_rel.kind != rel.kind:
            failures.append(dict(address=f"0x{source_text.address + off:08x}",
                                 reason="relocation type changed", original_type=rel.kind,
                                 compiled_type=target_rel.kind))
            statuses[off] = "type mismatch"
            continue
        if not target_rel and elf_type == 1:
            statuses[off] = "compiled ELF has no relocation at this site"
            continue
        a_base = source_base(rel)
        # ET_EXEC instruction words are already resolved, even when a linker
        # retained --emit-relocs metadata. ET_REL words still contain addends.
        b_base = target_bases.get(off) if elf_type == 1 else 0
        if a_base is None or b_base is None:
            statuses[off] = "undefined/common symbol or XFF section has no runtime address"
            continue
        a, b = source_words[off // 4], target_words[off // 4]
        if rel.kind == 2:
            check((off,), a_base + a, b_base + b)
        elif rel.kind == 4:
            a_dest = a_base + ((a & 0x3FFFFFF) << 2)
            b_dest = b_base + ((b & 0x3FFFFFF) << 2)
            if elf_type == 2:  # Resolved executable jump includes the PC region.
                b_dest |= (target_text.address + off + 4) & 0xF0000000
            check((off,), a_dest, b_dest)
        elif rel.kind == 5:
            lo = source_rels.get(off + 4)
            target_lo = target_rels.get(off + 4)
            a_lo_base = source_base(lo) if lo else None
            b_lo_base = target_bases.get(off + 4) if elf_type == 1 else 0
            # Restrict proof to adjacent LUI + signed-low instructions using
            # the same base register and equal symbol addresses. More distant
            # pairs can involve branches/register reuse and require CFG analysis.
            signed_low_opcodes = {8, 9, 24, 25, 26, 27, 30, 31, *range(32, 64)}
            a_lo = source_words[off // 4 + 1] if off + 4 < source_text.size else 0
            paired = (lo is not None and lo.kind == 6 and lo.symbol == rel.symbol
                      and a_lo >> 26 in signed_low_opcodes
                      and (a >> 16 & 31) == (a_lo >> 21 & 31)
                      and a_lo_base == a_base and b_lo_base == b_base
                      and (target_lo is None and elf_type == 2 or target_lo is not None and target_lo.kind == 6))
            if paired:
                b_lo = target_words[off // 4 + 1]
                check((off, off + 4), a_base + ((a & 0xFFFF) << 16) + sign16(a_lo),
                      b_base + ((b & 0xFFFF) << 16) + sign16(b_lo))
            else:
                statuses[off] = "HI16/LO16 pair requires control-flow analysis"
        elif off not in statuses:
            statuses[off] = "HI16/LO16 pair requires control-flow analysis"

    counts = Counter(statuses.values())
    checked = counts.pop("checked", 0)
    counts.pop("no address relocation", None)
    unverified = sum(counts.values())
    limits = ["Checks only the original XFF .text extent; other sections require separate checks.",
              "Destination checks do not prove arbitrary HI16/LO16 control-flow pairs or unresolved/runtime symbols.",
              "A match with unverified destinations is not a proof of all relocation targets; inspect the coverage counts."]
    if target_text.size > source_text.size:
        limits.append("Compiled .text contains additional bytes after the original extent; those bytes are not compared.")
        limits.append("Destinations in that extra .text suffix for non-text original targets have no XFF runtime mapping and are explicitly unverified.")
    return dict(schema_version=1, original_text_address=f"0x{source_text.address:08x}",
                original_text_bytes=source_text.size, compiled_text_bytes=target_text.size,
                words_checked=source_text.size // 4, relocation_sites_checked=len(source_rels),
                opcode_and_register_differences=len(differences), first_differences=differences[:20],
                relocation_destinations_checked=checked, relocation_destinations_unverified=unverified,
                unverified_destination_reasons=dict(sorted(counts.items())),
                first_unmapped_destinations=unmapped_destinations[:20],
                relocation_destination_differences=len(failures), first_destination_differences=failures[:20],
                instruction_match=not differences, verified_destination_match=not failures,
                all_relocation_destinations_verified=unverified == 0, limits=limits,
                match=not differences and not failures)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--original", type=Path, required=True, help="untouched original XFF2")
    parser.add_argument("--compiled", type=Path, required=True, help="MIPS ELF before fix_xff")
    parser.add_argument("--report", type=Path, help="also save JSON report at this path")
    args = parser.parse_args(argv)
    report_allowed = True
    try:
        if args.report:
            for input_path in (args.original, args.compiled):
                aliases_input = args.report.resolve() == input_path.resolve()
                if args.report.exists() and input_path.exists():
                    aliases_input |= args.report.samefile(input_path)
                if aliases_input:
                    report_allowed = False
                    raise InputError("report path must differ from both binary input files")
        report = verify(args.original.read_bytes(), args.compiled.read_bytes())
        report.update(original=str(args.original), compiled=str(args.compiled))
        exit_code = 0 if report["match"] else 1
    except (OSError, InputError) as exc:
        report = dict(schema_version=1, match=False, error=str(exc),
                      original=str(args.original), compiled=str(args.compiled))
        exit_code = 2
    rendered = json.dumps(report, indent=2) + "\n"
    if args.report and report_allowed:
        try:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(rendered, encoding="utf-8")
        except OSError as exc:
            report.update(match=False, error=f"cannot write report: {exc}")
            rendered = json.dumps(report, indent=2) + "\n"
            exit_code = 2
    print(rendered, end="")
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
