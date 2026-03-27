"""
Optional Art-Net (ArtDmx 0x5000) output to localhost / LAN for QLC+ etc.

UDP port 6454 — control sim uses 6470 by default to avoid conflict.
"""

from __future__ import annotations

import socket
import struct
from typing import Optional


ARTNET_PORT = 6454
ARTNET_OPCODE_DMX = 0x5000


def build_artdmx_packet(
    universe: int,
    dmx_512: bytes,
    sequence: int = 0,
) -> bytes:
    if len(dmx_512) < 512:
        dmx_512 = dmx_512 + b"\x00" * (512 - len(dmx_512))
    elif len(dmx_512) > 512:
        dmx_512 = dmx_512[:512]

    packet = bytearray()
    packet.extend(b"Art-Net\x00")
    packet.extend(struct.pack("<H", ARTNET_OPCODE_DMX))
    packet.extend(struct.pack(">H", 14))
    packet.append(sequence & 0xFF)
    packet.append(0)
    packet.append(universe & 0xFF)
    packet.append((universe >> 8) & 0x7F)
    packet.extend(struct.pack(">H", 512))
    packet.extend(dmx_512)
    return bytes(packet)


def send_artdmx(
    dmx_512: bytes,
    *,
    host: str = "127.0.0.1",
    universe: int = 0,
    sequence: int = 0,
    sock: Optional[socket.socket] = None,
) -> None:
    pkt = build_artdmx_packet(universe, dmx_512, sequence=sequence)
    own_sock = sock is None
    if sock is None:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.sendto(pkt, (host, ARTNET_PORT))
    finally:
        if own_sock:
            sock.close()
