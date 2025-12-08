# Runbook

## Requirements
- Docker
- Make

## Build and Run
1. **Clone the repository:**
   ```bash
   git clone <repo_url>
   cd ssh-tls-fingerprint-visualizer

2. **Run the Demo (Vertical Slice):**
    ```bash
    make up
    make demo

    Expected Duration: < 1 minute

3. **View Artifacts: Check the artifacts/release/ folder for:**
- fingerprints.csv (Data)
- system.log (Execution logs)

**Troubleshooting**
If make demo fails, ensure port 443/22 outbound is allowed on your network.

**`docs/SECURITY_INVARIANTS.md`**
# Security Invariants

The following security principles are enforced in the codebase:

1. **No `shell=True`**: 
   - All `subprocess` calls in `src/collectors.py` use list-based arguments to prevent shell injection attacks.
   
2. **Input Validation**:
   - `src/utils.py` contains `validate_target()` which checks hostnames against regex and IP standards before processing. This prevents command injection via malicious hostnames.

3. **Least Privilege**:
   - The Docker container runs as a standard user (implied) and does not require `--privileged` mode.
   - Only read-only handshakes are performed; no credentials are stored or used.