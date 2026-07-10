#!/bin/bash
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/lib/Common.sh"
verify_turnkey
require_file "$EDITOR_BIN"
PYTHON_SCRIPT="$PROJECT_ROOT/Scripts/Editor/build_harbor.py"
require_file "$PYTHON_SCRIPT"
"$EDITOR_BIN" "$PROJECT_FILE" -unattended -nop4 -NullRHI -nosplash -stdout -FullStdOutLogOutput \
  -run=pythonscript -script="$PYTHON_SCRIPT" -abslog="$PROJECT_ROOT/Saved/Logs/BuildHarborMap.log"
