# Hyundai Ioniq 5 / Kia EV6 - Connection Mode Configuration

## Overview

The Ioniq 5 and Kia EV6 have CAN gateways that prevent the regular OBD port from transmitting on certain required CAN busses. This module supports two connection modes:

1. **OBD Port Mode** (default): Standard connection through the OBD-II port. Suitable for read-only operations and basic monitoring.
2. **Intercept Harness Mode**: Connection through a CAN intercept harness that provides full access to all CAN busses, enabling advanced features like remote start, window control, and charge settings.

## Configuration

### Setting Connection Mode

The connection mode can be configured using the `xiq.con_obd` parameter:

- `true` (1): OBD Port mode (default)
- `false` (0): Intercept Harness mode

#### Via OVMS Commands

```bash
# Set to OBD Port mode
ovms> config set xiq.con_obd 1

# Set to Intercept Harness mode  
ovms> config set xiq.con_obd 0

# Check current mode
ovms> config get xiq.con_obd
```

#### Via SetFeature/GetFeature API

The connection mode can also be set using the standard OVMS feature API:

```cpp
// Set connection mode (key=20)
// value "1" = OBD Port, "0" = Intercept Harness
vehicle->SetFeature(20, "1");

// Get current mode
std::string mode = vehicle->GetFeature(20);
```

### Logging

When the configuration is loaded, the connection mode is logged:

```
I (xxxx) v-ioniq5: Connection mode: OBD port
```
or
```
I (xxxx) v-ioniq5: Connection mode: Intercept harness
```

## Implementation Details

### Member Variable

- `m_con_obd` (bool): Stores the current connection mode
  - `true`: OBD Port mode
  - `false`: Intercept Harness mode

### Configuration Parameter

- **Parameter Name**: `xiq.con_obd`
- **Type**: Boolean
- **Default**: `true` (OBD Port mode)
- **Location**: `vehicle_hyundai_ioniq5.cpp::ConfigChanged()`

### Feature Key

- **Key**: 20
- **SetFeature**: Writes to `xiq.con_obd` parameter
- **GetFeature**: Returns "1" for OBD Port, "0" for Intercept Harness

## Usage Considerations

### OBD Port Mode

**Pros:**
- Simple plug-and-play installation
- No vehicle modification required
- Suitable for monitoring and diagnostics

**Cons:**
- Limited CAN bus access due to gateway restrictions
- Cannot send commands on certain busses
- Some advanced features may not work

### Intercept Harness Mode

**Pros:**
- Full access to all CAN busses
- Enables advanced control features:
  - Remote start
  - Window control
  - Charge settings modification
  - Climate control
  - Door lock/unlock

**Cons:**
- Requires hardware installation
- More complex setup

## Future Enhancements

The connection mode flag (`m_con_obd`) can be used to:

1. Adjust CAN bus registration based on connection type
2. Enable/disable certain features based on available bus access
3. Modify polling strategies for different connection modes
4. Provide user feedback about feature availability

## Related Files

- `vehicle_hyundai_ioniq5.h`: Header file with `m_con_obd` member declaration
- `vehicle_hyundai_ioniq5.cpp`: Implementation of connection mode handling
  - `ConfigChanged()`: Loads configuration
  - `SetFeature()`: Handles feature key 20
  - `GetFeature()`: Returns current mode

## References

- OVMS Vehicle Framework Documentation
- Hyundai Ioniq 5 CAN Reverse Engineering Discord Logs
- Kia EV6 CAN Bus Documentation
