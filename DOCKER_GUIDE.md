# Docker Guide - Reproducible Setup

This guide explains how to use Docker to run the SSH/TLS Fingerprint Visualizer in a reproducible way.

---

## Prerequisites

1. **Docker Desktop** (Windows/Mac) or **Docker Engine + Docker Compose** (Linux)
   - Download: https://www.docker.com/products/docker-desktop/
   - Verify installation: `docker --version` and `docker compose version`

2. **Git** (to clone the repository)

---

## Quick Start (Reproducible Setup)

### Step 1: Clone the Repository

```bash
git clone <repository-url>
cd ssh-tls-fingerprint-visualizer
```

### Step 2: Build and Start Container

```bash
# Build the Docker image and start the container
make up
```

**What this does:**
- Builds the Docker image from `Dockerfile` (includes all dependencies)
- Compiles all executables inside the container
- Starts the container in the background
- Mounts `./data` and `./artifacts` directories for persistent storage

### Step 3: Run the Demo

```bash
# Run the complete end-to-end demonstration
make demo
```

**What this does:**
- Executes `scripts/demo.sh` inside the container
- Collects fingerprints from multiple hosts
- Creates a baseline
- Compares fingerprints
- Generates visualizations
- Exports metrics (JSON/CSV)

---

## Alternative: Manual Docker Commands

If you prefer not to use `make`, here are the equivalent commands:

### Build the Image

```bash
docker compose build
```

### Start the Container

```bash
docker compose up -d
```

### Run the Demo

```bash
docker compose exec app /bin/bash /app/scripts/demo.sh
```

### Stop the Container

```bash
docker compose down
```

---

## Interactive Usage

After running `make up`, you can use the container interactively:

### Enter the Container

```bash
docker compose exec app /bin/bash
```

### Run Individual Tools

Once inside the container:

```bash
# Collect TLS fingerprint
/app/fingerprint_tls github.com:443 --data-dir /app/data

# Collect SSH fingerprint
/app/fingerprint_ssh github.com --data-dir /app/data

# Create baseline
/app/baseline_diff create --baseline /app/data/baseline.json --data-dir /app/data

# Compare against baseline
/app/baseline_diff diff --baseline /app/data/baseline.json --data-dir /app/data

# Generate visualization
/app/visualize summary --data-dir /app/data --output /app/data/summary.json

# Generate evaluation dataset
/app/generate_eval_set --data-dir /app/data
```

### Exit the Container

```bash
exit
```

---

## File Persistence

The Docker setup uses **volume mounts** to persist data:

- **Host `./data/`** → **Container `/app/data/`**
  - All fingerprint data, baselines, and outputs are stored here
  - Data persists even after stopping/removing the container

- **Host `./artifacts/`** → **Container `/app/artifacts/`**
  - Sample outputs, charts, logs, and verification artifacts
  - Used for evidence and demonstration

**Important:** These directories are created automatically if they don't exist.

---

## Reproducibility Guarantees

This Docker setup ensures reproducibility by:

1. **Isolated Environment**: All dependencies are contained in the Docker image
2. **Consistent Build**: Same base image (Ubuntu 22.04) every time
3. **Versioned Dependencies**: Exact package versions from Ubuntu repositories
4. **No Host Dependencies**: Nothing required on your system except Docker
5. **Fresh Build**: Each `make up` rebuilds from scratch (or uses cached layers)

### For Maximum Reproducibility

To ensure a completely fresh build (no cached layers):

```bash
docker compose build --no-cache
docker compose up -d
```

---

## Troubleshooting

### Container Won't Start

```bash
# Check if Docker is running
docker ps

# View container logs
docker compose logs app

# Remove old containers and rebuild
docker compose down
docker compose build --no-cache
docker compose up -d
```

### Permission Errors

The container runs as non-root user (`appuser`). If you see permission errors:

```bash
# Ensure data directories are writable (on host)
chmod 755 data artifacts

# Or check container permissions
docker compose exec app ls -la /app/data
```

### Port Conflicts

The current setup doesn't expose ports. If you need network access:

Add to `docker-compose.yml`:
```yaml
ports:
  - "8080:8080"  # Example port mapping
```

### Out of Disk Space

```bash
# Clean up old Docker images/containers
docker system prune -a

# Remove specific image
docker rmi fingerprint-visualizer-app
```

---

## Complete Workflow Example

Here's a complete example from scratch:

```bash
# 1. Clone repository
git clone <repository-url>
cd ssh-tls-fingerprint-visualizer

# 2. Build and start
make up

# 3. Run full demo
make demo

# 4. Check results (on host)
ls -lh data/
ls -lh artifacts/release/

# 5. Interactive use (optional)
docker compose exec app /bin/bash
# ... run commands inside container ...
exit

# 6. Stop when done
make down
```

---

## Verification Checklist

After running `make up && make demo`, verify:

- [ ] Container is running: `docker ps` shows `fingerprint-visualizer`
- [ ] Data files created: `ls data/*.json` shows fingerprint files
- [ ] Artifacts generated: `ls artifacts/release/` shows output files
- [ ] No errors in logs: `docker compose logs app` shows successful execution
- [ ] Tools are accessible: `docker compose exec app /app/fingerprint_tls --help`

---

## Docker vs Native Build

**Use Docker (recommended) if:**
- You want reproducible environment
- You don't want to install dependencies locally
- You need to ensure consistency across machines
- You're demonstrating or submitting the project

**Use Native Build if:**
- You're actively developing and need fast iteration
- You already have all dependencies installed
- You prefer working directly with executables

---

## Additional Resources

- **Architecture**: See `docs/ARCHITECTURE.md`
- **Runbook**: See `docs/RUNBOOK.md` for operational details
- **Troubleshooting**: See `RUN_INSTRUCTIONS.md` for more help

---

**For submission:** The command `make up && make demo` must work on a fresh clone without any modifications. This guide ensures that!

