#!/usr/bin/env bash
set -euo pipefail

repo_root="${1:-/home/ucar/PetCargo}"
underlay="${2:-/home/ucar/2026-xunfei-race/devel/setup.bash}"
workspace="${3:-/home/ucar/petcargo_ws}"
package_link="$workspace/src/petcargo_ros"

case "$repo_root" in /home/ucar/*) ;; *) echo "repo_root must stay below /home/ucar" >&2; exit 2;; esac
case "$workspace" in /home/ucar/*) ;; *) echo "workspace must stay below /home/ucar" >&2; exit 2;; esac

if [[ ! -f "$underlay" ]]; then
  echo "Competition underlay not found: $underlay" >&2
  echo "Pass the actual devel/setup.bash path explicitly; no repository was modified." >&2
  exit 3
fi
if [[ ! -f "$repo_root/ros/petcargo_ros/package.xml" ]]; then
  echo "PetCargo ROS package missing below $repo_root" >&2
  exit 4
fi

mkdir -p "$workspace/src"
if [[ -e "$package_link" && ! -L "$package_link" ]]; then
  echo "Refusing to replace non-symlink path: $package_link" >&2
  exit 5
fi
ln -sfn "$repo_root/ros/petcargo_ros" "$package_link"
chmod +x "$repo_root"/ros/petcargo_ros/scripts/*.py "$repo_root"/tools/*.sh

if ! python3 -c 'import serial' >/dev/null 2>&1; then
  echo "Python module 'serial' is missing; installing pyserial for the ucar user."
  if ! python3 -m pip --version >/dev/null 2>&1; then
    echo "Python pip is unavailable; install python3-serial or python3-pip before deploying." >&2
    exit 6
  fi
  python3 -m pip install --user pyserial
fi
python3 -c 'import serial; print("pyserial ready:", serial.__version__)'

set +u
source "$underlay"
set -u
cd "$workspace"
catkin_make

echo "Built: $workspace"
echo "The competition repository was used only as a sourced underlay."
