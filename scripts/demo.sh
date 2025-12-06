#!/bin/bash
# Demo script for end-to-end vertical slice
# Shows: collect → baseline → compare → alert → visualize

set -e

DATA_DIR="${DATA_DIR:-/app/data}"
ARTIFACTS_DIR="${ARTIFACTS_DIR:-/app/artifacts}"

echo "=========================================="
echo "SSH/TLS Fingerprint Visualizer - Demo"
echo "=========================================="
echo ""

# Create directories
mkdir -p "$DATA_DIR" "$ARTIFACTS_DIR"

echo "Step 1: Collecting TLS fingerprints..."
echo "----------------------------------------"
/app/fingerprint_tls github.com:443 --data-dir "$DATA_DIR" || echo "Warning: Failed to collect from github.com"
sleep 1
/app/fingerprint_tls google.com:443 --data-dir "$DATA_DIR" || echo "Warning: Failed to collect from google.com"
sleep 1

echo ""
echo "Step 2: Creating baseline..."
echo "----------------------------------------"
/app/baseline_diff create --baseline "$DATA_DIR/baseline.json" --data-dir "$DATA_DIR"

echo ""
echo "Step 3: Collecting additional fingerprints for comparison..."
echo "----------------------------------------"
/app/fingerprint_tls github.com:443 --data-dir "$DATA_DIR" || true
sleep 1

echo ""
echo "Step 4: Comparing against baseline..."
echo "----------------------------------------"
/app/baseline_diff diff --baseline "$DATA_DIR/baseline.json" --data-dir "$DATA_DIR" || true

echo ""
echo "Step 5: Generating visualizations..."
echo "----------------------------------------"
/app/visualize summary --data-dir "$DATA_DIR" --output "$DATA_DIR/summary.json"
/app/visualize timeline --data-dir "$DATA_DIR" --output "$DATA_DIR/timeline.json"

echo ""
echo "Step 6: Generating evaluation dataset..."
echo "----------------------------------------"
/app/generate_eval_set --data-dir "$DATA_DIR" || echo "Warning: Some hosts may have failed"

echo ""
echo "Step 7: Exporting metrics..."
echo "----------------------------------------"
if [ -f "$DATA_DIR/tls_fingerprints.json" ]; then
    echo "Metrics exported to: $DATA_DIR/tls_fingerprints.json"
    echo "Summary exported to: $DATA_DIR/summary.json"
    echo "Timeline exported to: $DATA_DIR/timeline.json"
    
    # Copy to artifacts
    cp "$DATA_DIR"/*.json "$ARTIFACTS_DIR/" 2>/dev/null || true
    cp "$DATA_DIR"/*.csv "$ARTIFACTS_DIR/" 2>/dev/null || true
    echo "Artifacts copied to: $ARTIFACTS_DIR/"
fi

echo ""
echo "=========================================="
echo "Demo Complete!"
echo "=========================================="
echo ""
echo "Data files: $DATA_DIR/"
echo "Artifacts: $ARTIFACTS_DIR/"
echo ""
ls -lh "$DATA_DIR"/*.json 2>/dev/null || echo "No JSON files found"

