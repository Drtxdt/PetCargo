"""Read-only HEX/vector/memory/scan audit after build.bat all."""
import hashlib
import json
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = ROOT / "firmware/stc/build"

def read_hex(path):
    data, base, eof = {}, 0, False
    for line in path.read_text().splitlines():
        raw = bytes.fromhex(line.removeprefix(":"))
        assert line.startswith(":") and len(raw) == raw[0] + 5 and sum(raw) % 256 == 0, path
        kind, offset = raw[3], int.from_bytes(raw[1:3], "big")
        if kind == 0:
            for i, v in enumerate(raw[4:-1]):
                address = base + offset + i
                assert address not in data or data[address] == v
                data[address] = v
        elif kind == 1:
            eof = True
        elif kind == 4:
            base = int.from_bytes(raw[4:-1], "big") << 16
        elif kind == 2:
            base = int.from_bytes(raw[4:-1], "big") << 4
        else:
            raise AssertionError(f"Unexpected record: {kind}")
    assert eof and data
    return data

def audit(name, directory, vectors):
    path = directory / f"{name}.hex"
    data = read_hex(path)
    symbols = dict((symbol, int(address, 16)) for address, symbol in
                   re.findall(r"C:\s+([0-9A-Fa-f]+)\s+(_\w+)", (directory / f"{name}.map").read_text()))
    for address, symbol in vectors.items():
        assert data[address] == 2, f"Missing LJMP {symbol}"
        assert (data[address + 1] << 8 | data[address + 2]) == symbols[symbol], symbol
    report = (directory / f"{name}.mem").read_text()
    xram = re.search(r"EXTERNAL RAM\s+0x\w+\s+0x\w+\s+(\d+)\s+(\d+)", report)
    stack = re.search(r"with (\d+) bytes available", report)
    assert xram and int(xram[1]) <= int(xram[2]) == 2048
    assert stack and int(stack[1]) >= 128, "Insufficient stack reserve"
    assert max(data) < 61440
    return dict(image=str(path.relative_to(ROOT)), sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                code_high_water=max(data)+1, xram_bytes=int(xram[1]), stack_reserve=int(stack[1]))

vectors = {0x0B: "_timer0_isr", 0x23: "_uart1_isr", 0x3B: "_pca_isr", 0x43: "_uart2_isr"}
results = [audit(name, BUILD, vectors) for name in ("petcargo", "petcargo_diagnostic")]
results.append(audit("petcargo_remote", ROOT / "firmware/stc_remote/build", {0x0B: "_timer0_isr"}))
asm = (BUILD / "hal.asm").read_text()
scan = asm.split("_timer0_isr:", 1)[1].split("reti", 1)[0]
assert not re.search(r"\b[la]call\b", scan), "Scan ISR must not call helper functions"
assert not re.search(r"mov\s+a,\s*_P2\b", scan), "P2 pin readback is unsafe"
assert re.search(r"anl\s+_P2,#0xf0", scan) and re.search(r"orl\s+_P2,a", scan)
hal = (ROOT / "firmware/stc/src/hal.c").read_text()
assert "IE |= 0x40" not in hal and "TMOD &= 0xF0" in hal
assert "P_SW2 &= (uint8_t)~0x01" in hal
pins = (ROOT / "firmware/stc/include/stc15.h").read_text()
for address, pin in (("CC", "PIN_RTC_IO"), ("96", "PIN_RTC_RST"), ("C4", "PIN_SM_S1"), ("C3", "PIN_SM_S2")):
    assert re.search(rf"__at \(0x{address}\) {pin}\b", pins), pin
print(json.dumps(results, indent=2))
print("PASS HEX checksums, linked vectors, memory reserve, timer scan assembly and pin map")
