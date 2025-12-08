import csv
import logging
import os
import datetime
from src.collectors import get_tls_fingerprint, get_ssh_fingerprint
from src.utils import validate_target

# Setup Logging (Observability)
# Logs to both console and file
log_file = "artifacts/release/system.log"
os.makedirs(os.path.dirname(log_file), exist_ok=True)

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler(log_file)
    ]
)

# Targets for the demo
TARGETS = ["github.com", "google.com", "example.com"]
OUTPUT_FILE = "artifacts/release/fingerprints.csv"

def main():
    logging.info("Starting Fingerprint Collection Cycle")
    
    results = []
    
    for host in TARGETS:
        if not validate_target(host):
            logging.warning(f"Skipping invalid target: {host}")
            continue

        logging.info(f"Processing target: {host}")
        
        # 1. TLS
        tls_fp = get_tls_fingerprint(host)
        
        # 2. SSH (Using github.com as the main example for SSH as others might block port 22)
        ssh_fp = "N/A"
        if host == "github.com":
            ssh_fp = get_ssh_fingerprint(host)
            
        timestamp = datetime.datetime.now().isoformat()
        
        results.append({
            "timestamp": timestamp,
            "host": host,
            "tls_fingerprint": tls_fp,
            "ssh_fingerprint": ssh_fp
        })

    # 3. Write to CSV
    keys = ["timestamp", "host", "tls_fingerprint", "ssh_fingerprint"]
    file_exists = os.path.isfile(OUTPUT_FILE)
    
    with open(OUTPUT_FILE, 'a', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=keys)
        if not file_exists:
            writer.writeheader()
        writer.writerows(results)
        
    logging.info(f"Successfully wrote {len(results)} records to {OUTPUT_FILE}")

if __name__ == "__main__":
    main()