# Similar Projects Reference - Quick Guide

This document provides a quick reference to similar projects and specific files that can be useful for implementing Week 16 features.

---

## 🔗 Repository Links

1. **fp** - Go-based fingerprint tool
   - URL: https://github.com/gospider007/fp
   - License: Check repository
   - Language: Go

2. **finch** - TLS reverse proxy with fingerprinting
   - URL: https://github.com/0x4D31/finch
   - License: Check repository
   - Language: Go

3. **friTap** - SSL/TLS traffic analysis tool
   - URL: https://github.com/fkie-cad/friTap
   - License: Check repository
   - Language: Python/C

4. **hassh-rs** - SSH fingerprinting library
   - URL: https://github.com/flier/hassh-rs
   - License: Check repository
   - Language: Rust

5. **pqcscan** - Post-Quantum Cryptography scanner
   - URL: https://github.com/anvilsecure/pqcscan
   - License: Check repository
   - Language: Rust

6. **huginn-net** - Multi-protocol passive fingerprinting
   - URL: https://github.com/biandratti/huginn-net
   - License: Check repository
   - Language: Rust

---

## 📁 Useful Files by Feature

### JA3/JA3S Fingerprinting

#### From `fp` repository:
- **File:** `fp.go`
  - **What:** Core fingerprint detection including JA3, JA4, JA4H
  - **Use for:** Understanding JA3 calculation algorithm
  - **Key functions:** Look for JA3 hash calculation

#### From `finch` repository:
- **File:** TLS handshake inspection code
  - **What:** Real-time JA3/JA3S extraction from TLS connections
  - **Use for:** Learning TLS handshake parsing
  - **Key functions:** ClientHello/ServerHello extraction

#### Official JA3 Repository:
- **URL:** https://github.com/salesforce/ja3
- **Files:**
  - `ja3.py` - Reference implementation
  - Test cases with known-good values
  - Documentation and examples

---

### PCAP Parsing

#### From `friTap` repository:
- **Files to check:**
  - PCAP reading/parsing modules
  - TLS packet extraction code
  - Traffic analysis components
- **What:** PCAP file parsing and TLS decryption
- **Use for:** Learning PCAP parsing patterns, TLS handshake extraction from packets

#### libpcap Examples:
- **URL:** https://www.tcpdump.org/manpages/pcap.3pcap.html
- **Use for:** Official libpcap API documentation and examples

---

### TLS Handshake Structure

#### From `finch` repository:
- **Files:** TLS inspection code
- **What:** Real-time TLS handshake analysis
- **Use for:** Understanding ClientHello/ServerHello structure, cipher suite parsing

#### RFC References:
- **TLS 1.3:** RFC 8446 - https://tools.ietf.org/html/rfc8446
- **TLS 1.2:** RFC 5246 - https://tools.ietf.org/html/rfc5246
- **Use for:** Official TLS protocol specification

---

### Visualization

#### From `huginn-net` repository:
- **Files:** Web UI components, visualization code
- **What:** Interactive web-based fingerprint visualization
- **Use for:** Learning web dashboard patterns, real-time chart updates

#### From `pqcscan` repository:
- **Files:** HTML report generation code
- **What:** HTML/JSON report generation
- **Use for:** Learning report formatting and structure

---

### SSH Fingerprinting (Future Enhancement)

#### From `hassh-rs` repository:
- **Files:** SSH handshake parsing code
- **What:** HASSH algorithm implementation
- **Use for:** Client-side SSH fingerprinting (beyond host keys)

---

## 🎯 Quick Implementation Guide

### For JA3 Implementation:
1. Study `fp` repository's `fp.go` for JA3 calculation
2. Check official JA3 repository for test cases
3. Understand the formula: `md5(TLSVersion + "," + CipherSuites + "," + Extensions + "," + EllipticCurves + "," + EllipticCurvePointFormats)`

### For PCAP Parsing:
1. Review `friTap` PCAP reading code
2. Study libpcap documentation
3. Start with basic packet iteration, then add TLS filtering

### For TLS Handshake Extraction:
1. Read TLS RFCs (8446, 5246)
2. Study `finch` handshake inspection code
3. Use Wireshark to examine real TLS handshakes

---

## 🔍 Key Code Patterns to Look For

### JA3 Calculation Pattern:
```python
# Pseudo-code pattern (from official JA3)
def calculate_ja3(client_hello):
    tls_version = client_hello.version
    cipher_suites = ",".join(client_hello.cipher_suites)
    extensions = ",".join(client_hello.extensions)
    elliptic_curves = ",".join(client_hello.elliptic_curves)
    ec_point_formats = ",".join(client_hello.ec_point_formats)
    
    ja3_string = f"{tls_version},{cipher_suites},{extensions},{elliptic_curves},{ec_point_formats}"
    return md5(ja3_string)
```

### PCAP Reading Pattern:
```c
// Pseudo-code pattern (from libpcap)
pcap_t *handle = pcap_open_offline("file.pcap", errbuf);
struct pcap_pkthdr header;
const u_char *packet;

while ((packet = pcap_next(handle, &header)) != NULL) {
    // Process packet
    // Check if TCP packet on port 443
    // Extract TLS handshake
}
```

---

## 📚 Additional Learning Resources

### Understanding TLS:
- Wireshark TLS tutorial: https://www.wireshark.org/docs/wsug_html_chunked/ChapterIntroduction.html
- TLS handshake visualization: https://tls.ulfheim.net/

### PCAP Analysis:
- tcpdump examples: https://www.tcpdump.org/manpages/tcpdump.1.html
- Wireshark filters: https://www.wireshark.org/docs/man-pages/wireshark-filter.html

### Fingerprinting:
- JA3 blog post: https://engineering.salesforce.com/tls-fingerprinting-with-ja3-and-ja3s-247362855967
- HASSH documentation: https://github.com/salesforce/hassh

---

## ⚠️ Important Notes

1. **License Compliance:** Always check licenses before copying code
2. **Adaptation:** These are references - adapt to C++ and your project structure
3. **Testing:** Validate implementations with known-good test cases
4. **Standards:** Follow official specifications (JA3, TLS RFCs) when available

---

**Last Updated:** 2025-01-XX  
**Purpose:** Quick reference for implementing Week 16 features

