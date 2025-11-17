# SSH/TLS Fingerprint Visualizer — Project Board

## Columns
- Backlog
- In Progress
- Review
- Done

## Milestones
- **Week 10:** Proposal (PDF + repo skeleton + bootstrap)
- **Week 12:** Alpha (TLS/SSH SHA-256 collection + CSV/JSON export)
- **Week 14:** Beta (baseline diffs + simple charts + small eval set)
- **Week 16:** Final (metrics, polish; pair: JA3/JA3S analysis)

## Core Cards (add these to Backlog)
- TLS: certificate SHA-256 collector (CLI)
- SSH: host-key SHA-256 collector (CLI)
- Storage: CSV/JSON with timestamps + baseline file
- Diff view: detect & flag key/cert changes
- Visuals: small charts/tables (keys changed per host/time)
- Eval: correctness vs. `openssl` / `ssh-keygen`
- Ethics: anonymization + consent notes in README
- Packaging: README, diagram, usage examples
- (Pair) PCAP parser: JA3/JA3S fingerprints
- (Pair) Drift alerts + optional CT-log/JA4 comparison
