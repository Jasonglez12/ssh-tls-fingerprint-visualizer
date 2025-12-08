import ssl
import socket
import hashlib
import subprocess
import base64

def get_tls_fingerprint(host, port=443):
    """Retrieves TLS certificate and returns SHA-256 fingerprint."""
    try:
        context = ssl.create_default_context()
        context.check_hostname = False
        context.verify_mode = ssl.CERT_NONE
        
        with socket.create_connection((host, port), timeout=5) as sock:
            with context.wrap_socket(sock, server_hostname=host) as ssock:
                cert_bin = ssock.getpeercert(binary_form=True)
                return hashlib.sha256(cert_bin).hexdigest()
    except Exception as e:
        return f"ERROR: {str(e)}"

def get_ssh_fingerprint(host, port=22):
    """Wraps ssh-keyscan to retrieve SSH public key fingerprint."""
    try:
        # Security invariant: No shell=True. Arguments passed as list.
        cmd = ["ssh-keyscan", "-p", str(port), "-t", "rsa", host]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=5)
        
        if not result.stdout:
            return "ERROR: Connection failed or no keys"

        # Output format: host ssh-rsa AAAAB3...
        parts = result.stdout.strip().split()
        if len(parts) >= 3:
            key_b64 = parts[2]
            key_bytes = base64.b64decode(key_b64)
            return hashlib.sha256(key_bytes).hexdigest()
        return "ERROR: Parse failure"
    except Exception as e:
        return f"ERROR: {str(e)}"