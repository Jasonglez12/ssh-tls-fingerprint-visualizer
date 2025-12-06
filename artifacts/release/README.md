# Release Artifacts

This directory contains verification artifacts demonstrating the system's functionality and security properties.

## Directory Structure

- `charts/` - Generated visualization charts and tables
- `logs/` - Sample application logs
- `pcaps/` - Sample PCAP files (or references)
- `README.md` - This file

## Artifacts Description

### Charts and Visualizations

Sample output from the visualization tool showing:
- Fingerprint distribution
- Timeline of changes
- Summary statistics

### Logs

Sample application logs demonstrating:
- Key processing steps
- Error handling
- Alert generation

### PCAP Files

Sample network captures used for:
- JA3/JA3S fingerprint extraction
- Testing PCAP parsing functionality

## Generating Artifacts

To regenerate these artifacts:

```bash
# Run the demo
make demo

# Artifacts will be copied to this directory automatically
# Or manually copy from data/ directory
```

## Verification

These artifacts demonstrate:

1. **Functional Correctness**: Tools work as designed
2. **Security Properties**: Proper handling of sensitive data
3. **Performance**: Reasonable processing times
4. **Reliability**: Consistent results across runs

---

**Note:** Some artifacts may be omitted from repository due to size constraints.  
Generated artifacts should be recreated using `make demo` on a fresh clone.

