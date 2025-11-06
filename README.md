# CyberSentinel DLP - Windows Agent

🛡️ **Enterprise Data Loss Prevention Endpoint Agent for Windows**

⚡ **High-Performance C++ Implementation** - 5-10x faster, 80% less memory usage, <100ms startup time

[![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-0078D4?logo=windows)](https://www.microsoft.com/windows)
[![C++](https://img.shields.io/badge/C++-17-00599C?logo=c%2B%2B)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/python-3.8+-3776AB?logo=python&logoColor=white)](https://www.python.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/effaaykhan/cybersentinel-windows-agent?style=social)](https://github.com/effaaykhan/cybersentinel-windows-agent/stargazers)
[![GitHub issues](https://img.shields.io/github/issues/effaaykhan/cybersentinel-windows-agent)](https://github.com/effaaykhan/cybersentinel-windows-agent/issues)
[![GitHub last commit](https://img.shields.io/github/last-commit/effaaykhan/cybersentinel-windows-agent)](https://github.com/effaaykhan/cybersentinel-windows-agent/commits/main)

![Status](https://img.shields.io/badge/status-production--ready-brightgreen)
![Version](https://img.shields.io/badge/version-1.0.0-blue)
![Maintenance](https://img.shields.io/badge/maintenance-active-brightgreen)

## 🚀 Two Implementation Options

### **C++ Version** (Recommended for Production)
- ⚡ **5-10x faster** than Python version
- 💾 **80% less memory** (~10MB vs ~50MB)
- ⏱️ **<100ms startup time** (vs ~2 seconds)
- 🔧 Native Windows APIs for optimal performance
- 📦 Professional CMake build system
- **Best for:** Large deployments (100+ endpoints), performance-critical environments

### **Python Version** (Quick Deployment)
- 🎯 **One-click PowerShell installer**
- ⚙️ Easy to deploy and configure
- 🔌 Plug-and-play setup
- **Best for:** Small-medium deployments (<100 endpoints), quick testing

## Features

- ✅ **File System Monitoring** - Real-time monitoring of file operations
- ✅ **Clipboard Monitoring** - Detects sensitive data in clipboard
- ✅ **USB Device Detection** - Alerts on USB device connections
- ✅ **🆕 Browser Upload Detection** - Detects file uploads to Google Drive, OneDrive, Dropbox, etc.
- ✅ **🆕 Network Traffic Analysis** - Monitors HTTP/HTTPS uploads in real-time
- ✅ **🆕 Cloud Service Identification** - Identifies which cloud service files are uploaded to
- ✅ **Automatic Classification** - Pattern-based sensitive data detection
- ✅ **Real-time Reporting** - Sends events to central server
- ✅ **Configurable Monitoring** - Customize paths and file types

## Requirements

### C++ Version (100% Automatic Installation)
- ✅ Windows 10/11 or Windows Server 2016+
- ✅ Administrator privileges (run PowerShell as Admin)
- ✅ **That's it!** Everything else is installed automatically:
  - Visual Studio Build Tools 2022 (auto-installed)
  - Git (auto-installed)
  - CMake 3.20+ (auto-installed)
  - vcpkg (auto-setup)
  - Dependencies: libcurl, nlohmann/json (auto-compiled)

### Python Version (100% Automatic Installation)
- ✅ Windows 10/11 or Windows Server 2016+
- ✅ Administrator privileges (run PowerShell as Admin)
- ✅ **That's it!** Everything else is installed automatically:
  - Python 3.8+ (auto-installed)
  - All dependencies (auto-installed)

---

## 🚀 Quick Start

### Option 1: C++ Version (High Performance) ⚡

**One-Click Installation - 100% Automatic! (Recommended):**

```powershell
# Run PowerShell as Administrator, then execute:
iwr -useb https://raw.githubusercontent.com/effaaykhan/cybersentinel-windows-agent/main/install-cpp.ps1 | iex
```

**This FULLY automated installer will:**
- ✅ **Auto-install Visual Studio Build Tools 2022** (~3 GB download, 15-30 min)
- ✅ **Auto-install Git, CMake** (~2 min)
- ✅ **Setup vcpkg** and install dependencies (libcurl, nlohmann/json) (~5-10 min)
- ✅ **Download and build the C++ agent** (~2-3 min)
- ✅ **Configure agent** interactively (or silent mode)
- ✅ **Install as Windows Service** (optional)
- ✅ **Start monitoring immediately**

**⏱️ Total Time: ~25-45 minutes on first run (all automatic!)**
**⏱️ Subsequent runs: ~10-15 minutes (if VS Build Tools already installed)**

**🎯 ZERO manual steps required - just run the command and wait!**

**Silent Installation:**
```powershell
iwr -useb https://raw.githubusercontent.com/effaaykhan/cybersentinel-windows-agent/main/install-cpp.ps1 | iex -ServerURL "http://192.168.1.100:8000/api/v1" -AgentID "WIN-001" -AgentName "MyComputer" -AsService -Silent
```

**Manual Build:** See [BUILD.md](BUILD.md) for manual build instructions.

### Option 2: Python Version (Quick Setup)

**One-Click Installation:**

```powershell
# Run PowerShell as Administrator, then execute:
iwr -useb https://raw.githubusercontent.com/effaaykhan/cybersentinel-windows-agent/main/install.ps1 | iex
```

**This installer will:**
- ✅ Install Python if needed
- ✅ Download agent files
- ✅ Install dependencies
- ✅ Configure interactively
- ✅ Install as Windows Service
- ✅ Start monitoring

**Installation takes ~2-3 minutes**

---

## Performance Comparison

| Metric | C++ Version | Python Version | Improvement |
|--------|-------------|----------------|-------------|
| **Memory Usage** | ~10MB | ~50MB | **80% reduction** |
| **CPU Usage** | Low | Medium | **50-70% reduction** |
| **Startup Time** | <100ms | ~2 seconds | **20x faster** |
| **File Scanning** | Native regex | Python regex | **5-10x faster** |
| **First Install Time** | 25-45 min (100% auto) | 2-3 min (100% auto) | Trade-off |
| **Subsequent Install** | 10-15 min (100% auto) | 2-3 min (100% auto) | Faster |
| **Manual Steps** | **ZERO** | **ZERO** | Both fully automatic! |

**Recommendation:**
- **Production (100+ endpoints):** Use C++ version for performance
- **Testing/Small deployments:** Use Python version for ease of deployment

---

## Configuration

Both C++ and Python versions use the same `agent_config.json` file:

```json
{
  "server_url": "http://YOUR-SERVER-IP:8000/api/v1",
  "agent_name": "YOUR-AGENT-NAME",
  "monitoring": {
    "file_system": true,
    "clipboard": true,
    "usb_devices": true,
    "network_uploads": true,
    "browser_file_access": true,
    "monitored_paths": [
      "C:\\Users\\Public\\Documents",
      "C:\\Users\\%USERNAME%\\Desktop"
    ]
  }
}
```

### Network Monitoring (Browser Uploads)

**⚠️ Requires Administrator privileges and additional dependencies:**

```powershell
pip install scapy psutil
```

**Detected Cloud Services:**
- ✅ Google Drive (drive.google.com)
- ✅ Microsoft OneDrive (onedrive.live.com)
- ✅ Dropbox (dropbox.com)
- ✅ Box (box.com)
- ✅ iCloud Drive (icloud.com)
- ✅ MEGA (mega.nz)
- ✅ AWS S3 (s3.amazonaws.com)
- ✅ Azure Blob Storage (blob.core.windows.net)

**How it works:**
1. Monitors network packets for HTTP/HTTPS file uploads
2. Detects multipart/form-data (file upload pattern)
3. Identifies destination cloud service by domain
4. Monitors browser processes accessing files
5. Correlates file access with network activity
6. Triggers alerts based on DLP policies

## Installation as Windows Service

### C++ Version

**Using NSSM (Recommended):**
```powershell
# Download NSSM from https://nssm.cc/download

# Install service
nssm install CyberSentinelDLP "C:\path\to\build\bin\Release\CyberSentinelAgent.exe"
nssm set CyberSentinelDLP AppDirectory "C:\path\to\build\bin\Release"
nssm start CyberSentinelDLP

# Check status
nssm status CyberSentinelDLP
```

**Using sc.exe:**
```powershell
sc create CyberSentinelDLP binPath= "C:\path\to\CyberSentinelAgent.exe" start= auto
sc start CyberSentinelDLP
sc query CyberSentinelDLP
```

### Python Version

**Automatic (via installer):**
The one-click PowerShell installer offers to set up the service automatically.

**Manual:**
```powershell
nssm install CyberSentinelDLP "C:\Path\To\Python\python.exe" "C:\Path\To\agent.py"
nssm start CyberSentinelDLP
```

## Monitored Events

| Event Type | Description |
|------------|-------------|
| **File Created** | New file created in monitored directories |
| **File Modified** | File content changed |
| **File Moved** | File moved or renamed |
| **Clipboard Copy** | Sensitive data copied to clipboard |
| **USB Connected** | USB device plugged in |
| **🆕 Cloud Upload** | File uploaded to Google Drive, OneDrive, Dropbox, etc. |
| **🆕 Browser File Access** | Browser accessing sensitive files (potential upload) |
| **🆕 Network File Transfer** | Files transferred over network |

## Sensitive Data Detection

The agent detects:
- Credit Card Numbers (PAN)
- Social Security Numbers (SSN)
- Email Addresses
- API Keys and Secrets
- Custom patterns (configurable)

## Logs

Agent logs are saved to: `cybersentinel_agent.log`

## Troubleshooting

### Agent won't start
- Check if server URL is correct
- Verify network connectivity: `ping YOUR-SERVER-IP`
- Check logs in `cybersentinel_agent.log`

### No events showing on server
- Verify agent is running: Check Task Manager
- Check firewall rules: Port 8000 should be open
- Verify server API is accessible

### Permission errors
- Run as Administrator
- Check monitored path permissions

## Uninstallation

```powershell
# Stop service
Stop-Service -Name "CyberSentinelDLP"

# Remove service
Remove-Service -Name "CyberSentinelDLP"

# Or using NSSM
nssm stop CyberSentinelDLP
nssm remove CyberSentinelDLP confirm
```

## Support

### Documentation
- **C++ Build Guide:** [BUILD.md](BUILD.md)
- **Main Platform:** https://github.com/effaaykhan/cybersentinel-dlp
- **Issues:** https://github.com/effaaykhan/cybersentinel-windows-agent/issues

### Troubleshooting
- Check logs: `cybersentinel_agent.log`
- Review server connectivity
- Verify configuration in `agent_config.json`

## Version

**Version**: 1.0.0
**Platform**: Windows 10/11, Windows Server 2016+
**C++ Standard**: C++17
**Python Version**: 3.8+
**Last Updated**: January 2025

## License

MIT License - See [LICENSE](LICENSE) for details
