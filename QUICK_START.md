# Quick Start Guide

## 🚀 Building the Project

### Windows (PowerShell)

Since you're on Windows, here's the step-by-step process:

#### Step 1: Install Prerequisites

**Required:**
```powershell
# Install CMake (if not already installed)
# Download from: https://cmake.org/download/
# Or use chocolatey:
choco install cmake

# Install OpenSSL
# Download from: https://slproweb.com/products/Win32OpenSSL.html
# Or use vcpkg:
vcpkg install openssl:x64-windows
```

**Optional (for PCAP support):**
```powershell
# Install Npcap (WinPcap replacement)
# Download from: https://nmap.org/npcap/
# Note: For development headers, you may need WinPcap SDK
```

**Optional (for webhook alerts):**
```powershell
# Install libcurl (usually included with Windows 10+)
# Or use vcpkg:
vcpkg install curl:x64-windows
```

#### Step 2: Build the Project

```powershell
# Navigate to project directory
cd C:\Users\nickz\OneDrive\Documents\GitHub\ssh-tls-fingerprint-visualizer

# Create build directory
mkdir build
cd build

# Configure with CMake (Visual Studio)
cmake .. -G "Visual Studio 17 2022"

# Or if using MinGW:
# cmake .. -G "MinGW Makefiles"

# Build (Release mode)
cmake --build . --config Release
```

The executables will be in `build\Release\` (or `build\` for MinGW).

---

## 🎯 Running the Tools

### Basic Usage

#### 1. Collect TLS Fingerprints
```powershell
# From the build/Release directory:
.\fingerprint_tls.exe github.com:443
.\fingerprint_tls.exe google.com:443

# This saves fingerprints to data/tls_fingerprints.json
```

#### 2. Collect SSH Fingerprints
```powershell
.\fingerprint_ssh.exe github.com
.\fingerprint_ssh.exe gitlab.com

# This saves fingerprints to data/ssh_fingerprints.json
```

#### 3. Generate Evaluation Dataset
```powershell
.\generate_eval_set.exe

# Collects fingerprints from common hosts for testing
```

#### 4. Create Baseline
```powershell
.\baseline_diff.exe create --baseline data\baseline.json

# Creates a baseline from all current fingerprints
```

#### 5. Compare Against Baseline
```powershell
# First, collect some new fingerprints
.\fingerprint_tls.exe github.com:443

# Then compare
.\baseline_diff.exe diff --baseline data\baseline.json

# If changes detected, exit code will be 1
```

#### 6. Visualize Fingerprints
```powershell
.\visualize.exe summary --data-dir data
.\visualize.exe timeline --data-dir data --output data\timeline.json

# Shows ASCII table and generates JSON for external tools
```

---

## 🔥 New Week 16 Features

### PCAP Analysis (JA3/JA3S Fingerprinting)

**Note:** Requires libpcap. On Windows, you may need to compile with libpcap headers or skip this feature.

```powershell
# Analyze a PCAP file
.\fingerprint_pcap.exe capture.pcap --data-dir data --stats

# This will:
# - Extract TLS handshakes from PCAP
# - Calculate JA3 (client) and JA3S (server) fingerprints
# - Save to data/ja3_fingerprints.json and data/ja3s_fingerprints.json
# - Print statistics about fingerprint distribution
```

**If PCAP support isn't compiled:**
- The tool will show an error message
- Install libpcap development headers and rebuild
- Or use WSL2/Linux subsystem for PCAP analysis

### Alerts with Baseline Diff

```powershell
# Compare with webhook alert (if libcurl available)
.\baseline_diff.exe diff --baseline data\baseline.json --alert-webhook https://hooks.slack.com/services/YOUR/WEBHOOK/URL

# Or create an alert config file (alerts.conf):
# webhook_enabled=true
# webhook_url=https://your-webhook-url
.\baseline_diff.exe diff --baseline data\baseline.json --alert-config alerts.conf
```

---

## 📁 File Locations

After running tools, you'll find data in:

```
ssh-tls-fingerprint-visualizer/
├── build/
│   └── Release/          # (or build/ for MinGW)
│       ├── fingerprint_tls.exe
│       ├── fingerprint_ssh.exe
│       ├── baseline_diff.exe
│       ├── visualize.exe
│       ├── generate_eval_set.exe
│       └── fingerprint_pcap.exe  # (if PCAP support enabled)
└── data/
    ├── tls_fingerprints.json
    ├── tls_fingerprints.csv
    ├── ssh_fingerprints.json
    ├── ssh_fingerprints.csv
    ├── ja3_fingerprints.json     # (from PCAP analysis)
    ├── ja3s_fingerprints.json    # (from PCAP analysis)
    ├── baseline.json
    ├── timeline.json
    └── summary.json
```

---

## 🧪 Quick Test Workflow

Here's a complete test workflow:

```powershell
# 1. Build the project (from project root)
cd build
cmake --build . --config Release
cd Release

# 2. Collect some fingerprints
.\fingerprint_tls.exe github.com:443
.\fingerprint_tls.exe google.com:443
.\fingerprint_ssh.exe github.com

# 3. Generate evaluation dataset
.\generate_eval_set.exe

# 4. Create baseline
.\baseline_diff.exe create --baseline ..\..\data\baseline.json

# 5. Visualize
.\visualize.exe summary --data-dir ..\..\data

# 6. Collect new fingerprint and compare
.\fingerprint_tls.exe github.com:443
.\baseline_diff.exe diff --baseline ..\..\data\baseline.json

# Should show "UNCHANGED" if fingerprint is the same
```

---

## ⚠️ Troubleshooting

### "OpenSSL not found" error

```powershell
# Option 1: Use vcpkg
vcpkg install openssl:x64-windows
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake

# Option 2: Specify OpenSSL path
cmake .. -DOPENSSL_ROOT_DIR=C:\OpenSSL-Win64
```

### "libpcap support not compiled in"

This is expected if you don't have libpcap. The project will still build and work, just without PCAP analysis.

### "ssh-keyscan not found" (SSH collector)

```powershell
# Install Git for Windows (includes OpenSSH)
# Or install OpenSSH for Windows separately
```

### Executables not found

Make sure you're in the correct directory:
- Visual Studio builds: `build\Release\`
- MinGW builds: `build\`
- Or add the build directory to your PATH

---

## 📖 More Information

- **Building details:** See `BUILD.md`
- **PCAP analysis:** See `PCAP_ANALYSIS.md`
- **Beta features:** See `BETA_FEATURES.md`
- **Project itinerary:** See `PROJECT_ITINERARY.md`

---

## 💡 Tips

1. **Use PowerShell in Admin mode** if you need to install packages
2. **Use WSL2** if you want full PCAP support without Windows-specific issues
3. **Check CMake output** for which optional features were found
4. **Data directory** is created automatically when you run tools
5. **JSON files** can be viewed/edited with any text editor

