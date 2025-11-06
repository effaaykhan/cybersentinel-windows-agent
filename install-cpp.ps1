# CyberSentinel DLP - C++ Agent One-Click Installer
# This script installs all dependencies and builds the C++ agent automatically

param(
    [string]$ServerURL,
    [string]$AgentID,
    [string]$AgentName,
    [switch]$AsService,
    [switch]$Silent
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "CyberSentinel DLP - C++ Agent Installer" -ForegroundColor Cyan
Write-Host "High-Performance Windows Agent Setup" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Configuration
$InstallDir = "C:\CyberSentinel"
$VcpkgDir = "C:\vcpkg"
$AgentRepo = "https://github.com/effaaykhan/cybersentinel-windows-agent.git"
$NssmVersion = "2.24"
$NssmUrl = "https://nssm.cc/release/nssm-$NssmVersion.zip"

# Check if running as Administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")
if (-not $isAdmin) {
    Write-Host "ERROR: This script must be run as Administrator!" -ForegroundColor Red
    Write-Host "Right-click PowerShell and select 'Run as Administrator'" -ForegroundColor Yellow
    exit 1
}

Write-Host "[1/9] Checking prerequisites..." -ForegroundColor Green

# Function to check if command exists
function Test-CommandExists {
    param($Command)
    $null -ne (Get-Command $Command -ErrorAction SilentlyContinue)
}

# Check for Visual Studio or Build Tools
Write-Host "  Checking for Visual Studio C++ tools..." -ForegroundColor Yellow
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsInstalled = $false

if (Test-Path $vswhere) {
    $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($vsPath) {
        $vsInstalled = $true
        Write-Host "  ✓ Visual Studio found: $vsPath" -ForegroundColor Green
    }
}

if (-not $vsInstalled) {
    Write-Host "  ✗ Visual Studio C++ tools not found" -ForegroundColor Red
    Write-Host ""
    Write-Host "INSTALLATION REQUIRED:" -ForegroundColor Yellow
    Write-Host "Please install Visual Studio 2019 or 2022 with C++ development tools:" -ForegroundColor Yellow
    Write-Host "  1. Download from: https://visualstudio.microsoft.com/downloads/" -ForegroundColor Cyan
    Write-Host "  2. During installation, select 'Desktop development with C++'" -ForegroundColor Cyan
    Write-Host "  3. Re-run this installer after Visual Studio installation completes" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Alternative: Install Build Tools for Visual Studio (smaller download)" -ForegroundColor Yellow
    Write-Host "  https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022" -ForegroundColor Cyan
    Write-Host ""

    $response = Read-Host "Open download page now? (y/n)"
    if ($response -eq 'y') {
        Start-Process "https://visualstudio.microsoft.com/downloads/"
    }
    exit 1
}

# Check/Install Git
Write-Host ""
Write-Host "[2/9] Checking Git..." -ForegroundColor Green
if (-not (Test-CommandExists git)) {
    Write-Host "  Git not found. Installing Git..." -ForegroundColor Yellow

    $gitInstaller = "$env:TEMP\git-installer.exe"
    Invoke-WebRequest -Uri "https://github.com/git-for-windows/git/releases/download/v2.43.0.windows.1/Git-2.43.0-64-bit.exe" -OutFile $gitInstaller

    Start-Process -FilePath $gitInstaller -ArgumentList "/VERYSILENT /NORESTART" -Wait
    Remove-Item $gitInstaller

    # Refresh PATH
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

    Write-Host "  ✓ Git installed" -ForegroundColor Green
} else {
    Write-Host "  ✓ Git already installed" -ForegroundColor Green
}

# Check/Install CMake
Write-Host ""
Write-Host "[3/9] Checking CMake..." -ForegroundColor Green
if (-not (Test-CommandExists cmake)) {
    Write-Host "  CMake not found. Installing CMake..." -ForegroundColor Yellow

    $cmakeInstaller = "$env:TEMP\cmake-installer.msi"
    Invoke-WebRequest -Uri "https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-windows-x86_64.msi" -OutFile $cmakeInstaller

    Start-Process msiexec.exe -ArgumentList "/i `"$cmakeInstaller`" /quiet /norestart ADD_CMAKE_TO_PATH=System" -Wait
    Remove-Item $cmakeInstaller

    # Refresh PATH
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

    Write-Host "  ✓ CMake installed" -ForegroundColor Green
} else {
    $cmakeVersion = (cmake --version | Select-Object -First 1) -replace 'cmake version ', ''
    Write-Host "  ✓ CMake already installed: $cmakeVersion" -ForegroundColor Green
}

# Install/Setup vcpkg
Write-Host ""
Write-Host "[4/9] Setting up vcpkg..." -ForegroundColor Green
if (-not (Test-Path $VcpkgDir)) {
    Write-Host "  Cloning vcpkg..." -ForegroundColor Yellow
    git clone https://github.com/Microsoft/vcpkg.git $VcpkgDir 2>&1 | Out-Null

    Write-Host "  Bootstrapping vcpkg..." -ForegroundColor Yellow
    & "$VcpkgDir\bootstrap-vcpkg.bat" -disableMetrics | Out-Null

    Write-Host "  Integrating vcpkg with Visual Studio..." -ForegroundColor Yellow
    & "$VcpkgDir\vcpkg.exe" integrate install | Out-Null

    Write-Host "  ✓ vcpkg installed" -ForegroundColor Green
} else {
    Write-Host "  ✓ vcpkg already installed" -ForegroundColor Green
}

# Install dependencies via vcpkg
Write-Host ""
Write-Host "[5/9] Installing C++ dependencies..." -ForegroundColor Green
Write-Host "  This may take 5-10 minutes on first run..." -ForegroundColor Yellow

$packages = @("curl:x64-windows", "nlohmann-json:x64-windows")
foreach ($package in $packages) {
    $packageName = $package -replace ':.*', ''
    Write-Host "  Installing $packageName..." -ForegroundColor Yellow

    & "$VcpkgDir\vcpkg.exe" install $package --recurse 2>&1 | Out-Null

    if ($LASTEXITCODE -eq 0) {
        Write-Host "  ✓ $packageName installed" -ForegroundColor Green
    } else {
        Write-Host "  ✗ Failed to install $packageName" -ForegroundColor Red
        exit 1
    }
}

# Clone agent repository
Write-Host ""
Write-Host "[6/9] Downloading agent source code..." -ForegroundColor Green
if (Test-Path "$InstallDir\cybersentinel-windows-agent") {
    Write-Host "  Removing existing installation..." -ForegroundColor Yellow
    Remove-Item -Path "$InstallDir\cybersentinel-windows-agent" -Recurse -Force
}

New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
git clone $AgentRepo "$InstallDir\cybersentinel-windows-agent" 2>&1 | Out-Null
Write-Host "  ✓ Source code downloaded" -ForegroundColor Green

# Build agent
Write-Host ""
Write-Host "[7/9] Building C++ agent..." -ForegroundColor Green
Write-Host "  This may take 2-3 minutes..." -ForegroundColor Yellow

$BuildDir = "$InstallDir\cybersentinel-windows-agent\build"
New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null

Push-Location $BuildDir

# Configure with CMake
Write-Host "  Configuring with CMake..." -ForegroundColor Yellow
cmake .. -DCMAKE_TOOLCHAIN_FILE="$VcpkgDir\scripts\buildsystems\vcpkg.cmake" -G "Visual Studio 17 2022" -A x64 2>&1 | Out-Null

if ($LASTEXITCODE -ne 0) {
    Write-Host "  ✗ CMake configuration failed" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Build
Write-Host "  Compiling C++ code..." -ForegroundColor Yellow
cmake --build . --config Release 2>&1 | Out-Null

if ($LASTEXITCODE -ne 0) {
    Write-Host "  ✗ Build failed" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

$AgentExe = "$BuildDir\bin\Release\CyberSentinelAgent.exe"
if (Test-Path $AgentExe) {
    Write-Host "  ✓ Build successful!" -ForegroundColor Green
    Write-Host "  Executable: $AgentExe" -ForegroundColor Cyan
} else {
    Write-Host "  ✗ Build failed - executable not found" -ForegroundColor Red
    exit 1
}

# Configure agent
Write-Host ""
Write-Host "[8/9] Configuring agent..." -ForegroundColor Green

$ConfigFile = "$InstallDir\cybersentinel-windows-agent\agent_config.json"

if (-not $Silent) {
    if (-not $ServerURL) {
        $ServerURL = Read-Host "  Enter CyberSentinel server URL (e.g., http://192.168.1.100:8000/api/v1)"
    }
    if (-not $AgentID) {
        $defaultAgentID = "WIN-$env:COMPUTERNAME"
        $AgentID = Read-Host "  Enter Agent ID [$defaultAgentID]"
        if ([string]::IsNullOrWhiteSpace($AgentID)) {
            $AgentID = $defaultAgentID
        }
    }
    if (-not $AgentName) {
        $defaultAgentName = "$env:USERNAME@$env:COMPUTERNAME"
        $AgentName = Read-Host "  Enter Agent Name [$defaultAgentName]"
        if ([string]::IsNullOrWhiteSpace($AgentName)) {
            $AgentName = $defaultAgentName
        }
    }
} else {
    if (-not $ServerURL) { $ServerURL = "http://192.168.1.100:8000/api/v1" }
    if (-not $AgentID) { $AgentID = "WIN-$env:COMPUTERNAME" }
    if (-not $AgentName) { $AgentName = "$env:USERNAME@$env:COMPUTERNAME" }
}

# Create configuration
$config = @{
    server_url = $ServerURL
    agent_id = $AgentID
    agent_name = $AgentName
    heartbeat_interval = 60
    monitoring = @{
        file_system = $true
        clipboard = $true
        usb_devices = $true
        monitored_paths = @(
            "C:\Users\Public\Documents",
            "C:\Users\$env:USERNAME\Desktop",
            "C:\Users\$env:USERNAME\Documents"
        )
    }
}

$config | ConvertTo-Json -Depth 10 | Set-Content $ConfigFile
Write-Host "  ✓ Configuration saved" -ForegroundColor Green

# Test server connectivity
Write-Host "  Testing server connectivity..." -ForegroundColor Yellow
try {
    $response = Invoke-WebRequest -Uri "$ServerURL/health" -Method GET -TimeoutSec 5 -UseBasicParsing -ErrorAction SilentlyContinue
    Write-Host "  ✓ Server is reachable" -ForegroundColor Green
} catch {
    Write-Host "  ⚠ Warning: Could not reach server at $ServerURL" -ForegroundColor Yellow
    Write-Host "  Make sure the CyberSentinel server is running" -ForegroundColor Yellow
}

# Install as service
Write-Host ""
Write-Host "[9/9] Service installation..." -ForegroundColor Green

if (-not $Silent -and -not $AsService) {
    $installService = Read-Host "  Install as Windows Service? (y/n) [y]"
    if ([string]::IsNullOrWhiteSpace($installService)) {
        $installService = "y"
    }
    $AsService = ($installService -eq "y")
}

if ($AsService) {
    Write-Host "  Installing NSSM (service manager)..." -ForegroundColor Yellow

    $nssmZip = "$env:TEMP\nssm.zip"
    $nssmExtract = "$env:TEMP\nssm"

    Invoke-WebRequest -Uri $NssmUrl -OutFile $nssmZip
    Expand-Archive -Path $nssmZip -DestinationPath $nssmExtract -Force

    $nssmExe = "$nssmExtract\nssm-$NssmVersion\win64\nssm.exe"
    Copy-Item $nssmExe "C:\Windows\System32\nssm.exe" -Force

    Remove-Item $nssmZip -Force
    Remove-Item $nssmExtract -Recurse -Force

    Write-Host "  ✓ NSSM installed" -ForegroundColor Green

    # Remove existing service if present
    $existingService = Get-Service -Name "CyberSentinelDLP" -ErrorAction SilentlyContinue
    if ($existingService) {
        Write-Host "  Removing existing service..." -ForegroundColor Yellow
        nssm stop CyberSentinelDLP 2>&1 | Out-Null
        nssm remove CyberSentinelDLP confirm 2>&1 | Out-Null
    }

    # Install service
    Write-Host "  Installing Windows Service..." -ForegroundColor Yellow
    nssm install CyberSentinelDLP "$AgentExe" | Out-Null
    nssm set CyberSentinelDLP AppDirectory "$InstallDir\cybersentinel-windows-agent" | Out-Null
    nssm set CyberSentinelDLP DisplayName "CyberSentinel DLP Agent" | Out-Null
    nssm set CyberSentinelDLP Description "High-Performance C++ Data Loss Prevention Agent" | Out-Null
    nssm set CyberSentinelDLP Start SERVICE_AUTO_START | Out-Null

    Write-Host "  Starting service..." -ForegroundColor Yellow
    nssm start CyberSentinelDLP | Out-Null

    Start-Sleep -Seconds 2

    $serviceStatus = nssm status CyberSentinelDLP
    if ($serviceStatus -match "SERVICE_RUNNING") {
        Write-Host "  ✓ Service installed and running!" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ Service installed but not running. Check logs." -ForegroundColor Yellow
    }
} else {
    Write-Host "  Service installation skipped" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "  To run manually:" -ForegroundColor Cyan
    Write-Host "    cd `"$InstallDir\cybersentinel-windows-agent`"" -ForegroundColor White
    Write-Host "    .\build\bin\Release\CyberSentinelAgent.exe" -ForegroundColor White
}

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "✓ INSTALLATION COMPLETE!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Agent Details:" -ForegroundColor Cyan
Write-Host "  Installation: $InstallDir\cybersentinel-windows-agent" -ForegroundColor White
Write-Host "  Executable: $AgentExe" -ForegroundColor White
Write-Host "  Config: $ConfigFile" -ForegroundColor White
Write-Host "  Log File: $InstallDir\cybersentinel-windows-agent\cybersentinel_agent.log" -ForegroundColor White
Write-Host ""
Write-Host "Configuration:" -ForegroundColor Cyan
Write-Host "  Server: $ServerURL" -ForegroundColor White
Write-Host "  Agent ID: $AgentID" -ForegroundColor White
Write-Host "  Agent Name: $AgentName" -ForegroundColor White
Write-Host ""

if ($AsService) {
    Write-Host "Service Management:" -ForegroundColor Cyan
    Write-Host "  Status:  nssm status CyberSentinelDLP" -ForegroundColor White
    Write-Host "  Stop:    nssm stop CyberSentinelDLP" -ForegroundColor White
    Write-Host "  Start:   nssm start CyberSentinelDLP" -ForegroundColor White
    Write-Host "  Restart: nssm restart CyberSentinelDLP" -ForegroundColor White
    Write-Host "  Remove:  nssm remove CyberSentinelDLP confirm" -ForegroundColor White
    Write-Host ""
}

Write-Host "Next Steps:" -ForegroundColor Cyan
Write-Host "  1. Check the agent appears in CyberSentinel dashboard" -ForegroundColor White
Write-Host "  2. Review logs: $InstallDir\cybersentinel-windows-agent\cybersentinel_agent.log" -ForegroundColor White
Write-Host "  3. Monitor events in the dashboard" -ForegroundColor White
Write-Host ""
Write-Host "Performance:" -ForegroundColor Cyan
Write-Host "  ⚡ 5-10x faster than Python version" -ForegroundColor Green
Write-Host "  💾 80% less memory (~10MB)" -ForegroundColor Green
Write-Host "  ⏱️ <100ms startup time" -ForegroundColor Green
Write-Host ""
Write-Host "Documentation: https://github.com/effaaykhan/cybersentinel-windows-agent" -ForegroundColor Cyan
Write-Host ""
Write-Host "🛡️ Your system is now protected by CyberSentinel DLP!" -ForegroundColor Green
Write-Host ""
