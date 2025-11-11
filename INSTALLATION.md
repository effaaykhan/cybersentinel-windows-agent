# CyberSentinel DLP - Windows Agent Installation Guide

## Quick Start

### Step 1: Install Python (if not already installed)

1. Download Python 3.8+ from https://www.python.org/downloads/
2. **Important:** Check "Add Python to PATH" during installation
3. Restart your computer after installation

Verify installation:
```powershell
python --version
```

### Step 2: Install Dependencies

Open PowerShell in the agent directory:

```powershell
cd cybersentinel-windows-agent
pip install -r requirements.txt
```

**Important:** After installing, run the pywin32 post-install script:
```powershell
python C:\Python3X\Scripts\pywin32_postinstall.py -install
```

(Replace `Python3X` with your Python version folder, e.g., `Python311`)

### Step 3: Configure the Agent

Edit `agent_config.json`:

```powershell
notepad agent_config.json
```

Update these important fields:
- `server_url`: Your server IP (e.g., `http://172.23.19.78:8000/api/v1`)
- `agent_id`: Unique ID for this agent (e.g., `WIN-001`)
- `agent_name`: Name to identify this machine
- `monitored_paths`: Directories to monitor (use double backslashes: `C:\\Users\\Documents`)

Example configuration:
```json
{
  "server_url": "http://172.23.19.78:8000/api/v1",
  "agent_id": "WIN-001",
  "agent_name": "My-Windows-PC",
  "monitoring": {
    "monitored_paths": [
      "C:\\Users\\YourUsername\\Documents",
      "C:\\Users\\YourUsername\\Desktop"
    ]
  }
}
```

### Step 4: Run the Agent

```powershell
python agent.py
```

The agent will start monitoring and send events to the server. Check the log file:
```powershell
type cybersentinel_agent.log
```

## Common Issues

### Issue: "pip is not recognized"
**Fix:** Use `python -m pip` instead:
```powershell
python -m pip install -r requirements.txt
```

### Issue: "Failed to initialize WMI"
**Fix:** Run the pywin32 post-install script:
```powershell
python C:\Python3X\Scripts\pywin32_postinstall.py -install
```
Then restart PowerShell and try again.

### Issue: "ModuleNotFoundError: No module named 'win32clipboard'"
**Fix:** 
```powershell
pip uninstall pywin32
pip install pywin32
python C:\Python3X\Scripts\pywin32_postinstall.py -install
```

### Issue: Agent can't connect to server
**Fix:**
1. Check server URL in `agent_config.json`
2. Test connectivity: `Test-NetConnection -ComputerName YOUR-SERVER-IP -Port 8000`
3. Check Windows Firewall (may need to allow Python through firewall)

### Issue: "Path does not exist" errors
**Fix:** Replace `%USERNAME%` with your actual username in `agent_config.json`, or use full paths like `C:\\Users\\YourActualUsername\\Documents`

### Issue: Clipboard/USB monitoring not working
**Fix:**
1. Ensure pywin32 post-install script ran successfully
2. Don't run agent as Administrator (run as your regular user)
3. Check logs for specific errors: `type cybersentinel_agent.log`

## Running as Windows Service (Optional)

### Using NSSM (Recommended)

1. Download NSSM from https://nssm.cc/download
2. Extract to `C:\nssm`
3. Build executable (optional but recommended):
```powershell
pip install pyinstaller
pyinstaller --onefile --name "CyberSentinelAgent" agent.py
```
4. Install service:
```powershell
cd C:\nssm\win64
.\nssm install CyberSentinelDLP "C:\Path\To\CyberSentinelAgent.exe"
.\nssm set CyberSentinelDLP AppDirectory "C:\Path\To\cybersentinel-windows-agent"
.\nssm start CyberSentinelDLP
```

## Verification

After installation:
- [ ] Agent is running (check Task Manager for Python process)
- [ ] Agent appears in dashboard
- [ ] Logs show "Agent registered successfully"
- [ ] Test file creation triggers an event in dashboard
- [ ] Clipboard copy triggers event (if enabled)
- [ ] USB device connection triggers event (if enabled)

## Quick Reference

**Start agent:**
```powershell
python agent.py
```

**View logs:**
```powershell
type cybersentinel_agent.log
Get-Content cybersentinel_agent.log -Wait -Tail 50
```

**Check if running:**
```powershell
Get-Process python
```

**Stop agent:**
Press `Ctrl+C` or close the PowerShell window

