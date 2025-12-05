# SSH/TLS Fingerprint Visualizer - Project Completion Itinerary

**Last Updated:** 2025-01-XX  
**Current Status:** Beta Features (Week 14) ✅ Complete  
**Next Milestone:** Final Features (Week 16) 🚧 In Progress

---

## 📊 Project Status Overview

### ✅ Completed (Alpha - Week 12)
- [x] TLS server certificate SHA-256 fingerprinting
- [x] SSH host key SHA-256 fingerprinting (wraps `ssh-keyscan`/`ssh-keygen`)
- [x] CSV/JSON outputs with timestamps
- [x] Basic storage and retrieval system
- [x] CMake build system
- [x] Cross-platform support (Windows/Linux/macOS)

### ✅ Completed (Beta - Week 14)
- [x] Baseline diff functionality (`baseline_diff` tool)
- [x] Simple charts/visualizations (`visualize` tool)
- [x] Evaluation dataset generator (`generate_eval_set` tool)
- [x] JSON data export for external visualization tools

### ✅ Completed (Final - Week 16)
- [x] **JA3/JA3S fingerprinting** from PCAP files
- [x] **PCAP parsing** support (with libpcap)
- [x] **Advanced drift alerts** (console, webhooks via libcurl)
- [x] **Enhanced visualizations** (JA3/JA3S support in charts)
- [x] Integration with existing tools (`fingerprint_pcap`, alert integration in `baseline_diff`)

---

## 🔍 Similar Projects Analysis & Useful Resources

### 1. **fp** (Go-based fingerprint tool)
**Repository:** https://github.com/gospider007/fp  
**Useful Files:**
- `fp.go` - Core fingerprint detection logic
- `handler.go` - HTTP/2 fingerprint handling
- JA3, JA4, JA4H fingerprint implementations
- HTTP/2 fingerprinting methods

**What We Can Learn:**
- Client fingerprint detection algorithms
- Multi-protocol fingerprint support
- Integration patterns for different fingerprint types

**Useful for:**
- Implementing JA3/JA3S calculation algorithms
- Understanding TLS handshake fingerprinting
- Reference implementation for fingerprint extraction

---

### 2. **finch** (TLS Reverse Proxy with Fingerprinting)
**Repository:** https://github.com/0x4D31/finch  
**Useful Files:**
- TLS handshake inspection code
- JA3, JA4, JA4H fingerprint extraction
- Real-time fingerprint collection methods
- Rule-based evaluation system

**What We Can Learn:**
- Real-time TLS handshake interception
- JA3/JA3S calculation from live connections
- Fingerprint-based rule evaluation
- Integration with reverse proxy patterns

**Useful for:**
- Implementing JA3/JA3S from live connections (alternative to PCAP)
- Real-time fingerprint collection
- Fingerprint-based alerting mechanisms

---

### 3. **friTap** (SSL/TLS Traffic Analysis)
**Repository:** https://github.com/fkie-cad/friTap  
**Useful Files:**
- PCAP parsing and TLS decryption
- SSL/TLS key extraction automation
- Traffic analysis tooling
- Multi-OS support code

**What We Can Learn:**
- PCAP file parsing techniques
- TLS handshake extraction from PCAPs
- Certificate and key extraction methods
- Cross-platform network analysis

**Useful for:**
- **PCAP parsing implementation** ⭐ (Critical for Week 16)
- TLS handshake reconstruction from packets
- Network traffic analysis patterns
- Decryption workflow

---

### 4. **hassh-rs** (SSH Fingerprinting)
**Repository:** https://github.com/flier/hassh-rs  
**Useful Files:**
- SSH handshake parsing (Rusticata ssh-parser)
- SSH Key Exchange fingerprinting
- IPv4/IPv6 support
- HASSH algorithm implementation

**What We Can Learn:**
- SSH handshake analysis
- SSH client fingerprinting (HASSH)
- Alternative SSH fingerprinting methods beyond host keys
- Protocol parsing techniques

**Useful for:**
- Enhancing SSH fingerprinting (HASSH support)
- Understanding SSH handshake structure
- Client-side SSH fingerprinting

---

### 5. **pqcscan** (Post-Quantum Cryptography Scanner)
**Repository:** https://github.com/anvilsecure/pqcscan  
**Useful Files:**
- SSH/TLS server scanning methods
- JSON report generation
- HTML visualization generation
- Algorithm detection logic

**What We Can Learn:**
- Server scanning methodologies
- Report generation best practices
- HTML visualization techniques
- Algorithm detection patterns

**Useful for:**
- Enhanced reporting features
- HTML visualization improvements
- Server capability detection

---

### 6. **huginn-net** (Multi-Protocol Passive Fingerprinting)
**Repository:** https://github.com/biandratti/huginn-net  
**Useful Files:**
- TLS (JA4-style) analysis
- TCP/HTTP fingerprinting
- Interactive web application
- Real-time visualization

**What We Can Learn:**
- Web-based visualization approaches
- Interactive dashboards
- Real-time fingerprint display
- Multi-protocol analysis patterns

**Useful for:**
- **Web-based visualization** ⭐ (Enhancement for Week 16)
- Interactive chart implementations
- Real-time monitoring dashboards
- Multi-protocol analysis UI

---

## 🎯 Detailed Completion Plan - Week 16 Features

### Phase 1: PCAP Parsing Infrastructure

#### Task 1.1: Integrate PCAP Library
- [ ] Add libpcap/libpcapng dependency to CMakeLists.txt
  - **Windows:** Npcap (WinPcap replacement)
  - **Linux:** libpcap-dev package
  - **macOS:** libpcap via Homebrew
- [ ] Create PCAP reader abstraction class
  - File: `include/pcap_reader.h`
  - Functions: Open PCAP, iterate packets, filter TLS packets
- [ ] Implement basic packet parsing
  - File: `src/pcap_reader.cpp`
  - Extract IP headers, TCP segments
  - Reassemble TCP streams

**Useful Code References:**
- friTap: PCAP reading and parsing logic
- Wireshark dissector patterns for TLS detection

**Estimated Time:** 2-3 days

---

#### Task 1.2: TLS Handshake Extraction from PCAP
- [ ] Implement TLS packet detection
  - Identify TLS ClientHello packets (TCP port 443 or SNI)
  - Identify TLS ServerHello packets
  - Match ClientHello/ServerHello pairs
- [ ] Extract TLS handshake data
  - ClientHello: Cipher suites, extensions, TLS version
  - ServerHello: Selected cipher suite, extensions, TLS version
- [ ] Create TLS handshake record structure
  - File: `include/tls_handshake.h`
  - Fields: ClientHello data, ServerHello data, timestamps

**Useful Code References:**
- finch: TLS handshake inspection code
- fp: JA3 calculation from ClientHello
- RFC 8446 (TLS 1.3) and RFC 5246 (TLS 1.2) for handshake structure

**Estimated Time:** 3-4 days

---

### Phase 2: JA3/JA3S Fingerprinting

#### Task 2.1: Implement JA3 Fingerprint Calculation
- [ ] Parse ClientHello message
  - Extract TLS version
  - Extract cipher suites list
  - Extract extensions list
  - Extract elliptic curves and point formats (if present)
- [ ] Implement JA3 algorithm
  - Formula: `TLSVersion,CipherSuites,Extensions,EllipticCurves,EllipticCurvePointFormats`
  - Hash with MD5 (JA3 standard)
  - File: `src/ja3.cpp`
- [ ] Integrate with fingerprint storage
  - Add JA3 to FingerprintRecord structure
  - Store alongside SHA-256 fingerprints

**Useful Code References:**
- fp: `fp.go` - JA3 calculation implementation
- finch: JA3 extraction from TLS handshakes
- Official JA3 specification: https://github.com/salesforce/ja3

**JA3 Formula:**
```
JA3 = md5(TLSVersion + "," + CipherSuites + "," + Extensions + "," + EllipticCurves + "," + EllipticCurvePointFormats)
```

**Estimated Time:** 2-3 days

---

#### Task 2.2: Implement JA3S Fingerprint Calculation
- [ ] Parse ServerHello message
  - Extract TLS version
  - Extract selected cipher suite
  - Extract extensions list
- [ ] Implement JA3S algorithm
  - Formula: `TLSVersion,CipherSuite,Extensions`
  - Hash with MD5 (JA3S standard)
  - File: `src/ja3s.cpp`
- [ ] Match JA3/JA3S pairs
  - Link ClientHello (JA3) with ServerHello (JA3S)
  - Store paired fingerprints
  - Create visualization for JA3/JA3S distributions

**Useful Code References:**
- finch: JA3S extraction implementation
- fp: Server-side fingerprint handling
- Official JA3S specification: https://github.com/salesforce/ja3s

**JA3S Formula:**
```
JA3S = md5(TLSVersion + "," + CipherSuite + "," + Extensions)
```

**Estimated Time:** 2-3 days

---

#### Task 2.3: Create PCAP Analysis Tool
- [ ] Create `fingerprint_pcap` executable
  - File: `src/fingerprint_pcap.cpp`
  - Command: `fingerprint_pcap <pcap_file> [options]`
  - Options: `--output-dir`, `--filter`, `--format`
- [ ] Parse PCAP and extract JA3/JA3S
  - Iterate through packets
  - Identify TLS handshakes
  - Calculate fingerprints
  - Store results in data directory
- [ ] Generate statistics
  - JA3 distribution (most common client fingerprints)
  - JA3S distribution (most common server fingerprints)
  - JA3/JA3S pair frequency
  - Export to JSON/CSV

**Estimated Time:** 3-4 days

---

### Phase 3: Advanced Visualizations

#### Task 3.1: Enhanced Timeline Visualizations
- [ ] Add JA3/JA3S to timeline charts
  - Separate lines for SHA-256, JA3, JA3S
  - Color-coded by fingerprint type
- [ ] Implement fingerprint distribution charts
  - Bar charts showing JA3 frequency
  - Pie charts for JA3S distribution
  - Heatmaps for JA3/JA3S pairs
- [ ] Create interactive JSON output
  - Structure data for D3.js/Chart.js
  - Include metadata for tooltips
  - Export to HTML template

**Useful Code References:**
- huginn-net: Web-based visualization approach
- pqcscan: HTML report generation
- D3.js examples for network visualization

**Estimated Time:** 2-3 days

---

#### Task 3.2: Web-Based Visualization (Optional Enhancement)
- [ ] Create simple HTML dashboard
  - File: `web/index.html`
  - Use Chart.js or D3.js for rendering
  - Load JSON data from `visualize` tool
- [ ] Implement real-time updates (if time permits)
  - WebSocket connection for live data
  - Auto-refresh charts

**Estimated Time:** 2-3 days (optional)

---

### Phase 4: Advanced Drift Alerts

#### Task 4.1: Alert System Infrastructure
- [ ] Create alert configuration file
  - File: `include/alert_config.h`
  - Support for multiple alert backends
- [ ] Implement alert manager
  - File: `src/alert_manager.cpp`
  - Queue alerts
  - Route to appropriate backends

**Estimated Time:** 1-2 days

---

#### Task 4.2: Email Alerts
- [ ] Integrate email sending library
  - SMTP client (libcurl or similar)
  - HTML email template for alerts
- [ ] Configure alert thresholds
  - Alert on fingerprint changes
  - Alert on new hosts
  - Configurable frequency limits (avoid spam)

**Useful Code References:**
- Standard SMTP libraries (libcurl, libmail)
- Email template examples

**Estimated Time:** 2-3 days

---

#### Task 4.3: Webhook Alerts
- [ ] Implement HTTP webhook support
  - POST JSON payload to webhook URL
  - Configurable headers and authentication
- [ ] Support common webhook formats
  - Slack webhook format
  - Discord webhook format
  - Generic JSON webhook

**Useful Code References:**
- Standard HTTP client libraries
- Webhook payload examples from monitoring tools

**Estimated Time:** 1-2 days

---

#### Task 4.4: Alert Integration with Baseline Diff
- [ ] Integrate alerts into `baseline_diff` tool
  - Trigger alerts when changes detected
  - Include diff details in alert payload
- [ ] Add alert configuration options
  - Command-line flags: `--alert-email`, `--alert-webhook`
  - Config file: `~/.fingerprint_visualizer/alerts.conf`

**Estimated Time:** 1 day

---

### Phase 5: Testing & Documentation

#### Task 5.1: Unit Tests
- [ ] Create test suite for JA3/JA3S calculation
  - Test cases from official JA3 repository
  - Known-good PCAP files with expected fingerprints
- [ ] Test PCAP parsing
  - Various PCAP formats (pcap, pcapng)
  - Edge cases (fragmented packets, retransmissions)
- [ ] Integration tests
  - End-to-end PCAP → fingerprint → storage → visualization

**Estimated Time:** 2-3 days

---

#### Task 5.2: Documentation Updates
- [ ] Update README.md
  - Add JA3/JA3S usage examples
  - Document PCAP analysis workflow
  - Add alert configuration guide
- [ ] Create PCAP_ANALYSIS.md
  - How to use `fingerprint_pcap` tool
  - PCAP file requirements
  - Interpretation of JA3/JA3S results
- [ ] Update API documentation
  - Document new functions and classes
  - Code examples

**Estimated Time:** 2 days

---

#### Task 5.3: Sample Data & Examples
- [ ] Create sample PCAP files
  - Test PCAPs with known JA3/JA3S values
  - Place in `tests/data/` directory
- [ ] Create example workflows
  - Example scripts for common use cases
  - Tutorial walkthrough

**Estimated Time:** 1-2 days

---

## 📋 Implementation Priority

### **High Priority (Must Have for Week 16)**
1. ✅ PCAP parsing infrastructure
2. ✅ JA3/JA3S calculation
3. ✅ PCAP analysis tool (`fingerprint_pcap`)
4. ✅ Enhanced visualizations for JA3/JA3S
5. ✅ Basic alerting (at least one method)

### **Medium Priority (Nice to Have)**
6. Multiple alert backends (email + webhook)
7. Web-based visualization dashboard
8. Comprehensive test suite

### **Low Priority (Future Enhancements)**
9. HASSH support for SSH client fingerprinting
10. Real-time fingerprinting (without PCAP)
11. Machine learning-based anomaly detection

---

## 🔧 Technical Dependencies to Add

### Required Libraries
```cmake
# PCAP support
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPCAP REQUIRED libpcap)

# Optional: For enhanced networking (webhooks, email)
find_package(CURL)
find_package(OpenSSL REQUIRED) # Already have this
```

### New Source Files to Create
```
src/
  ├── pcap_reader.cpp           # PCAP file parsing
  ├── ja3.cpp                   # JA3 fingerprint calculation
  ├── ja3s.cpp                  # JA3S fingerprint calculation
  ├── fingerprint_pcap.cpp      # PCAP analysis tool
  ├── alert_manager.cpp         # Alert system
  └── email_alert.cpp           # Email alerting (optional)

include/
  ├── pcap_reader.h
  ├── ja3.h
  ├── ja3s.h
  └── alert_manager.h
```

---

## 📚 Key Resources & References

### Official Specifications
- **JA3 Specification:** https://github.com/salesforce/ja3
- **JA3S Specification:** https://github.com/salesforce/ja3s
- **HASSH Specification:** https://github.com/salesforce/hassh
- **TLS 1.3 RFC 8446:** https://tools.ietf.org/html/rfc8446
- **TLS 1.2 RFC 5246:** https://tools.ietf.org/html/rfc5246

### Libraries & Tools
- **libpcap:** https://www.tcpdump.org/manpages/pcap.3pcap.html
- **Npcap (Windows):** https://nmap.org/npcap/
- **OpenSSL:** Already integrated, use for MD5 hashing
- **Chart.js:** https://www.chartjs.org/ (for web visualizations)
- **D3.js:** https://d3js.org/ (alternative visualization library)

### Sample PCAP Files
- Wireshark Sample Captures: https://wiki.wireshark.org/SampleCaptures
- JA3 Test PCAPs: Check official JA3 repository
- Create test PCAPs using tcpdump/wireshark

---

## 🎓 Learning Resources

### Understanding JA3/JA3S
1. Read official JA3 documentation and examples
2. Analyze finch and fp source code for implementation patterns
3. Test with known-good PCAP files

### PCAP Parsing
1. Study libpcap documentation and examples
2. Review friTap PCAP parsing code
3. Practice with sample PCAP files in Wireshark

### TLS Handshake Structure
1. Review TLS RFCs (8446 for 1.3, 5246 for 1.2)
2. Use Wireshark to inspect TLS handshakes
3. Understand ClientHello and ServerHello message formats

---

## ✅ Completion Checklist

### Week 16 Deliverables
- [ ] `fingerprint_pcap` tool implemented and tested
- [ ] JA3/JA3S fingerprinting working from PCAP files
- [ ] Enhanced visualizations showing JA3/JA3S distributions
- [ ] At least one alerting method (email or webhook)
- [ ] Updated documentation with new features
- [ ] Sample PCAP files and test cases
- [ ] Integration tests passing

### Quality Gates
- [ ] All new code compiles on Windows, Linux, macOS
- [ ] No memory leaks (use valgrind/sanitizers)
- [ ] Error handling for malformed PCAPs
- [ ] Performance: Can process 100MB PCAP in < 30 seconds
- [ ] Documentation complete and accurate

---

## 🚀 Quick Start for Implementation

### Step 1: Set Up PCAP Library
```bash
# Linux
sudo apt-get install libpcap-dev

# macOS
brew install libpcap

# Windows
# Install Npcap from https://nmap.org/npcap/
```

### Step 2: Clone Reference Repositories
```bash
git clone https://github.com/salesforce/ja3.git
git clone https://github.com/0x4D31/finch.git
git clone https://github.com/fkie-cad/friTap.git
# Study their implementations
```

### Step 3: Start with JA3 Calculation
- Implement standalone JA3 calculator first
- Test with known ClientHello data
- Integrate into PCAP parsing once working

### Step 4: Build PCAP Parser Incrementally
- Start with basic packet reading
- Add TLS packet detection
- Add handshake extraction
- Add fingerprint calculation

---

## 📝 Notes

- **Keep existing functionality working:** Don't break Alpha/Beta features
- **Incremental development:** Build and test each component separately
- **Use test-driven development:** Create test cases before implementing features
- **Document as you go:** Don't leave documentation until the end
- **Ask for help:** Reference implementations are available, don't reinvent the wheel

---

**Created:** 2025-01-XX  
**Maintainers:** Jason Gonzalez & Nick Zanaboni  
**Status:** Active Development - Week 16

