# Windows Agent Testing Guide

## Testing Clipboard Monitoring

### How to Test:
1. **Copy sensitive data to clipboard:**
   - Open Notepad or any text editor
   - Type or paste one of these:
     - SSN: `123-45-6789` (should trigger CRITICAL event)
     - Email: `test@example.com` (should trigger MEDIUM event)
     - Credit Card: `1234 5678 9012 3456` (should trigger CRITICAL event)
     - API Key: `api_key=abc123xyz` (should trigger HIGH event)
   - Select the text and press `Ctrl+C` to copy

2. **Check for events:**
   - Wait 2-3 seconds (agent checks clipboard every 2 seconds)
   - Check dashboard at `http://172.23.19.78:3000/dashboard/events`
   - Look for events with:
     - Event type: `clipboard`
     - Event subtype: `clipboard_copy`
     - Severity matching the sensitive data type

3. **Check agent logs:**
   ```powershell
   Get-Content '\\wsl.localhost\Ubuntu\home\vansh\Code\cybersentinel-windows-agent\cybersentinel_agent.log' | Select-String "Clipboard"
   ```

### Expected Behavior:
- Agent detects clipboard content every 2 seconds
- Only triggers event if content contains sensitive data (SSN, email, API key, etc.)
- Event should appear in dashboard within 2-3 seconds
- Log should show: `Clipboard event: ['SSN']` or similar

---

## Testing USB Monitoring

### How to Test:
1. **Connect a USB device:**
   - Plug in any USB device (flash drive, mouse, keyboard, phone, etc.)
   - Wait 5-10 seconds (agent checks USB devices every 5 seconds)

2. **Check for events:**
   - Check dashboard at `http://172.23.19.78:3000/dashboard/events`
   - Look for events with:
     - Event type: `usb`
     - Event subtype: `usb_connected`
     - Severity: `medium`
     - Description: `USB device connected: [device name]`

3. **Check agent logs:**
   ```powershell
   Get-Content '\\wsl.localhost\Ubuntu\home\vansh\Code\cybersentinel-windows-agent\cybersentinel_agent.log' | Select-String "USB"
   ```

### Expected Behavior:
- Agent detects new USB device connections
- Creates event for each new device (tracks known devices to avoid duplicates)
- Event should appear in dashboard within 5-10 seconds
- Log should show: `USB device connected: [device name]`

### Troubleshooting:
- If USB monitoring shows errors in logs, it may need admin privileges
- Some USB devices may not appear if WMI query fails
- Try connecting different USB devices (flash drives work best)

---

## Quick Test Commands

### Test Clipboard (PowerShell):
```powershell
# Copy SSN to clipboard
"123-45-6789" | Set-Clipboard

# Copy email to clipboard  
"test@example.com" | Set-Clipboard

# Copy API key to clipboard
"api_key=secret123" | Set-Clipboard
```

### Check Events via API:
```bash
# Get auth token
TOKEN=$(curl -s -X POST http://172.23.19.78:8000/api/v1/auth/login \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "username=admin@cybersentinel.local&password=ChangeMe123!" \
  | python3 -c "import sys, json; print(json.load(sys.stdin)['access_token'])")

# Check clipboard events
curl -s http://172.23.19.78:8000/api/v1/events/ \
  -H "Authorization: Bearer $TOKEN" \
  | python3 -m json.tool | grep -A 5 "clipboard"

# Check USB events
curl -s http://172.23.19.78:8000/api/v1/events/ \
  -H "Authorization: Bearer $TOKEN" \
  | python3 -m json.tool | grep -A 5 "usb"
```

---

## Monitoring Intervals

- **Clipboard:** Checks every **2 seconds**
- **USB:** Checks every **5 seconds**
- **File System:** Real-time (immediate detection)
- **Heartbeat:** Every **60 seconds**

