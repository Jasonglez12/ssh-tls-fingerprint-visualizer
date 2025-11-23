# Beta Features Documentation

This document provides detailed information about the Beta features (Week 14) of the SSH/TLS Fingerprint Visualizer.

## Overview

The Beta version includes:
1. **Baseline Diff Functionality** - Compare fingerprints against a baseline to detect changes
2. **Simple Charts/Visualizations** - Visualize fingerprint changes over time
3. **Evaluation Dataset** - Generate test data with known-good fingerprints

## Baseline Diff

The baseline diff tool allows you to:
- Create a baseline from current fingerprints
- Compare new fingerprints against the baseline
- Detect changes (potential security issues!)

### Creating a Baseline

```bash
python scripts/baseline_diff.py create --baseline data/baseline.json
```

This creates a baseline file containing all current fingerprints. Use this after you've collected initial fingerprints from your infrastructure.

### Comparing Against Baseline

```bash
python scripts/baseline_diff.py diff --baseline data/baseline.json
```

This compares current fingerprints against the baseline and shows:
- **CHANGED** - Fingerprints that differ from baseline (⚠️ potential security issue!)
- **NEW** - Hosts not in baseline (new additions)
- **UNCHANGED** - Fingerprints matching baseline (✅ good)

The tool exits with code 1 if changes are detected, allowing use in automated monitoring.

### Example Output

```
=== Baseline Diff Results ===

⚠️  CHANGED (1):
  TLS example.com:443
    Baseline: AA:BB:CC:DD:EE:FF:...
    Current:  FF:EE:DD:CC:BB:AA:...

➕ NEW (1):
  SSH newserver.example.com:22: 11:22:33:44:55:66:...

✅ UNCHANGED (5):
  TLS github.com:443
  SSH github.com:22
  ...
```

## Visualization

The visualization tool generates charts showing fingerprint changes over time.

### Timeline Chart

Shows fingerprint changes over time for each host:

```bash
python scripts/visualize.py timeline --data-dir data --output data/timeline.png
```

This creates a line chart showing:
- X-axis: Time
- Y-axis: Number of unique fingerprints per host
- Multiple lines for different hosts

### Summary Chart

Shows fingerprint counts and changes per host:

```bash
python scripts/visualize.py summary --data-dir data --output data/summary.png
```

This creates two bar charts:
- Left: Collection counts per host (TLS vs SSH)
- Right: Unique fingerprints detected (changes) per host

### Generate Both Charts

```bash
python scripts/visualize.py all --data-dir data --output data/charts.png
```

This generates both timeline and summary charts (with different filenames).

## Evaluation Dataset

The evaluation dataset generator collects fingerprints from common hosts for testing.

### Generate Default Evaluation Dataset

```bash
python scripts/generate_eval_set.py
```

This collects fingerprints from:
- github.com (TLS:443, SSH:22)
- google.com (TLS:443)
- example.com (TLS:443)
- www.cloudflare.com (TLS:443)

### Generate Custom Evaluation Dataset

```bash
python scripts/generate_eval_set.py --hosts TLS:example.com:443 SSH:github.com:22
```

This allows you to specify custom hosts in the format `type:host:port`.

The evaluation dataset is saved to:
- `data/tls_fingerprints.json` / `.csv`
- `data/ssh_fingerprints.json` / `.csv`
- `data/eval_metadata.json` - Metadata about the evaluation run

## Usage Workflow

### Initial Setup

1. **Collect initial fingerprints:**
   ```bash
   python scripts/fingerprint_tls.py github.com:443
   python scripts/fingerprint_ssh.py github.com
   ```

2. **Generate evaluation dataset:**
   ```bash
   python scripts/generate_eval_set.py
   ```

3. **Create baseline:**
   ```bash
   python scripts/baseline_diff.py create --baseline data/baseline.json
   ```

### Ongoing Monitoring

1. **Collect new fingerprints periodically:**
   ```bash
   python scripts/fingerprint_tls.py github.com:443
   python scripts/fingerprint_ssh.py github.com
   ```

2. **Check for changes:**
   ```bash
   python scripts/baseline_diff.py diff --baseline data/baseline.json
   ```

3. **Visualize changes:**
   ```bash
   python scripts/visualize.py timeline --data-dir data --output data/timeline.png
   ```

### Automated Monitoring

You can use the baseline diff tool in automated scripts:

```bash
#!/bin/bash
# Collect fingerprints
python scripts/fingerprint_tls.py example.com:443

# Check for changes
if python scripts/baseline_diff.py diff --baseline data/baseline.json; then
    echo "No changes detected"
else
    echo "ALERT: Fingerprint changes detected!"
    # Send alert, update baseline, etc.
fi
```

## Data Files

All data files are stored in the `data/` directory:

- `tls_fingerprints.csv` / `tls_fingerprints.json` - TLS certificate fingerprints
- `ssh_fingerprints.csv` / `ssh_fingerprints.json` - SSH host key fingerprints
- `baseline.json` - Baseline fingerprints (created by `baseline_diff.py create`)
- `eval_metadata.json` - Evaluation dataset metadata

## Command Reference

### Baseline Diff

```bash
# Create baseline
python scripts/baseline_diff.py create [--baseline PATH] [--data-dir PATH] [--type TLS|SSH]

# Compare against baseline
python scripts/baseline_diff.py diff [--baseline PATH] [--data-dir PATH] [--type TLS|SSH] [--output PATH]
```

### Visualization

```bash
# Timeline chart
python scripts/visualize.py timeline [--data-dir PATH] [--type TLS|SSH] [--output PATH]

# Summary chart
python scripts/visualize.py summary [--data-dir PATH] [--type TLS|SSH] [--output PATH]

# Both charts
python scripts/visualize.py all [--data-dir PATH] [--type TLS|SSH] [--output PATH]
```

### Evaluation Dataset

```bash
# Default hosts
python scripts/generate_eval_set.py [--data-dir PATH] [--timeout SECONDS]

# Custom hosts
python scripts/generate_eval_set.py [--hosts TYPE:HOST:PORT ...] [--data-dir PATH] [--timeout SECONDS]
```

## Notes

- The baseline diff tool exits with code 0 if no changes are detected, code 1 if changes are found
- Charts are saved as PNG files (default) or displayed in a window if `--output` is not specified
- Evaluation dataset metadata includes success/failure information for troubleshooting
- All timestamps are in ISO 8601 format for easy parsing

