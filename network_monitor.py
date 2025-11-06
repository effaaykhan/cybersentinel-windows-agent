"""
Network Monitor Module - Detects file uploads through browsers
Monitors HTTP/HTTPS traffic for file uploads to cloud services
"""

import logging
import threading
import time
import re
from collections import deque
from datetime import datetime
from typing import Dict, List, Optional, Callable

try:
    from scapy.all import sniff, IP, TCP, Raw
    SCAPY_AVAILABLE = True
except ImportError:
    SCAPY_AVAILABLE = False
    logging.warning("scapy not available - network monitoring limited")

logger = logging.getLogger('NetworkMonitor')


# Known cloud service domains and patterns
CLOUD_SERVICES = {
    'google_drive': {
        'domains': ['drive.google.com', 'docs.google.com'],
        'upload_patterns': [r'/upload', r'/_/upload', r'/file/upload'],
        'name': 'Google Drive'
    },
    'onedrive': {
        'domains': ['onedrive.live.com', 'onedrive.com', '1drv.ms', 'sharepoint.com'],
        'upload_patterns': [r'/upload', r'/_api/web/GetFileByUrl'],
        'name': 'Microsoft OneDrive'
    },
    'dropbox': {
        'domains': ['dropbox.com', 'www.dropbox.com'],
        'upload_patterns': [r'/upload', r'/home/upload'],
        'name': 'Dropbox'
    },
    'box': {
        'domains': ['box.com', 'app.box.com'],
        'upload_patterns': [r'/api/2.0/files/content', r'/upload'],
        'name': 'Box'
    },
    'icloud': {
        'domains': ['icloud.com', 'www.icloud.com'],
        'upload_patterns': [r'/upload'],
        'name': 'iCloud Drive'
    },
    'mega': {
        'domains': ['mega.nz', 'mega.io'],
        'upload_patterns': [r'/upload'],
        'name': 'MEGA'
    },
    'aws_s3': {
        'domains': ['s3.amazonaws.com', '.s3.amazonaws.com'],
        'upload_patterns': [r'/'],
        'name': 'AWS S3'
    },
    'azure_blob': {
        'domains': ['blob.core.windows.net'],
        'upload_patterns': [r'/'],
        'name': 'Azure Blob Storage'
    }
}


class NetworkMonitor:
    """Monitors network traffic for file uploads"""

    def __init__(self, callback: Optional[Callable] = None):
        self.callback = callback
        self.running = False
        self.monitor_thread = None
        self.upload_buffer = deque(maxlen=100)  # Track recent uploads

    def start(self):
        """Start network monitoring"""
        if not SCAPY_AVAILABLE:
            logger.error("Cannot start network monitoring - scapy not installed")
            logger.info("Install: pip install scapy")
            return False

        self.running = True
        self.monitor_thread = threading.Thread(target=self._monitor_loop, daemon=True)
        self.monitor_thread.start()
        logger.info("Network monitoring started")
        return True

    def stop(self):
        """Stop network monitoring"""
        self.running = False
        if self.monitor_thread:
            self.monitor_thread.join(timeout=5)
        logger.info("Network monitoring stopped")

    def _monitor_loop(self):
        """Main monitoring loop using packet capture"""
        try:
            # Capture HTTP/HTTPS traffic (ports 80, 443)
            # Note: HTTPS content is encrypted, but we can still see domains and detect uploads by packet patterns
            sniff(
                filter="tcp port 80 or tcp port 443",
                prn=self._process_packet,
                store=False,
                stop_filter=lambda x: not self.running
            )
        except Exception as e:
            logger.error(f"Network monitoring error: {e}")
            logger.error("Make sure you run the agent as Administrator")

    def _process_packet(self, packet):
        """Process captured network packet"""
        try:
            if not packet.haslayer(IP) or not packet.haslayer(TCP):
                return

            ip_layer = packet[IP]
            tcp_layer = packet[TCP]

            # Check if packet has payload
            if packet.haslayer(Raw):
                payload = packet[Raw].load

                # Try to decode as HTTP
                try:
                    payload_str = payload.decode('utf-8', errors='ignore')

                    # Detect HTTP POST requests (file uploads)
                    if payload_str.startswith('POST ') or payload_str.startswith('PUT '):
                        self._analyze_http_upload(payload_str, ip_layer.dst, tcp_layer.dport)

                except:
                    pass

                # Detect multipart/form-data (file upload pattern)
                if b'Content-Type: multipart/form-data' in payload:
                    self._detect_file_upload(payload, ip_layer.dst)

        except Exception as e:
            logger.debug(f"Packet processing error: {e}")

    def _analyze_http_upload(self, http_request: str, dest_ip: str, dest_port: int):
        """Analyze HTTP request for file uploads"""
        lines = http_request.split('\r\n')

        if not lines:
            return

        request_line = lines[0]
        headers = {}

        # Parse headers
        for line in lines[1:]:
            if ':' in line:
                key, value = line.split(':', 1)
                headers[key.strip().lower()] = value.strip()

        # Check for file upload indicators
        content_type = headers.get('content-type', '')
        host = headers.get('host', '')

        # Detect multipart form data (file uploads)
        if 'multipart/form-data' in content_type:
            cloud_service = self._identify_cloud_service(host, request_line)

            if cloud_service:
                upload_event = {
                    'timestamp': datetime.utcnow().isoformat(),
                    'destination': cloud_service['name'],
                    'host': host,
                    'dest_ip': dest_ip,
                    'dest_port': dest_port,
                    'request': request_line,
                    'content_type': content_type
                }

                self._handle_upload_event(upload_event)

    def _detect_file_upload(self, payload: bytes, dest_ip: str):
        """Detect file upload from packet payload"""
        try:
            # Look for multipart boundary
            if b'boundary=' in payload:
                # Extract filename from Content-Disposition header
                match = re.search(b'filename="([^"]+)"', payload)
                if match:
                    filename = match.group(1).decode('utf-8', errors='ignore')
                    logger.info(f"Detected file upload: {filename} to {dest_ip}")

                    # Extract file content preview (first 1KB)
                    content_preview = self._extract_file_content(payload)

                    if self.callback:
                        self.callback({
                            'event_type': 'network_upload',
                            'filename': filename,
                            'dest_ip': dest_ip,
                            'content_preview': content_preview,
                            'timestamp': datetime.utcnow().isoformat()
                        })
        except Exception as e:
            logger.debug(f"File upload detection error: {e}")

    def _extract_file_content(self, payload: bytes) -> bytes:
        """Extract file content from multipart payload"""
        try:
            # Find the start of file content (after headers)
            parts = payload.split(b'\r\n\r\n')
            if len(parts) > 1:
                return parts[1][:1024]  # First 1KB
        except:
            pass
        return b''

    def _identify_cloud_service(self, host: str, request_line: str) -> Optional[Dict]:
        """Identify which cloud service is being used"""
        host_lower = host.lower()

        for service_id, service_info in CLOUD_SERVICES.items():
            # Check domain match
            for domain in service_info['domains']:
                if domain in host_lower:
                    # Check upload pattern in URL
                    for pattern in service_info['upload_patterns']:
                        if re.search(pattern, request_line, re.IGNORECASE):
                            return service_info

        return None

    def _handle_upload_event(self, event: Dict):
        """Handle detected upload event"""
        # Deduplicate (avoid multiple packets for same upload)
        event_key = f"{event['host']}:{event['timestamp'][:19]}"  # Group by second

        if event_key not in [e.get('key') for e in self.upload_buffer]:
            event['key'] = event_key
            self.upload_buffer.append(event)

            logger.warning(f"🌐 CLOUD UPLOAD DETECTED: {event['destination']}")
            logger.info(f"   Host: {event['host']}")
            logger.info(f"   Request: {event['request']}")

            if self.callback:
                self.callback({
                    'event_type': 'cloud_upload',
                    'service': event['destination'],
                    'host': event['host'],
                    'dest_ip': event['dest_ip'],
                    'timestamp': event['timestamp']
                })


class ProcessFileMonitor:
    """Monitors which processes access which files (browser file access)"""

    def __init__(self, callback: Optional[Callable] = None):
        self.callback = callback
        self.running = False
        self.browser_processes = ['chrome.exe', 'msedge.exe', 'firefox.exe', 'opera.exe', 'brave.exe']

    def start(self):
        """Start monitoring browser file access"""
        try:
            import psutil
            self.running = True
            threading.Thread(target=self._monitor_browser_files, daemon=True).start()
            logger.info("Process file monitoring started")
            return True
        except ImportError:
            logger.error("psutil not available - install: pip install psutil")
            return False

    def stop(self):
        """Stop monitoring"""
        self.running = False

    def _monitor_browser_files(self):
        """Monitor files opened by browsers"""
        import psutil

        known_files = set()

        while self.running:
            try:
                for proc in psutil.process_iter(['name', 'open_files']):
                    if proc.info['name'].lower() in self.browser_processes:
                        open_files = proc.info.get('open_files') or []

                        for file_obj in open_files:
                            file_path = file_obj.path

                            # Check if it's a user file (not browser cache/temp)
                            if self._is_user_file(file_path) and file_path not in known_files:
                                known_files.add(file_path)

                                logger.info(f"Browser accessing file: {file_path}")

                                if self.callback:
                                    self.callback({
                                        'event_type': 'browser_file_access',
                                        'process': proc.info['name'],
                                        'file_path': file_path,
                                        'timestamp': datetime.utcnow().isoformat()
                                    })

            except (psutil.NoSuchProcess, psutil.AccessDenied):
                pass
            except Exception as e:
                logger.debug(f"Process monitoring error: {e}")

            time.sleep(2)

    def _is_user_file(self, file_path: str) -> bool:
        """Check if file is a user file (not browser cache)"""
        file_path_lower = file_path.lower()

        # Exclude browser cache, temp, and system files
        exclude_patterns = [
            'cache', 'temp', 'tmp', 'appdata\\local\\google',
            'appdata\\local\\microsoft\\edge', 'cookies', 'history',
            'webdata', '.tmp', 'recovery'
        ]

        for pattern in exclude_patterns:
            if pattern in file_path_lower:
                return False

        # Include user document folders
        include_patterns = ['documents', 'desktop', 'downloads', 'pictures', 'videos']

        for pattern in include_patterns:
            if pattern in file_path_lower:
                return True

        return False


# Example usage
if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)

    def upload_callback(event):
        print(f"\n🚨 UPLOAD DETECTED: {event}")

    # Start network monitor
    net_monitor = NetworkMonitor(callback=upload_callback)
    net_monitor.start()

    # Start process monitor
    proc_monitor = ProcessFileMonitor(callback=upload_callback)
    proc_monitor.start()

    print("Monitoring for cloud uploads... (Press Ctrl+C to stop)")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        net_monitor.stop()
        proc_monitor.stop()
        print("\nMonitoring stopped")
