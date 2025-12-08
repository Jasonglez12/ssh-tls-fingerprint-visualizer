import re
import ipaddress

def validate_target(host):
    """
    Security Hardening: Validates that the input is a valid hostname or IP.
    Prevents command injection via the host string.
    """
    # Check if IP
    try:
        ipaddress.ip_address(host)
        return True
    except ValueError:
        pass

    # Check if valid hostname (regex)
    if len(host) > 255:
        return False
    if host[-1] == ".":
        host = host[:-1]
    
    # Simple regex for hostname validation
    allowed = re.compile(r"(?!-)[A-Z\d-]{1,63}(?<!-)$", re.IGNORECASE)
    return all(allowed.match(x) for x in host.split("."))