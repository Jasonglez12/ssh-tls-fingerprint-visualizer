# Docker Guide for Windows PowerShell

This guide provides Windows PowerShell-specific instructions for using Docker with the SSH/TLS Fingerprint Visualizer.

---

## Problem: PowerShell Doesn't Support `&&`

PowerShell doesn't support the `&&` operator like Bash does. If you see this error:

```
The token '&&' is not a valid statement separator in this version.
```

Use one of the solutions below instead.

---

## Solution 1: Use Docker Compose Directly (Recommended)

**If `make` is not installed**, use Docker Compose commands directly:

```powershell
# Step 1: Build the Docker image
docker compose build

# Step 2: Start container in background
docker compose up -d

# Step 3: Wait a moment for container to start
Start-Sleep -Seconds 3

# Step 4: Run the demo
docker compose exec -T app /bin/bash /app/scripts/demo.sh

# Step 5: Stop container when done
docker compose down
```

**Or if you prefer one-liner:**
```powershell
docker compose build; docker compose up -d; Start-Sleep -Seconds 3; docker compose exec -T app /bin/bash /app/scripts/demo.sh
```

---

## Solution 2: Install Make (Alternative)

If you want to use `make up && make demo`, install make:

**Option A: Using Chocolatey**
```powershell
choco install make
```

**Option B: Using MSYS2** (if you have it installed)
```powershell
C:\msys64\usr\bin\make.exe up
```

**Option C: Use Git Bash** (comes with Git for Windows)
```bash
# In Git Bash terminal
make up && make demo
```

---

## Solution 3: Use Git Bash (If Installed)

If you have Git Bash installed (comes with Git for Windows), you can use Bash syntax:

```bash
# In Git Bash
make up && make demo
```

---

## Complete PowerShell Workflow (No Make Required)

Here's the complete workflow for Windows PowerShell **without needing `make`**:

```powershell
# 1. Navigate to project directory
cd C:\Users\nickz\OneDrive\Documents\GitHub\ssh-tls-fingerprint-visualizer

# 2. Build Docker image
docker compose build

# 3. Start container in background
docker compose up -d

# 4. Wait a moment for container to start
Start-Sleep -Seconds 3

# 5. Verify container is running
docker ps

# 6. Run the demo
docker compose exec -T app /bin/bash /app/scripts/demo.sh

# 7. Check results
Get-ChildItem -Path .\data\ -ErrorAction SilentlyContinue
Get-ChildItem -Path .\artifacts\release\ -ErrorAction SilentlyContinue

# 8. Stop container when done
docker compose down
```

**Quick one-liner version:**
```powershell
docker compose build; docker compose up -d; Start-Sleep -Seconds 3; docker compose exec -T app /bin/bash /app/scripts/demo.sh
```

---

## Interactive Usage

To enter the container interactively:

```powershell
# Enter container
docker compose exec app /bin/bash

# Run tools inside container
/app/fingerprint_tls github.com:443 --data-dir /app/data
/app/visualize summary --data-dir /app/data

# Exit container
exit
```

---

## Troubleshooting

### Docker Desktop Not Running

**Error:** `The system cannot find the file specified` or `error during connect`

This means Docker Desktop is not running. To fix:

1. **Start Docker Desktop:**
   - Press `Win` key and search for "Docker Desktop"
   - Click to launch Docker Desktop
   - Wait for Docker to fully start (icon in system tray will show "Docker Desktop is running")

2. **Verify Docker is running:**
   ```powershell
   docker ps
   ```
   - If Docker is running, this will show running containers (or empty list)
   - If not running, you'll see the same error

3. **Check Docker Desktop status:**
   - Look for Docker icon in Windows system tray (bottom-right)
   - If it's not there, Docker Desktop isn't running
   - Right-click the icon to see options like "Start Docker Desktop"

**Alternative:** If Docker Desktop isn't installed, download from:
- https://www.docker.com/products/docker-desktop/

### Make Command Not Found

**This is normal!** PowerShell doesn't include `make` by default. You have two options:

1. **Use Docker Compose directly** (Recommended - no installation needed)
   ```powershell
   docker compose build
   docker compose up -d
   docker compose exec -T app /bin/bash /app/scripts/demo.sh
   ```

2. **Install make** (Optional):
   - **Chocolatey**: `choco install make`
   - **MSYS2/MinGW**: If already installed, use `C:\msys64\usr\bin\make.exe`
   - **Git Bash**: Use Git Bash terminal instead of PowerShell
   - **WSL2**: Use `wsl` and run commands in Linux environment

### Docker Compose Not Found

```powershell
# Try with hyphen (older versions)
docker-compose build
docker-compose up -d
```

### Permission Errors

```powershell
# Ensure directories are writable
New-Item -ItemType Directory -Force -Path .\data
New-Item -ItemType Directory -Force -Path .\artifacts
```

### Docker Compose Version Warning

**Warning:** `the attribute 'version' is obsolete`

This is harmless but can be fixed by removing the `version` line from `docker-compose.yml`. This has been fixed in the latest version of the file.

### Container Won't Start After Docker Starts

If Docker Desktop just started, wait a few seconds:

```powershell
# Wait for Docker to be ready
Start-Sleep -Seconds 5

# Check Docker is running
docker ps

# Now try building
docker compose build
```

---

## Quick Reference

| Bash/Linux Command | PowerShell Equivalent (No Make) |
|-------------------|--------------------------------|
| `make up && make demo` | `docker compose build; docker compose up -d; Start-Sleep -Seconds 3; docker compose exec -T app /bin/bash /app/scripts/demo.sh` |
| `make up` | `docker compose build; docker compose up -d` |
| `make demo` | `docker compose exec -T app /bin/bash /app/scripts/demo.sh` |
| `make down` | `docker compose down` |

**Or run commands separately:**
```powershell
docker compose build
docker compose up -d
docker compose exec -T app /bin/bash /app/scripts/demo.sh
docker compose down
```

---

## Verify Docker is Running

```powershell
# Check Docker version
docker --version

# Check Docker Compose version
docker compose version

# List running containers
docker ps

# View container logs
docker compose logs app
```

---

**See also:** [DOCKER_GUIDE.md](DOCKER_GUIDE.md) for general Docker instructions (Bash/Linux format)
