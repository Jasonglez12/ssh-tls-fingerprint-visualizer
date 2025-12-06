# Runbook

## Quick Start

### Prerequisites

- Docker and Docker Compose installed
- Git (for cloning repository)
- Network connectivity (for fingerprint collection)

### Fresh Clone Setup

```bash
# 1. Clone repository
git clone <repository-url>
cd ssh-tls-fingerprint-visualizer

# 2. Build and run demo
make up && make demo
```

This should complete in ≤5 minutes on a fresh clone.

## Detailed Rebuild Instructions

### Option 1: Using Docker (Recommended)

```bash
# Build Docker image
docker compose build

# Start container
docker compose up -d

# Run demo
docker compose exec app /bin/bash /app/scripts/demo.sh

# Or use make
make up
make demo
```

### Option 2: Native Build

#### Linux/macOS

```bash
# Install dependencies
sudo apt-get update  # Ubuntu/Debian
sudo apt-get install -y build-essential cmake libssl-dev libpcap-dev

# Or macOS
brew install cmake openssl libpcap

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run
./fingerprint_tls github.com:443
```

#### Windows

See `QUICK_START.md` for Windows-specific instructions using MSYS2 or Visual Studio.

## Common Operations

### Collect Fingerprints

```bash
# TLS fingerprint
./fingerprint_tls example.com:443 --data-dir data

# SSH fingerprint
./fingerprint_ssh example.com --data-dir data

# From PCAP file
./fingerprint_pcap capture.pcap --data-dir data --stats
```

### Baseline Management

```bash
# Create baseline
./baseline_diff create --baseline data/baseline.json --data-dir data

# Compare against baseline
./baseline_diff diff --baseline data/baseline.json --data-dir data

# With alerts
./baseline_diff diff --baseline data/baseline.json --alert-webhook <URL>
```

### Visualization

```bash
# Generate summary
./visualize summary --data-dir data --output data/summary.json

# Generate timeline
./visualize timeline --data-dir data --output data/timeline.json

# Both
./visualize all --data-dir data
```

## Troubleshooting

### Docker Issues

**Problem:** `docker compose` not found
```bash
# Try alternative command
docker-compose up -d
```

**Problem:** Permission denied
```bash
# Add user to docker group (Linux)
sudo usermod -aG docker $USER
# Log out and back in
```

**Problem:** Container won't start
```bash
# Check logs
docker compose logs app

# Rebuild from scratch
docker compose down
docker compose build --no-cache
docker compose up -d
```

### Build Issues

**Problem:** OpenSSL not found
```bash
# Linux
sudo apt-get install libssl-dev

# macOS
brew install openssl
export PKG_CONFIG_PATH="/usr/local/opt/openssl/lib/pkgconfig"

# Windows - Use MSYS2 or install from slproweb.com
```

**Problem:** libpcap not found (for PCAP support)
```bash
# Linux
sudo apt-get install libpcap-dev

# macOS
brew install libpcap

# Note: PCAP is optional, project will build without it
```

### Runtime Issues

**Problem:** Network timeouts
```bash
# Increase timeout
./fingerprint_tls example.com:443 --timeout 30
```

**Problem:** SSH collection fails
```bash
# Verify ssh-keyscan is installed
which ssh-keyscan

# Install OpenSSH client
sudo apt-get install openssh-client  # Linux
```

**Problem:** No data files created
```bash
# Check data directory permissions
ls -la data/

# Create directory if missing
mkdir -p data
```

### Data Issues

**Problem:** Baseline comparison shows all changed
```bash
# Verify baseline format
cat data/baseline.json

# Recreate baseline
./baseline_diff create --baseline data/baseline.json --data-dir data
```

## Maintenance

### Clean Build

```bash
# Clean build directory
make clean

# Or manually
rm -rf build
```

### Update Dependencies

```bash
# Rebuild Docker image
docker compose build --no-cache
```

### Backup Data

```bash
# Backup data directory
tar -czf fingerprint-data-backup.tar.gz data/

# Restore
tar -xzf fingerprint-data-backup.tar.gz
```

## Performance Tuning

### Collection Rate

For large-scale collection, add delays between requests:

```bash
for host in host1 host2 host3; do
    ./fingerprint_tls $host:443
    sleep 1  # Rate limiting
done
```

### Storage Optimization

- Use CSV for large datasets (smaller file size)
- Use JSON for programmatic access
- Archive old fingerprints regularly

## Monitoring

### Check System Health

```bash
# Verify executables work
./fingerprint_tls github.com:443 --data-dir test_data

# Check data directory
ls -lh data/*.json

# View recent fingerprints
tail -f data/tls_fingerprints.json
```

### Log Locations

- Application logs: Check console output
- Docker logs: `docker compose logs app`
- System logs: Check system log location (varies by OS)

## Emergency Procedures

### System Not Responding

1. Check Docker container status: `docker compose ps`
2. Restart container: `docker compose restart app`
3. Check resource usage: `docker stats`

### Data Corruption

1. Stop collection immediately
2. Backup current data: `cp -r data data.backup`
3. Verify JSON validity: `cat data/*.json | python -m json.tool`
4. Restore from backup if needed

### Security Incident

If unexpected fingerprint changes detected:

1. Verify changes: `./baseline_diff diff --baseline data/baseline.json`
2. Check logs for errors
3. Re-verify fingerprints manually
4. Update baseline if legitimate change
5. Investigate if suspicious

## Support

For issues:
1. Check this runbook
2. Review `README.md` and `BUILD.md`
3. Check GitHub issues
4. Review logs for error messages

