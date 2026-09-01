"""PetCargo STC/ROS binary protocol.

CRC16-CCITT is calculated over VERSION, TYPE, SEQ, LENGTH and PAYLOAD.
Multi-byte payload fields are little endian.
"""

from __future__ import annotations

import enum
import struct
from dataclasses import asdict, dataclass
from typing import Iterable, List

HEADER = b"\xAA\x55"
VERSION = 2
MAX_PAYLOAD = 32


class MessageType(enum.IntEnum):
    HELLO = 0x01
    HEARTBEAT = 0x02
    TELEMETRY = 0x10
    EVENT = 0x11
    MOTION_REQUEST = 0x20
    MOTION_RESULT = 0x21
    JOG_REQUEST = 0x22
    STOP = 0x31
    ACK = 0x7E
    NACK = 0x7F


class MotionKind(enum.IntEnum):
    LINEAR = 1
    ROTATE = 2
    LATERAL = 3


class MotionResultCode(enum.IntEnum):
    COMPLETED = 0
    STOPPED = 1
    TIMEOUT = 2
    ODOM_ERROR = 3
    REJECTED_BUSY = 4
    REJECTED_SAFETY = 5


class EventCode(enum.IntEnum):
    BRIGHT_LIGHT = 1
    SHAKE = 2
    FEED = 3
    TEMPERATURE = 4
    VOICE = 5
    BUTTON = 6
    FAULT = 8
    REMOTE = 9


@dataclass(frozen=True)
class Frame:
    msg_type: int
    seq: int
    payload: bytes = b""
    version: int = VERSION


@dataclass(frozen=True)
class Telemetry:
    uptime_ms: int
    light_raw: int
    temp_x10: int
    accel_x_mg: int
    accel_y_mg: int
    accel_z_mg: int
    vibration: int
    hall: int
    happiness: int
    fear: int
    flags: int
    feed_count: int

    _STRUCT = struct.Struct("<IBhhhhBBBBBH")

    @classmethod
    def unpack(cls, payload: bytes) -> "Telemetry":
        if len(payload) != cls._STRUCT.size:
            raise ValueError(f"telemetry payload must be {cls._STRUCT.size} bytes")
        return cls(*cls._STRUCT.unpack(payload))

    def pack(self) -> bytes:
        return self._STRUCT.pack(*asdict(self).values())


@dataclass(frozen=True)
class MotionRequest:
    motion_id: int
    kind: int
    distance_mm: int
    angle_cdeg: int
    max_speed: int

    _STRUCT = struct.Struct("<HBhiH")

    @classmethod
    def unpack(cls, payload: bytes) -> "MotionRequest":
        if len(payload) != cls._STRUCT.size:
            raise ValueError(f"motion request must be {cls._STRUCT.size} bytes")
        return cls(*cls._STRUCT.unpack(payload))

    def pack(self) -> bytes:
        return self._STRUCT.pack(
            self.motion_id, self.kind, self.distance_mm, self.angle_cdeg, self.max_speed
        )


@dataclass(frozen=True)
class JogRequest:
    direction: int
    speed_mm_s: int
    lease_ms: int

    _STRUCT = struct.Struct("<BHH")

    @classmethod
    def unpack(cls, payload: bytes) -> "JogRequest":
        if len(payload) != cls._STRUCT.size:
            raise ValueError(f"jog request must be {cls._STRUCT.size} bytes")
        return cls(*cls._STRUCT.unpack(payload))

    def pack(self) -> bytes:
        return self._STRUCT.pack(self.direction, self.speed_mm_s, self.lease_ms)


@dataclass(frozen=True)
class MotionResult:
    motion_id: int
    code: int
    actual_mm: int
    actual_cdeg: int

    _STRUCT = struct.Struct("<HBhi")

    @classmethod
    def unpack(cls, payload: bytes) -> "MotionResult":
        if len(payload) != cls._STRUCT.size:
            raise ValueError(f"motion result must be {cls._STRUCT.size} bytes")
        return cls(*cls._STRUCT.unpack(payload))

    def pack(self) -> bytes:
        return self._STRUCT.pack(self.motion_id, self.code, self.actual_mm, self.actual_cdeg)


def crc16_ccitt(data: bytes, initial: int = 0xFFFF) -> int:
    crc = initial
    for value in data:
        crc ^= value << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def encode_frame(frame: Frame) -> bytes:
    payload = bytes(frame.payload)
    if len(payload) > MAX_PAYLOAD:
        raise ValueError(f"payload exceeds {MAX_PAYLOAD} bytes")
    body = bytes((frame.version, int(frame.msg_type), frame.seq & 0xFF, len(payload))) + payload
    return HEADER + body + struct.pack("<H", crc16_ccitt(body))


class FrameParser:
    """Incremental parser that resynchronizes after noise and CRC failures."""

    def __init__(self) -> None:
        self._buffer = bytearray()
        self.crc_errors = 0
        self.length_errors = 0

    def feed(self, data: Iterable[int] | bytes) -> List[Frame]:
        self._buffer.extend(data)
        frames: List[Frame] = []
        while True:
            header_at = self._buffer.find(HEADER)
            if header_at < 0:
                if self._buffer[-1:] == HEADER[:1]:
                    self._buffer[:] = HEADER[:1]
                else:
                    self._buffer.clear()
                break
            if header_at:
                del self._buffer[:header_at]
            if len(self._buffer) < 8:
                break
            length = self._buffer[5]
            if length > MAX_PAYLOAD:
                self.length_errors += 1
                del self._buffer[0]
                continue
            frame_size = 2 + 4 + length + 2
            if len(self._buffer) < frame_size:
                break
            candidate = bytes(self._buffer[:frame_size])
            body = candidate[2:-2]
            expected = struct.unpack("<H", candidate[-2:])[0]
            if crc16_ccitt(body) != expected:
                self.crc_errors += 1
                del self._buffer[0]
                continue
            frames.append(Frame(body[1], body[2], body[4:], body[0]))
            del self._buffer[:frame_size]
        return frames


def telemetry_to_dict(value: Telemetry) -> dict:
    result = asdict(value)
    result["sleeping"] = bool(value.flags & 0x01)
    result["emergency"] = bool(value.flags & 0x02)
    result["calibrating"] = bool(value.flags & 0x04)
    result["remote_active"] = bool(value.flags & 0x20)
    return result
