# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if [ -z "$1" ]; then
  echo "Usage: $0 <process_name>"
  echo
  echo "Output the following for each process maching <process_name>:"
  echo "==== process <pid> ===="
  echo "<proc/cmdline>"
  echo "==== smaps_rollup ===="
  echo "<proc/smaps_rollup>"
  exit 1
fi

PROCESS_NAME="$1"

for PID in $(pgrep -f "$PROCESS_NAME"); do
  CMDLINE=$(cat "/proc/$PID/cmdline" 2>/dev/null | tr '\0' ' ')
  if [ "$?" -ne 0 ]; then
    # Process is probably gone.
    continue
  fi
  SMAPS_ROLLUP=$(cat "/proc/$PID/smaps_rollup" 2>/dev/null)
  if [ "$?" -ne 0 ]; then
    # Process is probably gone.
    continue
  fi
  echo "==== process $PID ===="
  echo "$CMDLINE"
  echo "==== smaps_rollup ===="
  echo "$SMAPS_ROLLUP"
done
