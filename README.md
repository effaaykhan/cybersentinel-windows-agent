# CyberSentinel DLP - Windows Agent

🛡️ **Enterprise Data Loss Prevention Endpoint Agent for Windows**

⚡ **NEW: High-Performance C++ Version Available!** - 5-10x faster, 80% less memory usage

[![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-0078D4?logo=windows)](https://www.microsoft.com/windows)
[![Python](https://img.shields.io/badge/python-3.8+-3776AB?logo=python&logoColor=white)](https://www.python.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/effaaykhan/cybersentinel-windows-agent?style=social)](https://github.com/effaaykhan/cybersentinel-windows-agent/stargazers)
[![GitHub issues](https://img.shields.io/github/issues/effaaykhan/cybersentinel-windows-agent)](https://github.com/effaaykhan/cybersentinel-windows-agent/issues)
[![GitHub last commit](https://img.shields.io/github/last-commit/effaaykhan/cybersentinel-windows-agent)](https://github.com/effaaykhan/cybersentinel-windows-agent/commits/main)

![Status](https://img.shields.io/badge/status-production--ready-brightgreen)
![Version](https://img.shields.io/badge/version-1.0.0-blue)
![Maintenance](https://img.shields.io/badge/maintenance-active-brightgreen)

## Features

- ✅ **File System Monitoring** - Real-time monitoring of file operations
- ✅ **Clipboard Monitoring** - Detects sensitive data in clipboard
- ✅ **USB Device Detection** - Alerts on USB device connections
- ✅ **Automatic Classification** - Pattern-based sensitive data detection
- ✅ **Real-time Reporting** - Sends events to central server
- ✅ **Configurable Monitoring** - Customize paths and file types

## Requirements

- Windows 10/11 or Windows Server 2016+
- Python 3.8+ (automatically installed if needed)
- Administrator privileges (for installation)

## 🚀 One-Click Installation

**The easiest way to install the agent - just copy and paste this into PowerShell:**

```powershell
# Run PowerShell as Administrator, then execute:
iwr -useb https://raw.githubusercontent.com/effaaykhan/cybersentinel-windows-agent/main/install.ps1 | iex
```

This single command will:
- ✅ Check and install Python if needed
- ✅ Download the latest agent files
- ✅ Install all dependencies automatically
- ✅ Configure the agent interactively
- ✅ Optionally install as Windows Service
- ✅ Start the agent immediately

**Interactive Setup:** The installer will ask for:
- Server IP address (e.g., `192.168.1.100`)
- Agent ID (auto-generated from computer name)
- Agent Name (auto-generated from user/computer)
- Whether to install as a Windows Service

**Silent Installation:**
```powershell
iwr -useb https://raw.githubusercontent.com/effaaykhan/cybersentinel-windows-agent/main/install.ps1 | iex -ServerURL "http://192.168.1.100:8000/api/v1" -AgentID "WIN-001" -AgentName "MyComputer" -AsService -Silent
```

---

## Alternative Installation Methods

### Option 1: Run from Source

```bash
# Install dependencies
pip install -r requirements.txt

# Configure agent
# Edit agent_config.json with your server URL

# Run agent
python agent.py
```

### Option 2: Build Executable

```bash
# Build standalone executable
build_agent.bat

# Run executable
dist\CyberSentinelAgent.exe
```

## Configuration

Edit `agent_config.json`:

```json
{
  "server_url": "http://YOUR-SERVER-IP:8000/api/v1",
  "agent_name": "YOUR-AGENT-NAME",
  "monitoring": {
    "file_system": true,
    "clipboard": true,
    "usb_devices": true,
    "monitored_paths": [
      "C:\\Users\\Public\\Documents",
      "C:\\Users\\%USERNAME%\\Desktop"
    ]
  }
}
```

## Installation as Windows Service

### Using NSSM (Recommended)

```powershell
# Download NSSM
# https://nssm.cc/download

# Install service
nssm install CyberSentinelDLP "C:\Path\To\CyberSentinelAgent.exe"

# Start service
nssm start CyberSentinelDLP

# Check status
nssm status CyberSentinelDLP
```

### Using PowerShell

```powershell
# Create service
New-Service -Name "CyberSentinelDLP" `
    -BinaryPathName "C:\Path\To\CyberSentinelAgent.exe" `
    -DisplayName "CyberSentinel DLP Agent" `
    -Description "Data Loss Prevention endpoint agent" `
    -StartupType Automatic

# Start service
Start-Service -Name "CyberSentinelDLP"

# Check status
Get-Service -Name "CyberSentinelDLP"
```

## Monitored Events

| Event Type | Description |
|------------|-------------|
| **File Created** | New file created in monitored directories |
| **File Modified** | File content changed |
| **File Moved** | File moved or renamed |
| **Clipboard Copy** | Sensitive data copied to clipboard |
| **USB Connected** | USB device plugged in |

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

For issues or questions:
- Check logs: `cybersentinel_agent.log`
- Review server logs
- Contact: support@cybersentinel.local

## Version

**Version**: 1.0.0
**Platform**: Windows 10/11, Windows Server 2016+
**Last Updated**: January 2025
