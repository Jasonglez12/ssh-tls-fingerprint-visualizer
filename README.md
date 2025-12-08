# SSH/TLS Fingerprint Visualizer

[![CI](https://github.com/Jasonglez12/ssh-tls-fingerprint-visualizer/workflows/CI/badge.svg)](https://github.com/Jasonglez12/ssh-tls-fingerprint-visualizer/actions)

Collect, baseline, and visualize **SSH host-key** and **TLS certificate** fingerprints to verify hosts and detect drift.  
Solo scope uses SHA-256 for SSH/TLS. Pair uplift adds **JA3/JA3S** (from PCAP), drift alerts, and richer charts.

---

## Overview

- **Why:** TOFU prompts and ad-hoc checks make it easy to miss key/cert changes or MITM.  
- **What:** Simple CLI collectors + CSV/JSON storage + small visualizations to make changes obvious.  
- **How:** Reproducible environment with `make bootstrap` and an optional Docker dev container.

---

## Features

### Alpha Features (Week 12)
- TLS server certificate **SHA-256** fingerprinting
- SSH host key **SHA-256** fingerprinting (wraps `ssh-keyscan`/`ssh-keygen`)
- CSV/JSON outputs with timestamps for easy baselining & diffs

### Beta Features (Week 14) ✨
- **Baseline diffs** - Compare fingerprints against baseline and detect changes
- **Simple charts** - Visualize fingerprint changes over time
- **Evaluation dataset** - Generate test data with known-good fingerprints

### Final Features (Week 16) ✨
- **JA3/JA3S fingerprinting** - Extract TLS client/server fingerprints from PCAP files
- **PCAP analysis tool** - `fingerprint_pcap` for analyzing network captures
- **Advanced drift alerts** - Webhook integration for automated monitoring
- **Enhanced visualizations** - Support for JA3/JA3S in charts and statistics

---

## Quick Start

### Using Docker (Recommended)

**Bash/Linux/macOS/Git Bash:**
```bash
# 1. Clone repository
git clone <repository-url>
cd ssh-tls-fingerprint-visualizer

# 2. Build Docker image and start container
make up

# 3. Run complete end-to-end demonstration
make demo

# 4. Stop container when done
make down
```

**Windows PowerShell:**
```powershell
# 1. Clone repository
git clone <repository-url>
cd ssh-tls-fingerprint-visualizer

# 2. Build Docker image
docker compose build

# 3. Start container
docker compose up -d

# 4. Run complete end-to-end demonstration
docker compose exec -T app /bin/bash /app/scripts/demo.sh

# 5. Stop container when done
docker compose down
```

📖 **Windows users:** See [WINDOWS_DOCKER.md](WINDOWS_DOCKER.md) for detailed Windows instructions.

**What this does:**
- Builds a Docker image with all dependencies (reproducible)
- Compiles all executables inside the container
- Runs a full demo: collect → baseline → compare → visualize
- Generates artifacts in `artifacts/release/`

**For interactive use:**
```bash
# Enter the container
docker compose exec app /bin/bash

# Run individual tools
/app/fingerprint_tls github.com:443 --data-dir /app/data
/app/visualize summary --data-dir /app/data
```

📖 **See [DOCKER_GUIDE.md](DOCKER_GUIDE.md) for detailed Docker instructions and troubleshooting.**  
📖 **Windows users:** See [WINDOWS_DOCKER.md](WINDOWS_DOCKER.md) for PowerShell-specific instructions.

### Native Build

#### Prerequisites

- **C++17 compiler** (GCC 8+, Clang 8+, MSVC 2019+)
- **CMake 3.15+**
- **OpenSSL development libraries**
- For SSH collector: OpenSSH client tools (`ssh-keyscan`, `ssh-keygen`)
- **Optional for PCAP support:** libpcap development libraries
- **Optional for webhook alerts:** libcurl development libraries

### Build

```bash
# 1) Create build directory
mkdir build && cd build

# 2) Configure with CMake
cmake ..

# 3) Build
cmake --build .

# On Windows (Visual Studio):
# cmake --build . --config Release

# On Unix/macOS:
# make
```

### Usage

```bash
# Binary executables will be in build/ directory

# 3) Run collectors
./build/fingerprint_tls example.com:443
./build/fingerprint_ssh github.com

# 4) (Beta) Generate evaluation dataset
./build/generate_eval_set

# 5) (Beta) Create baseline
./build/baseline_diff create --baseline data/baseline.json

# 6) (Beta) Compare against baseline (after collecting more fingerprints)
./build/baseline_diff diff --baseline data/baseline.json

# 7) (Beta) Visualize fingerprint changes (generates JSON data files)
./build/visualize timeline --data-dir data --output data/timeline.json
./build/visualize summary --data-dir data --output data/summary.json

# 8) (Week 16) Extract JA3/JA3S fingerprints from PCAP file
./build/fingerprint_pcap capture.pcap --data-dir data --stats

# 9) (Week 16) Baseline diff with alerts
./build/baseline_diff diff --baseline data/baseline.json --alert-webhook https://hooks.slack.com/services/YOUR/WEBHOOK/URL
```

## Documentation

### Getting Started
- **[Docker Guide](DOCKER_GUIDE.md)** - Comprehensive Docker instructions for reproducible setup ⭐
- **[Reproducible Setup](REPRODUCIBLE_SETUP.md)** - Quick reference for one-command setup
- **[Quick Start](QUICK_START.md)** - Quick start guide for Windows users
- **[Build Instructions](BUILD.md)** - Detailed build instructions
- **[Run Instructions](RUN_INSTRUCTIONS.md)** - How to run the project

### Technical Documentation
- **[Architecture](docs/ARCHITECTURE.md)** - System design, components, and data flow
- **[Runbook](docs/RUNBOOK.md)** - Operational guide with rebuild instructions and troubleshooting
- **[Security](docs/SECURITY.md)** - Security invariants, hardening measures, and threat model
- **[Summary](docs/SUMMARY.md)** - What works and what's next
- **[Evaluation](EVALUATION.md)** - Initial results and observations
- **[PCAP Analysis Guide](PCAP_ANALYSIS.md)** - Using the PCAP analysis tool

**Milestones:** Week 10 (Proposal) ✅ • Week 12 (Alpha) ✅ • Week 14 (Beta) ✅ • Week 16 (Final) ✅

---

## Beta Features Usage

### Baseline Management

Create a baseline from current fingerprints:
```bash
./build/baseline_diff create --baseline data/baseline.json
```

Compare current fingerprints against baseline:
```bash
./build/baseline_diff diff --baseline data/baseline.json
```

The diff command will show:
- ⚠️ **CHANGED** - Fingerprints that differ from baseline (potential security issue!)
- ➕ **NEW** - Hosts not in baseline
- ✅ **UNCHANGED** - Fingerprints matching baseline

### Visualization

The visualization tool generates JSON data files that can be used with external visualization tools (e.g., gnuplot, matplotlib, D3.js).

Generate timeline data showing fingerprint changes over time:
```bash
./build/visualize timeline --data-dir data --output data/timeline.json
```

Generate summary data showing fingerprint counts per host:
```bash
./build/visualize summary --data-dir data --output data/summary.json
```

Generate both data files:
```bash
./build/visualize all --data-dir data --output data/charts.json
```

The tool also prints an ASCII summary table to the console.

### Evaluation Dataset

Generate a small evaluation dataset with known-good fingerprints:
```bash
./build/generate_eval_set
```

This will collect fingerprints from common hosts (github.com, google.com, etc.) for testing purposes.

You can also specify custom hosts:
```bash
./build/generate_eval_set --hosts TLS:github.com:443 SSH:github.com:22
```

---

## Data Files

Fingerprints are stored in the `data/` directory:
- `tls_fingerprints.csv` / `tls_fingerprints.json` - TLS certificate fingerprints
- `ssh_fingerprints.csv` / `ssh_fingerprints.json` - SSH host key fingerprints
- `baseline.json` - Baseline fingerprints for comparison (created with `baseline_diff.py create`)
- `eval_metadata.json` - Evaluation dataset metadata

---

## Programs

- `fingerprint_tls` - Collect TLS certificate fingerprints
- `fingerprint_ssh` - Collect SSH host key fingerprints
- `baseline_diff` - Baseline management and comparison (Beta)
- `visualize` - Generate visualization data files (Beta)
- `generate_eval_set` - Generate evaluation dataset (Beta)
- `fingerprint_pcap` - Extract JA3/JA3S fingerprints from PCAP files (Week 16)

## Project Structure

```
.
├── CMakeLists.txt          # CMake build configuration
├── include/                # Header files
│   ├── utils.h            # Utility functions
│   └── fingerprint_common.h # Common fingerprint operations
├── src/                    # Source files
│   ├── utils.cpp
│   ├── fingerprint_common.cpp
│   ├── fingerprint_tls.cpp # TLS collector
│   ├── fingerprint_ssh.cpp # SSH collector
│   ├── baseline_diff.cpp   # Baseline comparison tool
│   ├── visualize.cpp       # Visualization tool
│   └── generate_eval_set.cpp # Evaluation dataset generator
└── build/                  # Build directory (created during build)
```

## Building on Windows

Using Visual Studio:
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

Using MinGW:
```powershell
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

## Building on Linux/macOS

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)  # Linux
# make -j$(sysctl -n hw.ncpu)  # macOS
```
