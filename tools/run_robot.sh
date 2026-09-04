#!/usr/bin/env bash
set -euo pipefail

repo_root="${1:-/home/ucar/PetCargo}"
underlay="${2:-/home/ucar/2026-xunfei-race/devel/setup.bash}"
workspace="${3:-/home/ucar/petcargo_ws}"
start_lidar="${4:-true}"

# Keep exactly one PetCargo roslaunch attached to this ROS master. Starting a
# second copy with the same node names makes ROS shut down nodes from the first
# copy; stopping either launch can then leave the other one half alive.
lock_file="/tmp/petcargo-run.lock"
exec 9>"$lock_file"
if ! flock -n 9; then
  echo "PetCargo is already running. Stop the existing run_robot.sh/roslaunch before starting another copy." >&2
  exit 6
fi

if [[ "$start_lidar" != "true" && "$start_lidar" != "false" ]]; then
  echo "start_lidar must be true or false." >&2
  exit 2
fi

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
if [[ "$start_lidar" == "true" ]] && ! rospack find ydlidar >/dev/null 2>&1; then
  echo "The ydlidar package is missing from the sourced underlay." >&2
  exit 5
fi
exec roslaunch petcargo_ros petcargo.launch serial_port:="$serial_port" web_root:="$repo_root/dashboard" start_lidar:="$start_lidar"
