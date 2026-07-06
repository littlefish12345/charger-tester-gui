# TODO - Missing Features & Commands

## Protocol Commands Not Yet Implemented

The following command types have been identified as part of the protocol space
but are not yet implemented. These should be added as the hardware/firmware
capabilities are finalized.

### Outgoing Commands (1xx range)

| Command | Name | Description |
|---------|------|-------------|
| 104-110 | Reserved | Expansion space for additional power/mode commands |
| 111 | (proposed) | Query device info / firmware version |
| 112 | (proposed) | Set over-voltage protection threshold |
| 113 | (proposed) | Set over-current protection threshold |
| 114 | (proposed) | Set over-temperature protection threshold |
| 115 | (proposed) | Enable/disable output |
| 116 | (proposed) | Factory reset |

### Incoming Reports (2xx range)

| Command | Name | Description |
|---------|------|-------------|
| 203 | (proposed) | Temperature report |
| 204 | (proposed) | Protection event alert (OVP/OCP/OTP triggered) |
| 205 | (proposed) | Device info response |
| 210-220 | Reserved | Expansion space |

## UI Features Not Yet Implemented

- [ ] Dedicated PD packet decoder (parsing individual PD messages like Source_Cap, Request, etc.)
- [ ] CSV data recording / export
- [ ] Chart time window presets (30s/60s/120s toggle buttons)
- [ ] Chart horizontal scroll for history
- [ ] Protection status indicators (OVP/OCP/OTP warning lights)
- [ ] Temperature / fan speed display
- [ ] Auto-polling timer for periodic status queries
- [ ] Command history log (last N commands sent with timestamps)
- [ ] Save/restore window geometry

## Implementation Notes

- The protocol uses text-based `<chip_id,command>{json}` format. Extension commands
  should follow this same format.
- Current power modes: XT CC(11), XT CV(12), TypeC Decoy(20), TypeC Decoy+CC(21)
- The chip ID is configurable (default: "ABCDABCD") via CommandBuilder::setChipId()
