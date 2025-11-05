# CyberSentinel DLP - Windows Agent One-Click Installer
# Usage: iwr -useb https://raw.githubusercontent.com/effaaykhan/cybersentinel-windows-agent/main/install.ps1 | iex

param(
    [string]$ServerURL = "",
    [string]$AgentID = "",
    [string]$AgentName = "",
    [string]$InstallPath = "C:\Program Files\CyberSentinel",
    [switch]$AsService = $false,
    [switch]$Silent = $false
)

# Colors for output
$ErrorColor = "Red"
$SuccessColor = "Green"
$InfoColor = "Cyan"
$WarningColor = "Yellow"

function Write-ColorOutput {
    param([string]$Message, [string]$Color = "White")
    Write-Host $Message -ForegroundColor $Color
}

function Test-Administrator {
    $currentUser = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    return $currentUser.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Test-Python {
    try {
        $pythonVersion = python --version 2>&1
        if ($pythonVersion -match "Python 3\.[8-9]|Python 3\.1[0-9]") {
            return $true
        }
        return $false
    } catch {
        return $false
    }
}

function Install-Python {
    Write-ColorOutput "`n[*] Python 3.8+ not found. Installing Python..." $InfoColor

    $pythonUrl = "https://www.python.org/ftp/python/3.11.7/python-3.11.7-amd64.exe"
    $pythonInstaller = "$env:TEMP\python-installer.exe"

    try {
        Write-ColorOutput "[*] Downloading Python 3.11.7..." $InfoColor
        Invoke-WebRequest -Uri $pythonUrl -OutFile $pythonInstaller -UseBasicParsing

        Write-ColorOutput "[*] Installing Python (this may take a few minutes)..." $InfoColor
        Start-Process -FilePath $pythonInstaller -ArgumentList "/quiet InstallAllUsers=1 PrependPath=1" -Wait

        # Refresh PATH
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

        Remove-Item $pythonInstaller -Force
        Write-ColorOutput "[+] Python installed successfully!" $SuccessColor
        return $true
    } catch {
        Write-ColorOutput "[-] Failed to install Python: $_" $ErrorColor
        return $false
    }
}

# Main installation script
Write-ColorOutput @"
╔══════════════════════════════════════════════════════════╗
║   CyberSentinel DLP - Windows Agent Installer v1.0       ║
║   Enterprise Data Loss Prevention                        ║
╚══════════════════════════════════════════════════════════╝
"@ $InfoColor

# Check if running as Administrator
if (-not (Test-Administrator)) {
    Write-ColorOutput "`n[-] ERROR: This script requires Administrator privileges!" $ErrorColor
    Write-ColorOutput "[!] Please run PowerShell as Administrator and try again." $WarningColor
    Write-ColorOutput "`nRight-click PowerShell -> Run as Administrator" $InfoColor
    exit 1
}

Write-ColorOutput "`n[+] Running with Administrator privileges" $SuccessColor

# Check for Python
Write-ColorOutput "`n[*] Checking Python installation..." $InfoColor
if (-not (Test-Python)) {
    if ($Silent) {
        Write-ColorOutput "[-] Python 3.8+ required but not found. Use -Silent:`$false for interactive install." $ErrorColor
        exit 1
    }

    $installPython = Read-Host "Python 3.8+ not found. Install Python now? (Y/n)"
    if ($installPython -ne "n" -and $installPython -ne "N") {
        if (-not (Install-Python)) {
            exit 1
        }
    } else {
        Write-ColorOutput "[-] Python is required. Please install Python 3.8+ manually." $ErrorColor
        exit 1
    }
} else {
    Write-ColorOutput "[+] Python found: $(python --version)" $SuccessColor
}

# Get configuration if not provided
if (-not $Silent) {
    Write-ColorOutput "`n╔══════════════════════════════════════════════════════════╗" $InfoColor
    Write-ColorOutput "║              AGENT CONFIGURATION                         ║" $InfoColor
    Write-ColorOutput "╚══════════════════════════════════════════════════════════╝" $InfoColor

    if ([string]::IsNullOrWhiteSpace($ServerURL)) {
        Write-ColorOutput "`nEnter your CyberSentinel server details:" $InfoColor
        $ServerIP = Read-Host "Server IP address (e.g., 192.168.1.100)"
        $ServerURL = "http://$($ServerIP):8000/api/v1"
    }

    if ([string]::IsNullOrWhiteSpace($AgentID)) {
        $defaultAgentID = "WIN-$env:COMPUTERNAME"
        $inputAgentID = Read-Host "Agent ID (press Enter for '$defaultAgentID')"
        $AgentID = if ([string]::IsNullOrWhiteSpace($inputAgentID)) { $defaultAgentID } else { $inputAgentID }
    }

    if ([string]::IsNullOrWhiteSpace($AgentName)) {
        $defaultAgentName = "$env:COMPUTERNAME-$env:USERNAME"
        $inputAgentName = Read-Host "Agent Name (press Enter for '$defaultAgentName')"
        $AgentName = if ([string]::IsNullOrWhiteSpace($inputAgentName)) { $defaultAgentName } else { $inputAgentName }
    }

    Write-ColorOutput "`n[*] Configuration:" $InfoColor
    Write-ColorOutput "    Server URL: $ServerURL" $InfoColor
    Write-ColorOutput "    Agent ID:   $AgentID" $InfoColor
    Write-ColorOutput "    Agent Name: $AgentName" $InfoColor

    $confirm = Read-Host "`nProceed with installation? (Y/n)"
    if ($confirm -eq "n" -or $confirm -eq "N") {
        Write-ColorOutput "Installation cancelled." $WarningColor
        exit 0
    }
}

# Create installation directory
Write-ColorOutput "`n[*] Creating installation directory: $InstallPath" $InfoColor
try {
    New-Item -Path $InstallPath -ItemType Directory -Force | Out-Null
    Write-ColorOutput "[+] Directory created successfully" $SuccessColor
} catch {
    Write-ColorOutput "[-] Failed to create directory: $_" $ErrorColor
    exit 1
}

# Download agent files from GitHub
Write-ColorOutput "`n[*] Downloading agent files from GitHub..." $InfoColor
$repoUrl = "https://github.com/effaaykhan/cybersentinel-windows-agent"
$zipUrl = "$repoUrl/archive/refs/heads/main.zip"
$zipFile = "$env:TEMP\cybersentinel-agent.zip"
$extractPath = "$env:TEMP\cybersentinel-agent"

try {
    # Download ZIP
    Write-ColorOutput "[*] Downloading from $repoUrl..." $InfoColor
    Invoke-WebRequest -Uri $zipUrl -OutFile $zipFile -UseBasicParsing
    Write-ColorOutput "[+] Downloaded successfully" $SuccessColor

    # Extract ZIP
    Write-ColorOutput "[*] Extracting files..." $InfoColor
    Expand-Archive -Path $zipFile -DestinationPath $extractPath -Force

    # Copy files to installation directory
    $sourceDir = Get-ChildItem -Path $extractPath -Directory | Select-Object -First 1
    Copy-Item -Path "$($sourceDir.FullName)\*" -Destination $InstallPath -Recurse -Force

    # Cleanup
    Remove-Item $zipFile -Force
    Remove-Item $extractPath -Recurse -Force

    Write-ColorOutput "[+] Agent files installed successfully" $SuccessColor
} catch {
    Write-ColorOutput "[-] Failed to download/extract agent files: $_" $ErrorColor
    exit 1
}

# Install Python dependencies
Write-ColorOutput "`n[*] Installing Python dependencies..." $InfoColor
try {
    Set-Location $InstallPath
    python -m pip install --upgrade pip --quiet
    python -m pip install -r requirements.txt --quiet
    Write-ColorOutput "[+] Dependencies installed successfully" $SuccessColor
} catch {
    Write-ColorOutput "[-] Failed to install dependencies: $_" $ErrorColor
    Write-ColorOutput "[!] You may need to install dependencies manually: pip install -r requirements.txt" $WarningColor
}

# Create configuration file
Write-ColorOutput "`n[*] Creating configuration file..." $InfoColor
$configContent = @"
{
  "server_url": "$ServerURL",
  "agent_id": "$AgentID",
  "agent_name": "$AgentName",
  "heartbeat_interval": 60,
  "monitoring": {
    "file_system": true,
    "clipboard": true,
    "usb_devices": true,
    "monitored_paths": [
      "C:\\Users\\Public\\Documents",
      "C:\\Users\\%USERNAME%\\Documents",
      "C:\\Users\\%USERNAME%\\Desktop",
      "C:\\Users\\%USERNAME%\\Downloads"
    ],
    "file_extensions": [
      ".pdf",
      ".docx",
      ".doc",
      ".xlsx",
      ".xls",
      ".csv",
      ".txt",
      ".json",
      ".xml",
      ".sql"
    ]
  },
  "classification": {
    "enabled": true,
    "max_file_size_mb": 10
  }
}
"@

try {
    $configContent | Out-File -FilePath "$InstallPath\agent_config.json" -Encoding UTF8 -Force
    Write-ColorOutput "[+] Configuration file created" $SuccessColor
} catch {
    Write-ColorOutput "[-] Failed to create configuration file: $_" $ErrorColor
    exit 1
}

# Test server connectivity
Write-ColorOutput "`n[*] Testing connection to server..." $InfoColor
try {
    $healthUrl = $ServerURL -replace "/api/v1", "/health"
    $response = Invoke-WebRequest -Uri $healthUrl -UseBasicParsing -TimeoutSec 5 -ErrorAction Stop
    if ($response.StatusCode -eq 200) {
        Write-ColorOutput "[+] Successfully connected to server!" $SuccessColor
    }
} catch {
    Write-ColorOutput "[!] WARNING: Could not connect to server at $ServerURL" $WarningColor
    Write-ColorOutput "[!] Please verify the server is running and accessible" $WarningColor
    Write-ColorOutput "[!] The agent will still be installed but may not connect until the server is available" $InfoColor
}

# Ask about service installation
if (-not $Silent) {
    Write-ColorOutput "`n╔══════════════════════════════════════════════════════════╗" $InfoColor
    Write-ColorOutput "║           SERVICE INSTALLATION (Optional)                ║" $InfoColor
    Write-ColorOutput "╚══════════════════════════════════════════════════════════╝" $InfoColor
    Write-ColorOutput "`nInstall as Windows Service for automatic startup?" $InfoColor
    Write-ColorOutput "  - Service will run automatically on system boot" $InfoColor
    Write-ColorOutput "  - Runs in background without user login" $InfoColor
    $installService = Read-Host "`nInstall as service? (Y/n)"
    $AsService = ($installService -ne "n" -and $installService -ne "N")
}

if ($AsService) {
    Write-ColorOutput "`n[*] Installing as Windows Service..." $InfoColor

    # Check if NSSM is available
    $nssmPath = "$InstallPath\nssm.exe"
    if (-not (Test-Path $nssmPath)) {
        Write-ColorOutput "[*] Downloading NSSM (Non-Sucking Service Manager)..." $InfoColor
        try {
            $nssmUrl = "https://nssm.cc/release/nssm-2.24.zip"
            $nssmZip = "$env:TEMP\nssm.zip"
            Invoke-WebRequest -Uri $nssmUrl -OutFile $nssmZip -UseBasicParsing
            Expand-Archive -Path $nssmZip -DestinationPath "$env:TEMP\nssm" -Force

            # Copy appropriate version (64-bit or 32-bit)
            $nssmExe = if ([Environment]::Is64BitOperatingSystem) {
                "$env:TEMP\nssm\nssm-2.24\win64\nssm.exe"
            } else {
                "$env:TEMP\nssm\nssm-2.24\win32\nssm.exe"
            }

            Copy-Item $nssmExe -Destination $nssmPath -Force
            Remove-Item $nssmZip -Force
            Remove-Item "$env:TEMP\nssm" -Recurse -Force
            Write-ColorOutput "[+] NSSM downloaded successfully" $SuccessColor
        } catch {
            Write-ColorOutput "[-] Failed to download NSSM: $_" $ErrorColor
            Write-ColorOutput "[!] Skipping service installation. You can install as service manually later." $WarningColor
            $AsService = $false
        }
    }

    if ($AsService) {
        try {
            # Get Python path
            $pythonPath = (Get-Command python).Source

            # Remove service if it already exists
            & $nssmPath stop CyberSentinelDLP 2>$null
            & $nssmPath remove CyberSentinelDLP confirm 2>$null

            # Install service
            & $nssmPath install CyberSentinelDLP $pythonPath "$InstallPath\agent.py"
            & $nssmPath set CyberSentinelDLP AppDirectory $InstallPath
            & $nssmPath set CyberSentinelDLP DisplayName "CyberSentinel DLP Agent"
            & $nssmPath set CyberSentinelDLP Description "Data Loss Prevention endpoint protection agent"
            & $nssmPath set CyberSentinelDLP Start SERVICE_AUTO_START
            & $nssmPath set CyberSentinelDLP AppStdout "$InstallPath\service.log"
            & $nssmPath set CyberSentinelDLP AppStderr "$InstallPath\service-error.log"
            & $nssmPath set CyberSentinelDLP AppRestartDelay 30000

            # Start service
            & $nssmPath start CyberSentinelDLP

            Start-Sleep -Seconds 2

            # Check service status
            $serviceStatus = Get-Service -Name "CyberSentinelDLP" -ErrorAction SilentlyContinue
            if ($serviceStatus -and $serviceStatus.Status -eq "Running") {
                Write-ColorOutput "[+] Service installed and started successfully!" $SuccessColor
            } else {
                Write-ColorOutput "[!] Service installed but may not be running. Check: Get-Service CyberSentinelDLP" $WarningColor
            }
        } catch {
            Write-ColorOutput "[-] Failed to install service: $_" $ErrorColor
            Write-ColorOutput "[!] You can run the agent manually: python `"$InstallPath\agent.py`"" $InfoColor
        }
    }
} else {
    Write-ColorOutput "`n[*] Service installation skipped" $InfoColor
    Write-ColorOutput "[*] You can run the agent manually: python `"$InstallPath\agent.py`"" $InfoColor
}

# Installation complete
Write-ColorOutput @"

╔══════════════════════════════════════════════════════════╗
║          INSTALLATION COMPLETED SUCCESSFULLY!            ║
╚══════════════════════════════════════════════════════════╝

Installation Details:
  • Location:      $InstallPath
  • Agent ID:      $AgentID
  • Agent Name:    $AgentName
  • Server URL:    $ServerURL
  • Service:       $(if ($AsService) { "Installed and Running" } else { "Not Installed" })

"@ $SuccessColor

if ($AsService) {
    Write-ColorOutput "The agent is now running as a Windows Service!" $SuccessColor
    Write-ColorOutput @"

Service Management Commands:
  • Check status:  Get-Service CyberSentinelDLP
  • Stop service:  Stop-Service CyberSentinelDLP
  • Start service: Start-Service CyberSentinelDLP
  • View logs:     Get-Content "$InstallPath\cybersentinel_agent.log" -Tail 50

"@ $InfoColor
} else {
    Write-ColorOutput "To run the agent manually:" $InfoColor
    Write-ColorOutput "  cd `"$InstallPath`"" $InfoColor
    Write-ColorOutput "  python agent.py" $InfoColor
    Write-ColorOutput "`nTo install as service later, run:" $InfoColor
    Write-ColorOutput "  .\nssm.exe install CyberSentinelDLP python `"$InstallPath\agent.py`"" $InfoColor
}

Write-ColorOutput @"

Next Steps:
  1. Verify agent appears in dashboard: $($ServerURL -replace '/api/v1', ':3000')
  2. Check agent status is 'Online'
  3. Test detection by creating a file with sensitive data
  4. Configure DLP policies in the dashboard

Documentation: https://github.com/effaaykhan/cybersentinel-windows-agent

"@ $InfoColor

Write-ColorOutput "Thank you for using CyberSentinel DLP!" $SuccessColor
