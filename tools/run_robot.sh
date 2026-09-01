#!/usr/bin/env bash
set -euo pipefail

repo_root="${1:-/home/ucar/PetCargo}"
underlay="${2:-/home/ucar/2026-xunfei-race/devel/setup.bash}"
workspace="${3:-/home/ucar/petcargo_ws}"

if [[ ! -f "$underlay" ]]; then echo "Missing underlay: $underlay" >&2; exit 2; fi
if [[ ! -f "$workspace/devel/setup.bash" ]]; then echo "Run install_on_robot.sh first." >&2; exit 3; fi
serial_port="/dev/petcargo_stc"
if [[ ! -e "$serial_port" ]]; then
  fallback_port="/dev/serial/by-id/usb-1a86_USB2.0-Serial-if00-port0"
  if [[ -e "$fallback_port" ]]; then
    serial_port="$fallback_port"
    echo "udev alias missing; using detected STC port: $serial_port" >&2
  else
    echo "STC serial device missing; reconnect the 1a86:7523 USB cable or install the udev rule." >&2
    exit 4
  fi
fi

set +u
source "$underlay"
source "$workspace/devel/setup.bash"
set -u
exec roslaunch petcargo_ros petcargo.launch serial_port:="$serial_port" web_root:="$repo_root/dashboard"
