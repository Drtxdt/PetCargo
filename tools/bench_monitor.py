"""Read diagnostic firmware ASCII at 9600 baud; never sends motion or control bytes."""
import argparse
import serial
from serial.tools import list_ports

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("port", nargs="?", help="e.g. COM7; omit to list available ports")
    args = parser.parse_args()
    if not args.port:
        for port in list_ports.comports():
            print(port.device, port.description)
        return
    # Set handshake outputs BEFORE opening the CH340 port; do not auto-reset.
    stream = serial.Serial(port=None, baudrate=9600, timeout=0.5)
    stream.dtr = False
    stream.rts = False
    stream.port = args.port
    stream.open()
    try:
        while True:
            line = stream.readline()
            if line:
                print(line.decode("ascii", errors="replace").rstrip(), flush=True)
    except KeyboardInterrupt:
        pass
    finally:
        stream.close()

if __name__ == "__main__":
    main()
