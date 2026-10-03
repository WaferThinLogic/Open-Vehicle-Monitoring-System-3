# Hyundai Ioniq 5 / Kia EV6 - CAN Write Commands

## Overview

This document describes the CAN write commands available for the Hyundai Ioniq 5 and Kia EV6. These commands allow remote control of various vehicle functions through the OVMS module.

## ⚠️ Safety Requirements

**ALL CAN WRITE COMMANDS REQUIRE:**

1. **Intercept Harness Mode** - Set `xiq.con_obd=0`
   - The Ioniq 5/EV6 have CAN gateways that block write commands on the standard OBD port
   - Attempting to send commands in OBD mode will be blocked and logged as an error

2. **CAN Write Enabled** - Set `xiq.canwrite=1`
   - This is a separate safety flag that must be explicitly enabled
   - Disabled by default to prevent accidental command transmission

3. **Vehicle in Park** - Most commands require `iq_shift_status == Park`
   - Commands will be rejected if the vehicle is not in Park
   - This is a critical safety feature

### Checking Requirements

```bash
# Check current connection mode (1=OBD, 0=Intercept)
OVMS> config get xiq.con_obd

# Check if CAN write is enabled
OVMS> config get xiq.canwrite

# Set to intercept harness mode (REQUIRED for commands)
OVMS> config set xiq.con_obd 0

# Enable CAN writes
OVMS> config set xiq.canwrite 1
```

## Available Commands

### 1. Door Lock/Unlock
- **Usage:** `OVMS> lock <pin>` / `OVMS> unlock <pin>`
- **Command:** `Send_IGMP_Command(0xbc, 0x11/0x10, 0x03)`
- **ECU:** IGMP (0x770)
- **Safety:** PIN required, Park required

### 2. Trunk Release
- **Command:** `Send_IGMP_Command(0xbc, 0x09, 0x03)`
- **ECU:** IGMP (0x770)
- **Safety:** PIN required, Park required

### 3. Turn Indicators
- **Functions:** `LeftIndicator(bool)`, `RightIndicator(bool)`
- **Command:** `Send_IGMP_Command(0xbc, 0x15/0x16, 0x03/0x00)`
- **ECU:** IGMP (0x770)

### 4. Rear Defogger
- **Function:** `RearDefogger(bool)`
- **Command:** `Send_IGMP_Command(0xbc, 0x0c, 0x03/0x00)`
- **ECU:** IGMP (0x770)

### 5. Mirror Fold/Unfold
- **Function:** `FoldMirrors(bool)`
- **Command:** `Send_BCM_Command(0xb0, 0x5b/0x5c, 0x03)`
- **ECU:** BCM (0x7A0)

### 6-9. Vehicle Settings (via CAN1)
- **Head Light Delay:** `SendCanMessageTriple(0x014, 0,0, 0x08/0x04, ...)`
- **One-Touch Turn Signal:** 0=off, 1=3, 2=5, 3=7 blinks
- **Auto Door Unlock:** 1=Off, 2=Off, 3=Shift-P, 4=Driver
- **Auto Door Lock:** 0=Off, 1=Speed, 2=Shift

### 10. Remote Climate Control (NEW)
- **Start Climate:** `StartClimate(PIN, duration_minutes)`
  - **ECU:** AirCon (0x7b3)
  - **Command:** `Send_AirCon_Command(0x01, 0x00, 0x00, 0x00)` (start)
  - **Safety:** PIN required, Park required, intercept harness required
  - **Duration:** Optional auto-shutoff in minutes (0 = manual stop)
  - **Note:** Uses "spamming" pattern (3x repeated messages) for reliability
  
- **Stop Climate:** `StopClimate(PIN)`
  - **ECU:** AirCon (0x7b3)
  - **Command:** `Send_AirCon_Command(0x00, 0x00, 0x00, 0x00)` (stop)
  - **Safety:** PIN required, Park required, intercept harness required

- **Set Temperature:** `SetClimateTemperature(celsius)` (NOT YET IMPLEMENTED)
  - Placeholder for future temperature control (15-30°C)
  - Requires additional reverse engineering

**Climate Control Usage:**
```bash
# Start climate control (runs until stopped)
OVMS> xhiq climate start <PIN>

# Start climate control with 15-minute auto-shutoff
OVMS> xhiq climate start <PIN> 15

# Stop climate control
OVMS> xhiq climate stop <PIN>
```

**Technical Details:**
- Sends Extended Diagnostic Session request to AirCon ECU
- Uses "spamming" technique (3 messages @ 100ms intervals) for reliability
- Auto-shutoff timer checked every 10 seconds via Ticker10
- State tracked: `m_climate_active`, `m_climate_duration_minutes`, `m_climate_start_time`

See [CLIMATE_CONTROL.md](CLIMATE_CONTROL.md) for complete documentation.

## Error Messages

```
E v-ioniq5: CAN write blocked: not in intercept harness mode
E v-ioniq5: CAN write blocked: canwrite not enabled
W v-ioniq5: Door lock command blocked: vehicle not in Park
E v-ioniq5: Door lock command failed: invalid PIN
```

## Testing

```bash
# Enable command mode
OVMS> config set xiq.con_obd 0
OVMS> config set xiq.canwrite 1

# Verify
OVMS> config get xiq.con_obd  # Should return 0

# Test safely with indicators first
```

## ⚠️ Disclaimer

USE AT YOUR OWN RISK. These commands can affect vehicle operation. Always test in a safe environment with proper hardware (intercept harness).
