"""Compile and execute production C with fake peripherals; no STC hardware emulation."""
import pathlib
import shutil
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
cc = shutil.which("gcc") or shutil.which("clang")
if not cc:
    raise SystemExit("Install GCC or Clang for the native firmware logic tests.")
with tempfile.TemporaryDirectory(prefix="petcargo-c-tests-") as directory:
    output = pathlib.Path(directory) / "firmware_tests.exe"
    command = [cc, "-std=c99", "-O2", "-Wall", "-Wextra", "-Wno-unused-parameter",
               "-Wno-misleading-indentation", "-D__xdata=", "-D__code=",
               "-I", str(ROOT / "firmware/stc/include"),
               str(ROOT / "tests/firmware_runtime_test.c")]
    command += [str(ROOT / "firmware/stc/src" / f"{name}.c")
                for name in ("runtime", "music", "score", "protocol")]
    subprocess.run(command + ["-o", str(output)], check=True)
    subprocess.run([str(output)], check=True)
    oled_output = pathlib.Path(directory) / "oled_tests.exe"
    subprocess.run([cc, "-std=c99", "-O2", "-Wall", "-Wextra",
                    "-I", str(ROOT / "firmware/stc/include"),
                    str(ROOT / "tests/oled_driver_test.c"),
                    "-o", str(oled_output)], check=True)
    subprocess.run([str(oled_output)], check=True)
