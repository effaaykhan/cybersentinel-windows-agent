# Windows Agent Troubleshooting Guide

## Issue 1: 401 Error - Failed to Register Agent

**Error:**
```
Failed to register agent: 401
```

**Causes & Solutions:**

### Solution 1: Check Server URL
The 401 error usually means the server URL is incorrect or the endpoint requires authentication.

1. **Verify your `agent_config.json` server URL:**
   ```powershell
   type agent_config.json | findstr server_url
   ```

2. **Make sure the URL format is correct:**
   ```json
   {
     "server_url": "http://YOUR-SERVER-IP:8000/api/v1"
   }
   ```
   - Use `http://` (not `https://`) unless your server uses SSL
   - Include the port number (`:8000`)
   - Include `/api/v1` at the end
   - **No trailing slash** after `/api/v1`

3. **Test the server URL manually:**
   ```powershell
   # Test if server is reachable
   Test-NetConnection -ComputerName YOUR-SERVER-IP -Port 8000
   
   # Test the registration endpoint
   Invoke-WebRequest -Uri "http://YOUR-SERVER-IP:8000/api/v1/agents/" -Method POST -ContentType "application/json" -Body '{"name":"test","os":"windows","ip_address":"127.0.0.1"}'
   ```

### Solution 2: Check Firewall
Windows Firewall might be blocking the connection:

```powershell
# Allow Python through firewall
New-NetFirewallRule -DisplayName "CyberSentinel Agent" `
    -Direction Outbound -Program (Get-Command python).Source `
    -Action Allow
```

### Solution 3: Verify Server is Running
Make sure the CyberSentinel server is running and accessible:
- Check if you can access the dashboard: `http://YOUR-SERVER-IP:3000`
- Check if the API is accessible: `http://YOUR-SERVER-IP:8000/api/v1/docs`

---

## Issue 2: WMI COM Error - USB Monitoring Failed

**Error:**
```
wmi.x_wmi_uninitialised_thread: <x_wmi: WMI returned a syntax error: you're probably running inside a thread without first calling pythoncom.CoInitialize[Ex]>
```

**Cause:** Your agent code is outdated. The fix for this issue exists in the latest version.

**Solution: Update Your Agent Code**

The agent file at `C:\Program Files\CyberSentinel\agent.py` needs to be updated. Here's what to do:

### Option 1: Download Latest Code (Recommended)
1. Download the latest agent code from the repository
2. Replace your `agent.py` file with the new version
3. Restart the agent

### Option 2: Manual Fix
If you can't update the code, add this fix to your `agent.py`:

1. **Find the `monitor_usb` method** (around line 275)

2. **Add COM initialization at the start:**
   ```python
   def monitor_usb(self):
       """Monitor USB device connections and disconnections"""
       logger.info("USB monitoring started")
       # Initialize COM for this thread (required for WMI)
       pythoncom.CoInitialize()
       try:
           c = wmi.WMI()
       except Exception as e:
           logger.error(f"Failed to initialize WMI: {e}")
           pythoncom.CoUninitialize()
           return
       
       # ... rest of the code ...
   ```

3. **Add cleanup at the end of the method:**
   At the end of the `while self.running:` loop, add:
   ```python
   finally:
       pythoncom.CoUninitialize()
   ```

### Option 3: Disable USB Monitoring Temporarily
If you don't need USB monitoring right now, you can disable it:

Edit `agent_config.json`:
```json
{
  "monitoring": {
    "usb_devices": false
  }
}
```

---

## Issue 3: Warnings (Non-Critical)

These warnings can be ignored - they don't affect functionality:

1. **Wireshark warning:** `WARNING: Wireshark is installed, but cannot read manuf !`
   - This is just a warning from the scapy library
   - Doesn't affect agent functionality

2. **CryptographyDeprecationWarning:** About TripleDES
   - This is a deprecation warning from a library
   - Will be fixed in future library updates
   - Doesn't affect current functionality

---

## Quick Fix Checklist

If you're getting errors, try these in order:

1. **Check server URL:**
   ```powershell
   type agent_config.json
   ```
   Verify `server_url` is correct format: `http://IP:8000/api/v1`

2. **Test server connectivity:**
   ```powershell
   Test-NetConnection -ComputerName YOUR-SERVER-IP -Port 8000
   ```

3. **Update agent code:**
   - Download latest version from repository
   - Replace your `agent.py` file

4. **Reinstall pywin32 (if USB monitoring fails):**
   ```powershell
   pip uninstall pywin32
   pip install pywin32
   python C:\Python311\Scripts\pywin32_postinstall.py -install
   ```

5. **Check firewall:**
   ```powershell
   Get-NetFirewallRule | Where-Object {$_.DisplayName -like "*Python*"}
   ```

6. **View detailed logs:**
   ```powershell
   type cybersentinel_agent.log
   ```

---

## Still Having Issues?

If the agent still doesn't work:

1. **Check the full error message** in `cybersentinel_agent.log`
2. **Verify Python version:** `python --version` (should be 3.8+)
3. **Verify dependencies:** `pip list | findstr "requests watchdog pywin32 WMI"`
4. **Test with minimal config** - disable optional features:
   ```json
   {
     "server_url": "http://YOUR-SERVER-IP:8000/api/v1",
     "agent_id": "TEST-001",
     "agent_name": "Test",
     "monitoring": {
       "file_system": true,
       "clipboard": false,
       "usb_devices": false,
       "network_uploads": false,
       "browser_file_access": false
     }
   }
   ```

