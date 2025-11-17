# SSH/TLS Fingerprint Visualizer

Collect, baseline, and visualize **SSH host-key** and **TLS certificate** fingerprints to verify hosts and detect drift.  
Solo scope uses SHA-256 for SSH/TLS. Pair uplift adds **JA3/JA3S** (from PCAP), drift alerts, and richer charts.

---

## Overview

- **Why:** TOFU prompts and ad-hoc checks make it easy to miss key/cert changes or MITM.  
- **What:** Simple CLI collectors + CSV/JSON storage + small visualizations to make changes obvious.  
- **How:** Reproducible environment with `make bootstrap` and an optional Docker dev container.

---

## Features

- TLS server certificate **SHA-256** fingerprinting
- SSH host key **SHA-256** fingerprinting (wraps `ssh-keyscan`/`ssh-keygen`)
- CSV/JSON outputs with timestamps for easy baselining & diffs
- (Pair) Parse PCAPs for **JA3/JA3S** and visualize distributions

---

## Quick Start

Prereqs (any OS):
- **Python 3.11+**
- (Optional) **Docker Desktop**
- For SSH collector: OpenSSH client tools (`ssh-keyscan`, `ssh-keygen`)

```bash
# 1) one-command bootstrap (creates .venv and tries docker-compose up -d)
make bootstrap

# 2) (optional) activate local venv if you’re not using Docker
source .venv/bin/activate   # Linux/macOS
# .venv\Scripts\activate    # Windows PowerShell

# 3) run collectors
python scripts/fingerprint_tls.py example.com:443
python scripts/fingerprint_ssh.py github.com
```

## Project Board

We track work on a simple Kanban board with four columns: **Backlog → In Progress → Review → Done**.

**Milestones:** Week 10 (Proposal) • Week 12 (Alpha) • Week 14 (Beta) • Week 16 (Final)

See details and task list in [`docs/PROJECT_BOARD.md`](docs/PROJECT_BOARD.md).
