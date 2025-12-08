# Reproducible Setup - Quick Reference

This document provides the minimal steps to run the project reproducibly using Docker.

---

## ✅ One-Command Setup (Fresh Clone)

```bash
git clone <repository-url> && cd ssh-tls-fingerprint-visualizer && make up && make demo
```

**That's it!** This single command:
1. Clones the repository
2. Builds the Docker image with all dependencies
3. Starts the container
4. Runs the complete end-to-end demo

---

## Step-by-Step Breakdown

### 1. Clone Repository
```bash
git clone <repository-url>
cd ssh-tls-fingerprint-visualizer
```

### 2. Build & Start Container
```bash
make up
```

**What happens:**
- Docker builds image from `Dockerfile`
- Installs all dependencies (OpenSSL, libpcap, etc.)
- Compiles all executables
- Starts container in background

### 3. Run Demo
```bash
make demo
```

**What happens:**
- Collects fingerprints from multiple hosts
- Creates baseline
- Compares fingerprints
- Generates visualizations
- Exports metrics to `artifacts/release/`

### 4. Stop Container (when done)
```bash
make down
```

---

## Verify It Works

After `make demo`, check:

```bash
# Data files created
ls -lh data/*.json

# Artifacts generated
ls -lh artifacts/release/

# Container running
docker ps
```

---

## Interactive Use

After `make up`, enter container:

```bash
docker compose exec app /bin/bash
```

Then run tools:
```bash
/app/fingerprint_tls github.com:443 --data-dir /app/data
/app/visualize summary --data-dir /app/data
```

Exit: `exit`

---

## Reproducibility Guarantees

✅ **No host dependencies** - Only Docker required  
✅ **Isolated environment** - All dependencies in container  
✅ **Consistent builds** - Same base image every time  
✅ **Fresh builds** - Optional: `docker compose build --no-cache`  
✅ **Data persistence** - Data stored in `./data/` and `./artifacts/`

---

## Troubleshooting

**Container won't start?**
```bash
docker compose down
docker compose build --no-cache
docker compose up -d
```

**Permission errors?**
```bash
chmod 755 data artifacts
```

**Need fresh build?**
```bash
docker compose build --no-cache
make up
```

---

## Full Documentation

- **Detailed Docker Guide**: See [DOCKER_GUIDE.md](DOCKER_GUIDE.md)
- **Build Instructions**: See [BUILD.md](BUILD.md)
- **Run Instructions**: See [RUN_INSTRUCTIONS.md](RUN_INSTRUCTIONS.md)
- **Architecture**: See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)

---

**For submission:** The command `make up && make demo` works on any fresh clone. ✅

