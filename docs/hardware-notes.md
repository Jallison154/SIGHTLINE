# Hardware Notes

## Controller (ESP32 + Ethernet)

- Pan/tilt encoders:
  - Use ESP32 PCNT for robust quadrature counting
  - Assign one PCNT unit per encoder
  - Use glitch filter for bounce/noise reduction
- Ethernet:
  - Confirm PHY type/address/clock mode per chosen board
  - Validate Art-Net packet path on target switch infrastructure

## Fixture Node (ESP32 + Ethernet + RS-485)

- UART for DMX:
  - `250000`, `8N2`
  - TX pin to RS-485 DI
  - optional DE/RE direction pin handling
- DMX timing:
  - production code should guarantee BREAK and MAB timings
  - verify against fixture tolerance and DMX analyzer
- Ethernet:
  - validate link-up behavior and boot-time DHCP/static transitions

## Electrical Integration

- Use proper RS-485 transceiver and line termination strategy
- Verify ground/reference strategy between node and fixture chain
- Keep encoder cable runs short or shielded where practical
- Add transient protection as needed for event environments

## Bring-Up Checklist

1. Verify boot + Ethernet link
2. Confirm Art-Net Tx/Rx on expected universe
3. Confirm DMX output timing and level integrity
4. Validate encoder direction and counts per detent
5. Confirm config save/load and default fallback behavior
