# PCAP Analysis with JA3/JA3S Fingerprinting

This document describes how to use the `fingerprint_pcap` tool to extract JA3/JA3S fingerprints from PCAP files.

## Overview

The `fingerprint_pcap` tool analyzes PCAP files to extract TLS handshake information and calculate JA3 (client) and JA3S (server) fingerprints. These fingerprints can be used to identify specific TLS client and server implementations.

## Prerequisites

### Installing libpcap

**Linux (Debian/Ubuntu):**
```bash
sudo apt-get install libpcap-dev
```

**Linux (RHEL/CentOS):**
```bash
sudo yum install libpcap-devel
```

**macOS:**
```bash
brew install libpcap
```

**Windows:**
1. Install Npcap from https://nmap.org/npcap/
2. Install WinPcap development headers (optional, for compilation)

Note: If libpcap is not available, the tool will still compile but PCAP parsing will be disabled.

## Usage

### Basic Usage

```bash
./build/fingerprint_pcap <pcap_file> [options]
```

### Options

- `--data-dir DIR` - Data directory for storing fingerprints (default: `data`)
- `--output FORMAT` - Output format: `json`, `csv`, or `both` (default: `both`)
- `--stats` - Print statistics about fingerprint distribution

### Examples

#### Basic PCAP Analysis
```bash
./build/fingerprint_pcap capture.pcap
```

This will:
1. Parse the PCAP file
2. Extract TLS handshake packets
3. Calculate JA3/JA3S fingerprints
4. Save results to `data/ja3_fingerprints.json` and `data/ja3_fingerprints.csv`

#### With Statistics
```bash
./build/fingerprint_pcap capture.pcap --stats
```

This will also print:
- Total number of TLS handshakes processed
- Number of unique JA3 fingerprints
- Number of unique JA3S fingerprints
- Top 10 most common JA3 fingerprints
- Top 10 most common JA3S fingerprints

#### Custom Data Directory
```bash
./build/fingerprint_pcap capture.pcap --data-dir /path/to/data
```

## Understanding JA3/JA3S

### JA3 (Client Fingerprint)

JA3 is a method for fingerprinting TLS clients based on their ClientHello message. The fingerprint is calculated from:
- TLS Version
- Cipher Suites
- Extensions
- Elliptic Curves
- Elliptic Curve Point Formats

The result is an MD5 hash that uniquely identifies the TLS client implementation.

### JA3S (Server Fingerprint)

JA3S is a method for fingerprinting TLS servers based on their ServerHello message. The fingerprint is calculated from:
- TLS Version
- Selected Cipher Suite
- Extensions

The result is an MD5 hash that uniquely identifies the TLS server implementation.

## Output Format

### JSON Output

Fingerprints are saved to `data/ja3_fingerprints.json`:

```json
[
  {
    "timestamp": "2025-01-15T10:30:45.123456Z",
    "type": "JA3",
    "host": "192.168.1.100",
    "port": 443,
    "fingerprint": "a1b2c3d4e5f6..."
  },
  {
    "timestamp": "2025-01-15T10:30:45.234567Z",
    "type": "JA3S",
    "host": "192.168.1.100",
    "port": 443,
    "fingerprint": "f6e5d4c3b2a1..."
  }
]
```

### CSV Output

Fingerprints are also saved to `data/ja3_fingerprints.csv`:

```csv
timestamp,type,host,port,fingerprint
2025-01-15T10:30:45.123456Z,JA3,192.168.1.100,443,a1b2c3d4e5f6...
2025-01-15T10:30:45.234567Z,JA3S,192.168.1.100,443,f6e5d4c3b2a1...
```

## Integration with Other Tools

### Visualization

You can visualize JA3/JA3S fingerprints using the `visualize` tool:

```bash
# Generate timeline visualization including JA3/JA3S
./build/visualize timeline --data-dir data --output data/timeline.json

# Generate summary with JA3/JA3S counts
./build/visualize summary --data-dir data --output data/summary.json
```

### Baseline Comparison

JA3/JA3S fingerprints are included in baseline comparisons:

```bash
# Create baseline (includes JA3/JA3S if present)
./build/baseline_diff create --baseline data/baseline.json

# Compare against baseline (will detect JA3/JA3S changes)
./build/baseline_diff diff --baseline data/baseline.json
```

## Use Cases

1. **Client Identification** - Identify specific browsers, tools, or applications based on JA3 fingerprints
2. **Server Identification** - Identify TLS server implementations based on JA3S fingerprints
3. **Anomaly Detection** - Detect unusual TLS clients or servers in network traffic
4. **Security Monitoring** - Track changes in TLS configurations over time

## Limitations

- Only processes TLS handshake packets (ClientHello and ServerHello)
- Requires proper TCP stream reassembly (may miss fragmented handshakes)
- PCAP file must contain complete TLS handshake packets
- IPv4 and IPv6 are supported, but complex tunneling may not be handled

## Troubleshooting

### "libpcap support not compiled in"

If you see this error, install libpcap development libraries and rebuild:

```bash
# Install libpcap-dev
sudo apt-get install libpcap-dev  # Debian/Ubuntu
# or
sudo yum install libpcap-devel    # RHEL/CentOS
# or
brew install libpcap              # macOS

# Rebuild
cd build
cmake ..
make
```

### "No TLS handshakes found"

Possible reasons:
- PCAP file doesn't contain TLS traffic
- TLS packets are fragmented and not properly reassembled
- Port is not 443 or 8443 (tool only checks common TLS ports)

### Performance Issues

For large PCAP files:
- Process may take some time depending on file size
- Memory usage scales with number of unique connections
- Consider filtering PCAP files to TLS traffic only before processing

## References

- [JA3 Specification](https://github.com/salesforce/ja3)
- [JA3S Specification](https://github.com/salesforce/ja3s)
- [libpcap Documentation](https://www.tcpdump.org/manpages/pcap.3pcap.html)

