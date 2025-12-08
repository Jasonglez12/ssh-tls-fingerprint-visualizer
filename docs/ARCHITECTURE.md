# Architecture

## Components

1. **Collectors (`src/collectors.py`)**
   - **TLS:** Uses Python's `ssl` library to perform a handshake and retrieve the certificate (DER format) to compute a SHA-256 hash.
   - **SSH:** Wraps the system utility `ssh-keyscan` to retrieve public host keys, then computes the hash.

2. **Orchestrator (`src/main.py`)**
   - Iterates through a target list.
   - Validates inputs.
   - Calls collectors.
   - Logs activities to `system.log`.
   - Appends results to `fingerprints.csv`.

3. **Visualizer (`src/visualizer.py`)**
   - Reads the CSV history.
   - Displays the latest fingerprints in a table.
   - Analyzes history to detect if a fingerprint has changed (Drift Detection).