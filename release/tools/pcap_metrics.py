#!/usr/bin/env python3
"""
Utility for extracting JA3/JA3S fingerprints and basic timing metrics from PCAP captures.

The script shells out to `tshark` for parsing and writes results to CSV files under the
specified output directory (defaults to ``release/artifacts/csv``). It expects that
`tshark` is installed with JA3 fields enabled (modern builds expose ``tls.handshake.ja3``
and ``tls.handshake.ja3s``).
"""

from __future__ import annotations

import argparse
import csv
import statistics
import subprocess
from pathlib import Path
from typing import Iterable, Sequence


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract JA3/JA3S fingerprints and timing metrics from PCAP files."
    )
    parser.add_argument(
        "pcaps",
        nargs="+",
        help="Paths to PCAP files to parse (globs are not expanded automatically).",
    )
    # Command-line parameters are documented here to keep the parsing logic localized and
    # discoverable for operators tweaking tshark versions or output locations.
    parser.add_argument(
        "--output-dir",
        default=Path("release/artifacts/csv"),
        type=Path,
        help="Directory where ja3_summary.csv and timing_metrics.csv will be written.",
    )
    parser.add_argument(
        "--tshark-bin",
        default="tshark",
        help="Path to the tshark executable with JA3 fields available.",
    )
    return parser.parse_args()


def run_tshark(
    pcap: Path, fields: Sequence[str], display_filter: str | None, tshark_bin: str
) -> list[list[str]]:
    """Run tshark and return rows split on tabs for the requested fields."""
    cmd: list[str] = [tshark_bin, "-r", str(pcap), "-T", "fields"]
    for field in fields:
        cmd.extend(["-e", field])
    if display_filter:
        cmd.extend(["-Y", display_filter])

    result = subprocess.run(cmd, capture_output=True, text=True, check=False)
    if result.returncode != 0:
        raise RuntimeError(
            f"tshark failed for {pcap} (exit {result.returncode}): {result.stderr.strip()}"
        )

    return [line.split("\t") for line in result.stdout.splitlines() if line.strip()]


def _first_non_empty(values: Iterable[str]) -> str | None:
    for value in values:
        value = value.strip()
        if value:
            return value
    return None


def extract_ja3_values(pcap: Path, tshark_bin: str) -> tuple[list[str], list[str]]:
    """Return unique JA3 and JA3S values seen in the capture."""
    client_rows = run_tshark(
        pcap,
        fields=["tls.handshake.ja3", "ssl.handshake.ja3"],
        display_filter="tls.handshake.type == 1",
        tshark_bin=tshark_bin,
    )
    server_rows = run_tshark(
        pcap,
        fields=["tls.handshake.ja3s", "ssl.handshake.ja3s"],
        display_filter="tls.handshake.type == 2",
        tshark_bin=tshark_bin,
    )

    clients = {
        _first_non_empty(row) for row in client_rows if _first_non_empty(row)
    }
    servers = {
        _first_non_empty(row) for row in server_rows if _first_non_empty(row)
    }
    return sorted(clients), sorted(servers)


def extract_timing_metrics(pcap: Path, tshark_bin: str) -> dict[str, float]:
    """
    Compute simple timing metrics using frame timestamps.

    Values returned:
    - packet_count: total packets in the capture
    - duration_seconds: time delta between first and last frame
    - mean_interarrival: average time between packets
    - median_interarrival: median time between packets
    - max_interarrival: largest gap between packets
    """
    rows = run_tshark(
        pcap,
        fields=["frame.time_epoch"],
        display_filter=None,
        tshark_bin=tshark_bin,
    )
    timestamps = [float(row[0]) for row in rows if row and row[0].strip()]
    if not timestamps:
        return {
            "packet_count": 0,
            "duration_seconds": 0.0,
            "mean_interarrival": 0.0,
            "median_interarrival": 0.0,
            "max_interarrival": 0.0,
        }

    timestamps.sort()
    packet_count = len(timestamps)
    duration = timestamps[-1] - timestamps[0] if packet_count > 1 else 0.0

    deltas = [b - a for a, b in zip(timestamps[:-1], timestamps[1:])]
    mean_delta = statistics.mean(deltas) if deltas else 0.0
    median_delta = statistics.median(deltas) if deltas else 0.0
    max_delta = max(deltas) if deltas else 0.0

    return {
        "packet_count": packet_count,
        "duration_seconds": duration,
        "mean_interarrival": mean_delta,
        "median_interarrival": median_delta,
        "max_interarrival": max_delta,
    }


def write_ja3_summary(rows: list[dict[str, str]], output_dir: Path) -> None:
    path = output_dir / "ja3_summary.csv"
    with path.open("w", newline="") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=["pcap", "client_ja3", "server_ja3s", "client_count", "server_count"],
        )
        writer.writeheader()
        for row in rows:
            writer.writerow(row)


def write_timing_summary(rows: list[dict[str, str | float]], output_dir: Path) -> None:
    path = output_dir / "timing_metrics.csv"
    with path.open("w", newline="") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "pcap",
                "packet_count",
                "duration_seconds",
                "mean_interarrival",
                "median_interarrival",
                "max_interarrival",
            ],
        )
        writer.writeheader()
        for row in rows:
            writer.writerow(row)


def main() -> None:
    args = parse_args()

    output_dir: Path = args.output_dir
    output_dir.mkdir(parents=True, exist_ok=True)

    ja3_rows: list[dict[str, str]] = []
    timing_rows: list[dict[str, str | float]] = []

    for pcap_arg in args.pcaps:
        pcap_path = Path(pcap_arg)
        if not pcap_path.is_file():
            raise FileNotFoundError(f"PCAP does not exist: {pcap_path}")

        client_ja3, server_ja3 = extract_ja3_values(pcap_path, args.tshark_bin)
        timing = extract_timing_metrics(pcap_path, args.tshark_bin)

        ja3_rows.append(
            {
                "pcap": pcap_path.name,
                "client_ja3": ";".join(client_ja3),
                "server_ja3s": ";".join(server_ja3),
                "client_count": len(client_ja3),
                "server_count": len(server_ja3),
            }
        )

        timing_rows.append({"pcap": pcap_path.name, **timing})

    write_ja3_summary(ja3_rows, output_dir)
    write_timing_summary(timing_rows, output_dir)


if __name__ == "__main__":
    main()
