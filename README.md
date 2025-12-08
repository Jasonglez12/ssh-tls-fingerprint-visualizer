# SSH/TLS Fingerprint Visualizer

**Contributors:** Jason Gonzalez, Nick Zanaboni  
**Class:** CECS 478 - Security  

## Project Overview
This tool automates the collection of SSH host keys and TLS certificate fingerprints. It establishes a baseline, stores results, and provides a visualization layer to detect drift (potential security events or misconfigurations).

## Project Demo
[text](https://youtu.be/sD5U9OdxUi8)

## Quick Start (Runbook)
To run the full vertical slice (Test -> Collect -> Visualize) in Docker:

```bash
make up && make demo

