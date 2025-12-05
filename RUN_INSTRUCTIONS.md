# How to Run the Project

## Current Status

The build is encountering OpenSSL dependency issues. Here are the steps to resolve and run:

## Option 1: Quick Setup (Recommended)

### Step 1: Install OpenSSL Development Headers

**Download and install OpenSSL for Windows:**
- Visit: https://slproweb.com/products/Win32OpenSSL.html
- Download: **OpenSSL 3.x Win64** (not just the binaries, but the full installer)
- Install to: `C:\OpenSSL-Win64` (or note the path)

### Step 2: Run Setup Script

```powershell
# Run the setup script
.\setup_build.ps1
```

This will:
- Find OpenSSL automatically
- Configure CMake with the right settings
- Set up the build directory

### Step 3: Build

```powershell
cd build
cmake --build . --config Release
```

### Step 4: Run Tools

```powershell
cd Release

# Collect TLS fingerprint
.\fingerprint_tls.exe github.com:443

# Collect SSH fingerprint
.\fingerprint_ssh.exe github.com

# Visualize
.\visualize.exe summary --data-dir ..\..\data
```

---

## Option 2: Manual Setup

### Step 1: Install OpenSSL

Download OpenSSL from: https://slproweb.com/products/Win32OpenSSL.html
- Choose: **OpenSSL 3.x Win64**
- Install to default location

### Step 2: Configure CMake

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -DOPENSSL_ROOT_DIR="C:\Program Files\OpenSSL-Win64"
```

If OpenSSL is in a different location, adjust the path.

### Step 3: Build

```powershell
cmake --build . --config Release
```

### Step 4: Run

See Option 1, Step 4 above.

---

## Option 3: Use MSYS2/MINGW

If you have MSYS2 installed (which you do at `C:\msys64`):

### Step 1: Open MSYS2 MINGW64 Terminal

```powershell
C:\msys64\msys2_shell.cmd -mingw64
```

### Step 2: Navigate to Project

```bash
cd /c/Users/nickz/OneDrive/Documents/GitHub/ssh-tls-fingerprint-visualizer
```

### Step 3: Build

```bash
mkdir -p build
cd build
cmake .. -G "MinGW Makefiles"
make -j$(nproc)
```

### Step 4: Run

```bash
./fingerprint_tls.exe github.com:443
./fingerprint_ssh.exe github.com
./visualize.exe summary --data-dir ../data
```

---

## Option 4: Use WSL2 (Linux Subsystem)

If you have WSL2 installed:

### Step 1: Open WSL2

```powershell
wsl
```

### Step 2: Install Dependencies

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libssl-dev libpcap-dev
```

### Step 3: Navigate and Build

```bash
cd /mnt/c/Users/nickz/OneDrive/Documents/GitHub/ssh-tls-fingerprint-visualizer
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### Step 4: Run

```bash
./fingerprint_tls github.com:443
./fingerprint_ssh github.com
```

---

## Troubleshooting

### OpenSSL Not Found

**Problem:** `Could NOT find OpenSSL`

**Solution:**
1. Make sure you installed the **full OpenSSL** (not just runtime)
2. Check if `include\openssl` directory exists in OpenSSL installation
3. Set OPENSSL_ROOT_DIR manually:
   ```powershell
   cmake .. -DOPENSSL_ROOT_DIR="C:\Path\To\OpenSSL"
   ```

### Visual Studio Not Found

**Problem:** CMake can't find Visual Studio

**Solution:**
1. Install Visual Studio 2022 Build Tools
2. Or use MinGW (Option 3 above)
3. Or use WSL2 (Option 4 above)

### Make Not Found (MinGW)

**Problem:** `CMAKE_MAKE_PROGRAM is not set`

**Solution:**
```powershell
# Install make in MSYS2
# In MSYS2 terminal:
pacman -S make
```

---

## Quick Test (Once Built)

After successful build, test the tools:

```powershell
# Navigate to build/Release (or build/ for MinGW)
cd build\Release

# Test TLS fingerprint collection
.\fingerprint_tls.exe github.com:443
.\fingerprint_tls.exe google.com:443

# Test SSH fingerprint collection  
.\fingerprint_ssh.exe github.com

# Generate evaluation dataset
.\generate_eval_set.exe

# Create baseline
.\baseline_diff.exe create --baseline ..\..\data\baseline.json

# Visualize
.\visualize.exe summary --data-dir ..\..\data

# Check results
cat ..\..\data\tls_fingerprints.json
```

---

## Expected Output

After running `fingerprint_tls.exe github.com:443`, you should see:

```
Collecting TLS fingerprint from github.com:443...
✓ TLS fingerprint saved: AA:BB:CC:DD:EE:FF:...
  Host: github.com:443
  Timestamp: 2025-01-XX...
```

And a file created at: `data\tls_fingerprints.json`

---

**Need Help?** Check `BUILD.md` for more detailed build instructions.

