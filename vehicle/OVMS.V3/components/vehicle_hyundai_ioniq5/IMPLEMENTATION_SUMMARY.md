# Remote Climate Control Implementation Summary

## Overview
Remote climate control (preconditioning) for Hyundai Ioniq 5 / Kia EV6 OVMS module.

## Files Modified

### 1. vehicle_hyundai_ioniq5.h
- Added climate state variables: `m_climate_active`, `m_climate_duration_minutes`, `m_climate_start_time`
- Added methods: `StartClimate()`, `StopClimate()`, `SetClimateTemperature()`, `Send_AirCon_Command()`, `SendClimateCommand()`, `SendClimateSpamMessages()`

### 2. vehicle_hyundai_ioniq5.cpp
- Constructor: Initialize climate variables
- Ticker10(): Auto-shutoff timer check
- Implemented StartClimate(), StopClimate(), SetClimateTemperature()

### 3. hif_can_send.cpp
- Send_AirCon_Command(): HVAC ECU communication (0x7b3)
- SendClimateCommand(): High-level command with safety checks
- SendClimateSpamMessages(): 3x repeated messages for reliability

### 4. CAN_COMMANDS.md
- Added Section 10: Remote Climate Control documentation

### 5. CLIMATE_CONTROL.md (NEW)
- Complete user guide with requirements, commands, troubleshooting

## Safety Features
1. Intercept harness required (xiq.con_obd=0)
2. CAN write enabled (xiq.canwrite=1)
3. PIN authentication required
4. Vehicle must be in Park
5. Optional auto-shutoff timer
6. State tracking prevents duplicates

## Usage
```bash
# Start climate (manual stop)
xhiq climate start <PIN>

# Start with 15-min auto-shutoff
xhiq climate start <PIN> 15

# Stop climate
xhiq climate stop <PIN>
```

## Technical Details
- ECU: AirCon 0x7b3 → 0x7bb
- Session: Extended Diagnostic (0x03)
- Service: 0x2F (IO Control By ID)
- Spam pattern: 3 messages @ 100ms intervals

## Known Limitations
- Temperature control not implemented (placeholder only)
- Fan speed control not available
- Seat climate/steering heater not implemented
- May require I-CAN for full functionality

## Testing
1. Verify intercept harness installed
2. Set xiq.con_obd=0, xiq.canwrite=1
3. Test with indicators first
4. Start with short climate duration
5. Monitor vehicle behavior

## Status
✅ Implementation complete
⏳ Ready for testing
⏳ CLI command registration pending (see CLIMATE_CONTROL.md)