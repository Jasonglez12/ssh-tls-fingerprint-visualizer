# Architecture Documentation

## System Overview

The SSH/TLS Fingerprint Visualizer is a C++17-based CLI tool for collecting, baselining, and visualizing SSH host-key and TLS certificate fingerprints to detect changes and potential security issues.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Fingerprint Visualizer                    │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   TLS        │  │   SSH        │  │   PCAP       │      │
│  │  Collector   │  │  Collector   │  │  Analyzer    │      │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘      │
│         │                  │                  │              │
│         └──────────────────┼──────────────────┘              │
│                            │                                  │
│                    ┌───────▼────────┐                        │
│                    │  Common        │                        │
│                    │  Storage       │                        │
│                    │  (JSON/CSV)    │                        │
│                    └───────┬────────┘                        │
│                            │                                  │
│         ┌──────────────────┼──────────────────┐              │
│         │                  │                  │              │
│  ┌──────▼──────┐  ┌───────▼──────┐  ┌───────▼──────┐       │
│  │  Baseline   │  │ Visualization │  │   Alerts     │       │
│  │  Manager    │  │   Generator   │  │   Manager    │       │
│  └─────────────┘  └───────────────┘  └──────────────┘       │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

## Component Descriptions

### 1. Collectors

**TLS Collector (`fingerprint_tls`)**
- Connects to TLS/SSL servers
- Extracts server certificates
- Calculates SHA-256 fingerprints
- Stores results in JSON/CSV format

**SSH Collector (`fingerprint_ssh`)**
- Uses `ssh-keyscan` to retrieve host keys
- Calculates SHA-256 fingerprints
- Supports multiple key types (RSA, ECDSA, ED25519)

**PCAP Analyzer (`fingerprint_pcap`)**
- Parses PCAP files using libpcap
- Extracts TLS handshakes
- Calculates JA3 (client) and JA3S (server) fingerprints
- Pairs ClientHello/ServerHello messages

### 2. Storage Layer

**FingerprintStorage**
- Unified storage interface
- Supports JSON and CSV formats
- Timestamp tracking
- Type-aware storage (TLS, SSH, JA3, JA3S)

### 3. Analysis Tools

**Baseline Manager (`baseline_diff`)**
- Creates baseline snapshots
- Compares current fingerprints against baseline
- Detects changes, new hosts, and unchanged fingerprints
- Integrates with alert system

**Visualization Generator (`visualize`)**
- Generates timeline charts
- Creates summary statistics
- Exports JSON data for external tools
- ASCII table output

### 4. Alert System

**AlertManager**
- Console alerts (always enabled)
- Webhook alerts (via libcurl)
- Email alerts (infrastructure ready)
- Configurable alert routing

## Data Flow

### Collection Flow
```
Host Input → Collector → Certificate/Key Extraction → SHA-256 Hash → Storage
```

### Analysis Flow
```
Storage → Baseline Creation → Comparison → Change Detection → Alert → Visualization
```

### PCAP Analysis Flow
```
PCAP File → Packet Parsing → TLS Handshake Extraction → JA3/JA3S Calculation → Storage
```

## Key Design Decisions

1. **File-based Storage**: Simple, portable, easy to inspect
2. **Modular Architecture**: Each tool is independent, share common library
3. **Cross-platform**: Supports Windows, Linux, macOS
4. **Optional Dependencies**: Graceful degradation when libpcap/libcurl unavailable
5. **Non-root Execution**: Docker containers run as non-privileged user

## Technology Stack

- **Language**: C++17
- **Build System**: CMake 3.15+
- **Dependencies**:
  - OpenSSL (required) - TLS/SSL operations, hashing
  - libpcap (optional) - PCAP file parsing
  - libcurl (optional) - Webhook alerts
- **Container**: Docker with multi-stage build

## Security Considerations

- Input validation on all user inputs
- Least-privilege execution in containers
- No execution of untrusted external commands
- Secure handling of network connections
- File path sanitization

## Performance Characteristics

- Fingerprint collection: ~1-2 seconds per host
- Baseline comparison: O(n) where n = number of fingerprints
- PCAP processing: Depends on file size, typically 10-100 MB/min
- Memory usage: Minimal, processes one fingerprint at a time

## Extension Points

The architecture supports:
- Additional fingerprint types (JA4, HASSH, etc.)
- Custom alert backends
- Alternative storage backends (database, cloud storage)
- Real-time monitoring (vs. batch processing)

