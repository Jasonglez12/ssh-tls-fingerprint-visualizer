import unittest
from unittest.mock import patch, MagicMock
from src.collectors import get_tls_fingerprint, get_ssh_fingerprint
from src.utils import validate_target

class TestFingerprints(unittest.TestCase):

    # Happy Path Test
    @patch('src.collectors.ssl.create_default_context')
    @patch('src.collectors.socket.create_connection')
    def test_tls_success(self, mock_socket, mock_context):
        # Mocking the SSL context and socket to avoid real network calls
        mock_ssock = MagicMock()
        mock_ssock.getpeercert.return_value = b'fake_cert_data'
        
        context_instance = mock_context.return_value
        context_instance.wrap_socket.return_value.__enter__.return_value = mock_ssock
        
        fp = get_tls_fingerprint("example.com")
        # SHA256 of b'fake_cert_data'
        self.assertNotEqual(fp, "ERROR")
        self.assertTrue(len(fp) > 10)

    # Negative Test (Network Error)
    def test_tls_failure(self):
        # Using a non-existent domain/port to force an error
        fp = get_tls_fingerprint("invalid.domain.fails", port=12345)
        self.assertTrue(fp.startswith("ERROR"))

    # Security/Hardening Test
    def test_validation(self):
        self.assertTrue(validate_target("google.com"))
        self.assertFalse(validate_target("google.com; rm -rf /")) # Injection attempt
        self.assertTrue(validate_target("8.8.8.8"))

if __name__ == '__main__':
    unittest.main()