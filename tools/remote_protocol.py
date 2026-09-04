"""PetCargo USB remote serial framing, independent from serial/HTTP I/O."""

HEADER = b"\xA5\x5A"
VERSION = 2
FRAME_SIZE = 6


def checksum(data: bytes) -> int:
    value = 0
    for byte in data:
        value ^= byte
    return value


def encode(direction: int, sequence: int) -> bytes:
    if direction not in range(6):
        raise ValueError("direction must be 0..5")
    frame = HEADER + bytes((VERSION, direction, sequence & 0xFF))
    return frame + bytes((checksum(frame),))


class RemoteParser:
    def __init__(self) -> None:
        self.buffer = bytearray()
        self.frames = 0
        self.errors = 0

    def feed(self, data: bytes):
        self.buffer.extend(data)
        result = []
        while True:
            start = self.buffer.find(HEADER)
            if start < 0:
                if self.buffer[-1:] == HEADER[:1]:
                    del self.buffer[:-1]
                else:
                    self.buffer.clear()
                break
            if start:
                del self.buffer[:start]
                self.errors += 1
            if len(self.buffer) < FRAME_SIZE:
                break
            frame = bytes(self.buffer[:FRAME_SIZE])
            if frame[2] != VERSION or frame[3] > 5 or checksum(frame[:5]) != frame[5]:
                del self.buffer[0]
                self.errors += 1
                continue
            del self.buffer[:FRAME_SIZE]
            self.frames += 1
            result.append({"direction": frame[3], "sequence": frame[4]})
        return result
