#!/usr/bin/env bash
set -euo pipefail

repo_root="${1:-/home/ucar/PetCargo}"
rule="$repo_root/ros/petcargo_ros/udev/99-petcargo-stc.rules"
if [[ ! -f "$rule" ]]; then
  echo "Rule not found: $rule" >&2
  exit 2
fi
install -m 0644 "$rule" /etc/udev/rules.d/99-petcargo-stc.rules
udevadm control --reload-rules
udevadm trigger
echo "Reconnect the STC USB cable, then verify: ls -l /dev/petcargo_stc"

