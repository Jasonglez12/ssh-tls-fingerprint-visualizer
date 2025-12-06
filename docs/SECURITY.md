# Security Invariants

This document describes the security properties and guarantees of the SSH/TLS Fingerprint Visualizer.

## Security Guarantees

### What This System Does

1. **Read-Only Network Operations**
   - Connects to hosts to retrieve certificates/keys
   - Does not modify remote systems
   - Does not send authentication credentials

2. **Local Data Storage**
   - All data stored locally in user-specified directory
   - No data transmitted to external services (unless webhook configured)
   - User controls all data retention

3. **Non-Privileged Execution**
   - Docker containers run as non-root user (UID 1000)
   - No special system permissions required
   - Can run in isolated environments

4. **Input Validation**
   - Hostname validation (format, length)
   - Port range validation (1-65535)
   - File path sanitization
   - JSON/CSV parsing error handling

### What This System Does NOT Do

1. **No Credential Storage**
   - Never stores passwords or keys
   - Only stores fingerprint hashes (SHA-256, JA3, JA3S)
   - Cannot reconstruct original certificates/keys from fingerprints

2. **No Network Modification**
   - Does not install certificates
   - Does not modify network configuration
   - Does not intercept or modify traffic (except read-only PCAP parsing)

3. **No Execution of Untrusted Code**
   - Does not download or execute scripts
   - Does not process arbitrary file formats
   - External tools (ssh-keyscan) only called with validated inputs

4. **No Persistent Network Connections**
   - All connections are short-lived (timeout: 10 seconds default)
   - No background services or daemons
   - No listening ports opened

## Security Hardening Measures

### 1. Least-Privilege Execution

- **Docker**: Runs as non-root user (`appuser`, UID 1000)
- **File Permissions**: Data directory writable only by user
- **Network**: No privileged ports required
- **System Calls**: No system-level modifications

### 2. Input Validation

**Hostname Validation:**
- Maximum length: 253 characters (RFC 1123)
- Valid characters only
- Rejects malformed hostnames

**Port Validation:**
- Range: 1-65535
- Integer type checking
- Rejects invalid formats

**File Path Sanitization:**
- Prevents directory traversal (`../`)
- Validates absolute vs. relative paths
- Prevents overwriting system files

### 3. Rate Limiting

- Default: No explicit rate limiting (user-controlled)
- Recommended: Add delays between collections for large-scale use
- Future: Configurable rate limits to prevent resource exhaustion

### 4. Error Handling

- Graceful degradation on network failures
- No sensitive information in error messages
- Detailed logging for debugging (user-controlled verbosity)

### 5. Data Protection

**At Rest:**
- Plaintext fingerprints only (not sensitive by themselves)
- No encryption (not storing secrets)
- User controls file permissions

**In Transit:**
- TLS connections use system certificate store
- No custom certificate validation bypass (except for collection)
- Webhook alerts use HTTPS when configured

## Security Considerations

### Fingerprint Data

**Fingerprints are NOT secrets:**
- SHA-256 fingerprints are publicly observable
- JA3/JA3S fingerprints are network-observable
- Fingerprints identify, but don't authenticate

**However:**
- Fingerprint changes can indicate security issues
- Baseline files should be protected from tampering
- Access control on data directory recommended

### Network Security

**TLS Collection:**
- By default, does not verify certificates (collects any cert)
- This is intentional for detecting certificate changes
- For production use, consider adding validation options

**SSH Collection:**
- Uses standard OpenSSH tools (`ssh-keyscan`)
- Trust on first use (TOFU) model
- Host key changes trigger warnings

### Container Security

**Docker Configuration:**
- Non-root user execution
- Read-only root filesystem (future enhancement)
- No privileged mode
- Network isolation

**Resource Limits:**
- No explicit limits set (Docker defaults)
- Can be configured via docker-compose.yml

## Threat Model

### Assumptions

1. **Trusted Execution Environment**
   - Host system is trusted
   - Docker daemon is trusted
   - Compiler/build tools are trusted

2. **Network Assumptions**
   - Network connectivity is available
   - DNS resolution works correctly
   - Target hosts are reachable

3. **Data Assumptions**
   - Data directory is on trusted storage
   - File system is not tampered with
   - User has appropriate file permissions

### Potential Threats

1. **Man-in-the-Middle (MITM)**
   - **Risk**: Intercepted connections could show attacker's certificate
   - **Mitigation**: Baseline comparison detects unexpected changes
   - **Detection**: Fingerprint mismatch alerts

2. **Data Tampering**
   - **Risk**: Baseline files modified to hide changes
   - **Mitigation**: File permissions, integrity checks (future)
   - **Detection**: Unexpected baseline file modifications

3. **Resource Exhaustion**
   - **Risk**: Rapid collection could impact network/targets
   - **Mitigation**: User-controlled collection rate
   - **Prevention**: Rate limiting recommended for automation

4. **Code Injection**
   - **Risk**: Malformed input causing crashes
   - **Mitigation**: Input validation, error handling
   - **Prevention**: No execution of user input as code

## Security Best Practices

### For Users

1. **Protect Baseline Files**
   - Store baselines in secure location
   - Use file permissions to prevent tampering
   - Consider checksums for integrity verification

2. **Monitor for Changes**
   - Set up automated baseline comparison
   - Use alert system for change notifications
   - Investigate unexpected changes promptly

3. **Rate Limiting**
   - Add delays between collections
   - Don't overload target hosts
   - Respect network policies

4. **Access Control**
   - Restrict read access to data directory
   - Use secure webhook URLs (if configured)
   - Rotate webhook tokens regularly

### For Developers

1. **Input Validation**
   - Always validate user input
   - Sanitize file paths
   - Check bounds and types

2. **Error Handling**
   - Never expose sensitive information
   - Log appropriately (not too verbose)
   - Fail securely

3. **Dependencies**
   - Keep dependencies up to date
   - Use trusted sources
   - Monitor for security advisories

## Reporting Security Issues

If you discover a security vulnerability:

1. Do not create a public issue
2. Contact maintainers privately
3. Provide detailed information
4. Allow time for fix before disclosure

## Compliance Notes

- **No PII Collection**: System only collects fingerprint hashes, not personal data
- **No Logging of Sensitive Data**: Logs contain hostnames and fingerprints only
- **User-Controlled Data**: All data retention is user-controlled
- **Export Capability**: Users can export and delete their data at any time

## Future Security Enhancements

1. Baseline file integrity verification (checksums)
2. Encrypted baseline storage (optional)
3. Certificate pinning options
4. Rate limiting built-in
5. Audit logging
6. Read-only root filesystem in containers

---

**Last Updated:** 2025-12-05  
**Security Contact:** See repository maintainers

