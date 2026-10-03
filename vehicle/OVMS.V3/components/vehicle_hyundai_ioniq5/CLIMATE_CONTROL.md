# Remote Climate Control for Hyundai Ioniq 5 / Kia EV6

## Overview

Remote climate control (preconditioning) allows you to start or stop the vehicle's HVAC system remotely via OVMS commands. This feature requires the **intercept harness** to be installed, as it involves writing to the CAN bus.

## Requirements

1. **Intercept Harness**: The CCU intercept harness must be installed (OBD port mode does not support CAN writes)
2. **CAN Write Enabled**: Set `xiq.canwrite=1` in configuration
3. **Connection Mode**: Set `xiq.con_obd=0` (intercept harness mode)
4. **Valid PIN**: Your vehicle PIN code is required for authentication
5. **Vehicle in Park**: Climate commands only work when the vehicle is in Park

## Configuration

Before using climate control, ensure the following configuration:

```bash
# Enable CAN writes
OVMS> config set xiq.canwrite 1

# Set connection mode to intercept harness (required for writes)
OVMS> config set xiq.con_obd 0

# Verify settings
OVMS> config get xiq.canwrite    # Should return 1
OVMS> config get xiq.con_obd     # Should return 0
```

## Commands

### Start Climate Control

```bash
# Start climate control (runs until manually stopped)
OVMS> vehicle climate start <PIN>

# Start climate control with auto-shutoff after 15 minutes
OVMS> vehicle climate start <PIN> 15
```

**Parameters:**
- `PIN`: Your vehicle PIN code (required)
- `duration_minutes`: Optional auto-shutoff duration (default: 0 = no auto-shutoff)

### Stop Climate Control

```bash
# Stop climate control
OVMS> vehicle climate stop <PIN>
```

**Parameters:**
- `PIN`: Your vehicle PIN code (required)

### Set Temperature (Not Yet Implemented)

```bash
# Set target temperature (placeholder - not yet functional)
OVMS> vehicle climate temp <temperature_celsius>
```

**Note**: Temperature control requires additional reverse engineering. The command structure is defined but the CAN message format needs to be determined through further research.

## Safety Features

The climate control implementation includes multiple safety checks:

1. **Intercept Harness Verification**: Commands are blocked if `xiq.con_obd=1` (OBD port mode)
2. **CAN Write Authorization**: Commands are blocked if `xiq.canwrite=0`
3. **PIN Authentication**: Valid PIN required for all climate commands
4. **Park Position Check**: Commands only work when vehicle is in Park (gear=0)
5. **Auto-Shutoff**: Optional automatic shutoff after specified duration
6. **State Tracking**: Prevents duplicate start/stop commands

## Technical Details

### ECU Information

- **AirCon ECU**: `0x7b3` (TX) → `0x7bb` (RX)
- **Session Mode**: Extended Diagnostic Session (`0x03`)
- **Service ID**: `0x2F` (IO Control By ID)

### Command Structure

```
Start: 7b3 [len] 2F 01 00 00 00
Stop:  7b3 [len] 2F 00 00 00 00
```

### Spam Message Pattern

Based on community reverse engineering findings, climate commands require repeated transmission ("spamming") for reliable execution. The implementation sends 3 repeated commands with 100ms delays.

## Known Limitations

1. **Temperature Control**: Setting specific temperatures is not yet implemented
2. **Fan Speed**: Fan speed control not yet available
3. **Seat Heating/Cooling**: Not implemented
4. **Steering Wheel Heater**: Not implemented
5. **I-CAN Dependency**: Some climate functions may require I-CAN access (still under investigation)

## Troubleshooting

### Climate control not starting

1. Verify intercept harness is installed and properly connected
2. Check configuration: `xiq.con_obd` should be `0`
3. Verify CAN writes are enabled: `xiq.canwrite` should be `1`
4. Ensure vehicle is in Park
5. Verify PIN is correct
6. Check logs for error messages

### Climate control stops immediately

- This may indicate the command was not properly received by the HVAC ECU
- Try sending the command again
- Check that no other system is interfering with CAN communication

### Auto-shutoff not working

- The ticker runs every 10 seconds, so shutoff may be delayed by up to 10 seconds
- Check that the climate control state is being tracked correctly

## Future Enhancements

The following features are planned for future implementation:

1. **Temperature Setting**: Full temperature control (15-30°C)
2. **Fan Speed Control**: Adjustable fan speeds
3. **Seat Climate**: Heated and cooled seat control
4. **Steering Wheel Heater**: Remote activation
5. **Scheduled Preconditioning**: Time-based climate activation
6. **Battery Preconditioning**: Integration with charge scheduling

## References

- [Ioniq5 CAN Repository](https://github.com/tylerharvey/Ioniq5_CAN)
- [ironiq.py - Preconditioning Implementation](https://github.com/dragz/ironiq/blob/main/ironiq.py)
- [WiCAN-Ti Preconditioning](https://github.com/eiiot/wicant-i-precondition)
- Discord: Electroniq Buttons - CAN Reverse Engineering community

## Disclaimer

**USE AT YOUR OWN RISK**: Remote climate control involves writing to the vehicle's CAN bus. Incorrect usage could potentially affect vehicle systems. Always test in a safe environment and monitor vehicle behavior. The authors and contributors are not responsible for any damage or issues that may arise from using this feature.