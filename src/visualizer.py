import pandas as pd
import os
from tabulate import tabulate

INPUT_FILE = "artifacts/release/fingerprints.csv"

def run_viz():
    if not os.path.exists(INPUT_FILE):
        print("No data found to visualize.")
        return

    try:
        df = pd.read_csv(INPUT_FILE)
    except pd.errors.EmptyDataError:
        print("Data file is empty.")
        return
    
    print("\n=== EVALUATION REPORT ===")
    print(f"Total Records: {len(df)}")
    print(f"Distinct Hosts: {df['host'].nunique()}")
    
    print("\n--- Latest Fingerprints ---")
    latest = df.sort_values('timestamp').groupby('host').tail(1)
    print(tabulate(latest, headers='keys', tablefmt='grid', showindex=False))

    # Basic Drift Check (Evaluation)
    print("\n--- Integrity/Drift Check ---")
    for host in df['host'].unique():
        host_data = df[df['host'] == host]
        unique_tls = host_data['tls_fingerprint'].nunique()
        
        # If we have more than 1 unique fingerprint for a host, drift occurred
        status = "STABLE" if unique_tls == 1 else "DRIFT DETECTED"
        print(f"Host: {host:<15} | TLS Status: {status}")

if __name__ == "__main__":
    run_viz()