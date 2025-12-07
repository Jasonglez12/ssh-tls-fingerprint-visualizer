# Building SSH/TLS Fingerprint Visualizer

This project is written in C++17 and uses CMake for building.

## Prerequisites

### Required
- **C++17 compiler**
  - GCC 8+ (Linux)
  - Clang 8+ (macOS/Linux)
  - MSVC 2019+ (Windows)
- **CMake 3.15+**
- **OpenSSL development libraries**
  - Linux: `sudo apt-get install libssl-dev` (Debian/Ubuntu)
  - Linux: `sudo yum install openssl-devel` (RHEL/CentOS)
  - macOS: `brew install openssl`
  - Windows: Install from [OpenSSL website](https://slproweb.com/products/Win32OpenSSL.html) or use vcpkg

### Optional
- **OpenSSH client tools** (required for SSH collector)
  - Linux: Usually pre-installed
  - macOS: Usually pre-installed
  - Windows: Install [Git for Windows](https://git-scm.com/download/win) or OpenSSH for Windows

## Build Instructions

### Linux/macOS

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)  # Linux
# make -j$(sysctl -n hw.ncpu)  # macOS

# Run tests/executables
./fingerprint_tls github.com:443
./fingerprint_ssh github.com
```

### Windows (Visual Studio)

```powershell
# Create build directory
mkdir build
cd build

# Configure with CMake (adjust version as needed)
cmake .. -G "Visual Studio 17 2022"

# Build Release configuration
cmake --build . --config Release

# Run executables (adjust path based on your build)
.\Release\fingerprint_tls.exe github.com:443
.\Release\fingerprint_ssh.exe github.com
```

### Windows (MinGW)

```powershell
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "MinGW Makefiles"

# Build
cmake --build .

# Run executables
.\fingerprint_tls.exe github.com:443
.\fingerprint_ssh.exe github.com
```

## CMake Options

You can customize the build with CMake options:

```bash
# Set install prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local

# Set build type (Debug/Release)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build shared libraries
cmake .. -DBUILD_SHARED_LIBS=ON
```

## Troubleshooting

### OpenSSL not found

If CMake can't find OpenSSL:

```bash
# Linux
sudo apt-get install libssl-dev

# macOS
brew install openssl
export PKG_CONFIG_PATH="/usr/local/opt/openssl/lib/pkgconfig"

# Windows
# Install OpenSSL and set OPENSSL_ROOT_DIR
cmake .. -DOPENSSL_ROOT_DIR=C:/OpenSSL-Win64
```

### Build errors on Windows

- Make sure you have the correct Visual Studio version
- Ensure OpenSSL is properly installed and in PATH
- Try using vcpkg to manage dependencies:
  ```powershell
  vcpkg install openssl:x64-windows
  cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
  ```

### ssh-keyscan not found (SSH collector)

- Linux/macOS: Install OpenSSH client
- Windows: Install Git for Windows or OpenSSH for Windows
- The SSH collector will fail gracefully if ssh-keyscan is not available

## Reproducibility

The repository ships a fully automated demo run to make deterministic outputs easy to regenerate.

1. Start from a clean slate and boot the helper container (optional but verified as part of the flow):

   ```bash
   make clean
   make up
   ```

2. Run the deterministic demo pipeline:

   ```bash
   make demo
   ```

   - Uses `data/demo_hosts.txt` for fixed input hosts (TLS: `example.com:443`, SSH: `github.com:22`).
   - Applies a default timestamp `2024-01-01T00:00:00.000Z` and seed `1337` for every collector call. Override with `DEMO_TIMESTAMP=<iso8601>` and `DEMO_SEED=<int>` if needed.
   - Captures the execution log at `release/artifacts/demo_run.log`.

3. Inspect reproducible artifacts (all timestamps normalized):

   - `release/artifacts/tls_fingerprints.csv` / `.json`
   - `release/artifacts/ssh_fingerprints.csv` / `.json`
   - `release/artifacts/eval_metadata.json`
   - `release/artifacts/baseline.json`
   - `release/artifacts/SHA256SUMS.txt` (hashes over every artifact for quick verification)

4. Verify the run by checking the printed hashes:

   ```bash
   cat release/artifacts/SHA256SUMS.txt
   ```

If you prefer to call binaries directly, each collector accepts reproducibility flags:

- `fingerprint_tls` and `fingerprint_ssh`: `--timestamp <iso8601>` to pin record timestamps.
- `generate_eval_set`: `--hosts-file <path>`, `--seed <int>` to deterministically shuffle, and `--timestamp <iso8601>` to propagate fixed timestamps through child collectors and metadata.
- `baseline_diff`: `--timestamp <iso8601>` to normalize baseline metadata output.

## Installation

After building, you can install the executables:

```bash
cmake --build . --target install
```

This will install the executables to `CMAKE_INSTALL_PREFIX/bin`.

