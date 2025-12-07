#!/usr/bin/env python3
"""Generate fingerprint visualizations from release CSV artifacts.

This script reads SSH/TLS CSV summaries from ``release/artifacts/csv`` and
exports simple PNG and SVG visuals to ``release/artifacts/figures``.
"""
from __future__ import annotations

import argparse
import csv
import json
import math
from collections import Counter
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Dict, Iterable, List, Tuple

CSV_DIR = Path("release/artifacts/csv")
FIGURE_DIR = Path("release/artifacts/figures")
METADATA_PATH = Path("release/artifacts/eval_metadata.json")

# Basic 5x7 pixel font for ASCII text rendering
FONT_5X7: Dict[str, List[str]] = {
    "A": ["01110", "10001", "10001", "11111", "10001", "10001", "10001"],
    "B": ["11110", "10001", "11110", "10001", "10001", "10001", "11110"],
    "C": ["01110", "10001", "10000", "10000", "10000", "10001", "01110"],
    "D": ["11110", "10001", "10001", "10001", "10001", "10001", "11110"],
    "E": ["11111", "10000", "11110", "10000", "10000", "10000", "11111"],
    "F": ["11111", "10000", "11110", "10000", "10000", "10000", "10000"],
    "G": ["01110", "10001", "10000", "10000", "10011", "10001", "01111"],
    "H": ["10001", "10001", "11111", "10001", "10001", "10001", "10001"],
    "I": ["01110", "00100", "00100", "00100", "00100", "00100", "01110"],
    "J": ["00001", "00001", "00001", "00001", "10001", "10001", "01110"],
    "K": ["10001", "10010", "11100", "10010", "10001", "10001", "10001"],
    "L": ["10000", "10000", "10000", "10000", "10000", "10000", "11111"],
    "M": ["10001", "11011", "10101", "10101", "10001", "10001", "10001"],
    "N": ["10001", "11001", "10101", "10011", "10001", "10001", "10001"],
    "O": ["01110", "10001", "10001", "10001", "10001", "10001", "01110"],
    "P": ["11110", "10001", "10001", "11110", "10000", "10000", "10000"],
    "Q": ["01110", "10001", "10001", "10001", "10101", "10010", "01101"],
    "R": ["11110", "10001", "10001", "11110", "10010", "10001", "10001"],
    "S": ["01111", "10000", "10000", "01110", "00001", "00001", "11110"],
    "T": ["11111", "00100", "00100", "00100", "00100", "00100", "00100"],
    "U": ["10001", "10001", "10001", "10001", "10001", "10001", "01110"],
    "V": ["10001", "10001", "10001", "10001", "01010", "01010", "00100"],
    "W": ["10001", "10001", "10001", "10101", "10101", "11011", "10001"],
    "X": ["10001", "01010", "00100", "00100", "01010", "01010", "10001"],
    "Y": ["10001", "01010", "00100", "00100", "00100", "00100", "00100"],
    "Z": ["11111", "00001", "00010", "00100", "01000", "10000", "11111"],
    "0": ["01110", "10001", "10011", "10101", "11001", "10001", "01110"],
    "1": ["00100", "01100", "00100", "00100", "00100", "00100", "01110"],
    "2": ["01110", "10001", "00001", "00010", "00100", "01000", "11111"],
    "3": ["11110", "00001", "00001", "01110", "00001", "00001", "11110"],
    "4": ["10001", "10001", "10001", "11111", "00001", "00001", "00001"],
    "5": ["11111", "10000", "11110", "00001", "00001", "00001", "11110"],
    "6": ["01111", "10000", "11110", "10001", "10001", "10001", "01110"],
    "7": ["11111", "00001", "00010", "00100", "01000", "01000", "01000"],
    "8": ["01110", "10001", "10001", "01110", "10001", "10001", "01110"],
    "9": ["01110", "10001", "10001", "01111", "00001", "00001", "11110"],
    " ": ["00000", "00000", "00000", "00000", "00000", "00000", "00000"],
    "-": ["00000", "00000", "00000", "11111", "00000", "00000", "00000"],
    "/": ["00001", "00010", "00100", "01000", "10000", "00000", "00000"],
    ":": ["00000", "00100", "00100", "00000", "00100", "00100", "00000"],
    ".": ["00000", "00000", "00000", "00000", "00000", "00110", "00110"],
    ",": ["00000", "00000", "00000", "00000", "00000", "00110", "01100"],
    "(": ["00010", "00100", "01000", "01000", "01000", "00100", "00010"],
    ")": ["01000", "00100", "00010", "00010", "00010", "00100", "01000"],
    "_": ["00000", "00000", "00000", "00000", "00000", "00000", "11111"],
}


@dataclass
class Canvas:
    width: int
    height: int
    background: Tuple[int, int, int] = (255, 255, 255)

    def __post_init__(self) -> None:
        self.pixels: List[List[Tuple[int, int, int]]] = [
            [self.background for _ in range(self.width)] for _ in range(self.height)
        ]
        self.svg_elements: List[str] = []

    def set_pixel(self, x: int, y: int, color: Tuple[int, int, int]) -> None:
        if 0 <= x < self.width and 0 <= y < self.height:
            self.pixels[y][x] = color

    def draw_rect(self, x: int, y: int, w: int, h: int, color: Tuple[int, int, int]) -> None:
        for yy in range(max(0, y), min(self.height, y + h)):
            for xx in range(max(0, x), min(self.width, x + w)):
                self.set_pixel(xx, yy, color)
        self.svg_elements.append(
            f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{rgb_hex(color)}" />'
        )

    def draw_text(self, x: int, y: int, text: str, color: Tuple[int, int, int], scale: int = 2) -> None:
        cursor_x = x
        for ch in text.upper():
            pattern = FONT_5X7.get(ch, FONT_5X7.get("?", FONT_5X7[" "]))
            for row_idx, row in enumerate(pattern):
                for col_idx, bit in enumerate(row):
                    if bit == "1":
                        for dy in range(scale):
                            for dx in range(scale):
                                self.set_pixel(cursor_x + col_idx * scale + dx, y + row_idx * scale + dy, color)
            cursor_x += (len(pattern[0]) * scale) + scale
        font_size = 7 * scale
        self.svg_elements.append(
            f'<text x="{x}" y="{y + font_size}" font-family="Arial, sans-serif" font-size="{font_size}" '
            f'fill="{rgb_hex(color)}">{escape_svg(text)}</text>'
        )

    def save_png(self, path: Path) -> None:
        import struct
        import zlib

        raw = b"".join(
            b"\x00" + b"".join(struct.pack("BBB", *pix) for pix in row) for row in self.pixels
        )
        comp = zlib.compress(raw, 9)

        def chunk(tag: bytes, data: bytes) -> bytes:
            return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

        header = struct.pack(">IIBBBBB", self.width, self.height, 8, 2, 0, 0, 0)
        png_bytes = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) + chunk(b"IDAT", comp) + chunk(b"IEND", b"")
        path.write_bytes(png_bytes)

    def save_svg(self, path: Path) -> None:
        svg_body = "\n".join(self.svg_elements)
        path.write_text(
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{self.width}" height="{self.height}" '
            f'viewBox="0 0 {self.width} {self.height}">\n'
            f'<rect width="100%" height="100%" fill="{rgb_hex(self.background)}" />\n'
            f"{svg_body}\n" "</svg>\n",
            encoding="utf-8",
        )


def rgb_hex(color: Tuple[int, int, int]) -> str:
    return "#" + "".join(f"{c:02x}" for c in color)


def escape_svg(text: str) -> str:
    return (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
        .replace("'", "&apos;")
    )


def load_metadata_date(meta_path: Path = METADATA_PATH) -> str:
    if meta_path.exists():
        try:
            metadata = json.loads(meta_path.read_text())
            created = metadata.get("created")
            if created:
                return datetime.fromisoformat(created.replace("Z", "+00:00")).date().isoformat()
        except (ValueError, json.JSONDecodeError):
            pass
    return datetime.utcnow().date().isoformat()


def load_csv_records(path: Path) -> List[Dict[str, str]]:
    if not path.exists():
        return []
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        return list(reader)


def _filtered(records: Iterable[Dict[str, str]], record_type: str) -> List[Dict[str, str]]:
    target = record_type.upper()
    return [row for row in records if row.get("type", "").upper() == target]


def count_fingerprints(records: Iterable[Dict[str, str]], value_key: str = "fingerprint") -> Counter:
    counts: Counter = Counter()
    for row in records:
        value = row.get(value_key) or row.get("fingerprint")
        host = row.get("host", "unknown")
        if value:
            counts[(host, value)] += 1
    return counts


def plot_bar(counts: Counter, title: str, x_label: str, output_base: Path, date_label: str, bar_color=(44, 127, 184)) -> None:
    width, height = 1000, 700
    canvas = Canvas(width, height)
    margin_left, margin_right, margin_top, margin_bottom = 120, 50, 80, 130

    canvas.draw_text(30, 30, f"{title} (CAPTURED {date_label})", (20, 20, 20), scale=3)

    if not counts:
        canvas.draw_text(width // 2 - 120, height // 2, "NO RECORDS AVAILABLE", (100, 100, 100), scale=3)
        canvas.save_png(output_base.with_suffix(".png"))
        canvas.save_svg(output_base.with_suffix(".svg"))
        return

    labels = [f"{host}\n{fingerprint}" for host, fingerprint in counts.keys()]
    values = list(counts.values())
    max_value = max(values)

    plot_width = width - margin_left - margin_right
    plot_height = height - margin_top - margin_bottom
    bar_spacing = 10
    bar_width = max(10, int((plot_width - bar_spacing * (len(values) - 1)) / len(values)))

    # Axes
    x0, y0 = margin_left, height - margin_bottom
    canvas.draw_rect(x0, y0, plot_width, 2, (0, 0, 0))
    canvas.draw_rect(x0, y0 - plot_height, 2, plot_height, (0, 0, 0))

    step = max(1, math.ceil(max_value / 4))
    for tick_value in range(0, max_value + 1, step):
        y = y0 - int((tick_value / max_value) * plot_height)
        canvas.draw_rect(x0 - 6, y - 1, plot_width + 12, 2, (230, 230, 230))
        canvas.draw_text(40, y - 8, str(tick_value), (0, 0, 0), scale=2)

    # Bars and x-labels
    for idx, (label, value) in enumerate(zip(labels, values)):
        bar_x = x0 + idx * (bar_width + bar_spacing)
        bar_height = int((value / max_value) * (plot_height - 10))
        bar_y = y0 - bar_height
        canvas.draw_rect(bar_x, bar_y, bar_width, bar_height, bar_color)

        short_label = label.replace("\n", " ")
        if len(short_label) > 18:
            short_label = short_label[:17] + "…"
        canvas.draw_text(bar_x, y0 + 20, short_label, (0, 0, 0), scale=2)

    canvas.draw_text(width // 2 - 80, height - 40, x_label, (0, 0, 0), scale=3)
    canvas.draw_text(20, height // 2, "OBSERVATIONS", (0, 0, 0), scale=3)

    # Legend
    legend_x = width - margin_right - 200
    legend_y = margin_top - 20
    canvas.draw_rect(legend_x, legend_y, 20, 20, bar_color)
    canvas.draw_text(legend_x + 30, legend_y - 5, "SAMPLES", (0, 0, 0), scale=2)

    canvas.save_png(output_base.with_suffix(".png"))
    canvas.save_svg(output_base.with_suffix(".svg"))


def plot_table(summary_rows: List[Tuple[str, int, int, int]], title: str, output_base: Path, date_label: str) -> None:
    width, height = 900, 420
    canvas = Canvas(width, height)
    canvas.draw_text(30, 30, f"{title} (CAPTURED {date_label})", (20, 20, 20), scale=3)

    columns = ["DATASET", "HOSTS", "UNIQUE FINGERPRINTS", "SAMPLES"]
    row_height = 60
    col_widths = [360, 120, 240, 120]
    start_x, start_y = 40, 120

    # Header background
    header_color = (220, 235, 250)
    canvas.draw_rect(start_x, start_y, sum(col_widths), row_height, header_color)
    text_y = start_y + 18
    x_cursor = start_x + 10
    for idx, col in enumerate(columns):
        canvas.draw_text(x_cursor, text_y, col, (0, 0, 0), scale=2)
        x_cursor += col_widths[idx]

    # Rows
    if not summary_rows:
        summary_rows = [("NO RECORDS", 0, 0, 0)]
    for row_idx, row in enumerate(summary_rows, start=1):
        y = start_y + row_idx * row_height
        x_cursor = start_x + 10
        cells = [row[0], str(row[1]), str(row[2]), str(row[3])]
        if row_idx % 2 == 0:
            canvas.draw_rect(start_x, y, sum(col_widths), row_height, (245, 245, 245))
        for col_idx, cell in enumerate(cells):
            canvas.draw_text(x_cursor, y + 18, cell, (0, 0, 0), scale=2)
            x_cursor += col_widths[col_idx]

    # Grid lines
    table_height = row_height * (len(summary_rows) + 1)
    canvas.draw_rect(start_x, start_y, 2, table_height, (0, 0, 0))
    canvas.draw_rect(start_x + sum(col_widths), start_y, 2, table_height, (0, 0, 0))
    for i in range(len(col_widths)):
        x = start_x + sum(col_widths[:i])
        canvas.draw_rect(x, start_y, 2, table_height, (0, 0, 0))
    for i in range(len(summary_rows) + 2):
        y = start_y + i * row_height
        canvas.draw_rect(start_x, y, sum(col_widths), 2, (0, 0, 0))

    canvas.save_png(output_base.with_suffix(".png"))
    canvas.save_svg(output_base.with_suffix(".svg"))


def build_summary(records: List[Dict[str, str]], label: str) -> Tuple[str, int, int, int]:
    hosts = {row.get("host") for row in records if row.get("host")}
    fingerprints = {row.get("fingerprint") for row in records if row.get("fingerprint")}
    return (label, len(hosts), len(fingerprints), len(records))


def main() -> None:
    parser = argparse.ArgumentParser(description="Plot SSH/TLS CSV artifact summaries.")
    parser.add_argument("--csv-dir", type=Path, default=CSV_DIR, help="Directory containing SSH/TLS CSV summaries.")
    parser.add_argument("--figure-dir", type=Path, default=FIGURE_DIR, help="Directory to write figure files.")
    args = parser.parse_args()

    csv_dir: Path = args.csv_dir
    figure_dir: Path = args.figure_dir
    figure_dir.mkdir(parents=True, exist_ok=True)

    date_label = load_metadata_date()

    ssh_records = load_csv_records(csv_dir / "ssh_fingerprints.csv")
    tls_records = load_csv_records(csv_dir / "tls_fingerprints.csv")

    tls_counts = count_fingerprints(_filtered(tls_records, "TLS"))
    ssh_counts = count_fingerprints(_filtered(ssh_records, "SSH"))

    plot_bar(
        tls_counts,
        title="TLS FINGERPRINT DISTRIBUTION",
        x_label="HOST / FINGERPRINT",
        output_base=figure_dir / "tls_fingerprint_distribution",
        date_label=date_label,
    )
    plot_bar(
        ssh_counts,
        title="SSH HOST-KEY DISTRIBUTION",
        x_label="HOST / FINGERPRINT",
        output_base=figure_dir / "ssh_fingerprint_distribution",
        date_label=date_label,
        bar_color=(75, 140, 60),
    )

    summary_rows = [
        build_summary(tls_records, "TLS FINGERPRINTS"),
        build_summary(ssh_records, "SSH FINGERPRINTS"),
    ]
    plot_table(summary_rows, "CAPTURE SUMMARY", figure_dir / "capture_summary", date_label)


if __name__ == "__main__":
    main()
