#!/bin/bash
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/lib/Common.sh"
verify_turnkey
require_file "$EDITOR_BIN"
"$EDITOR_BIN" "$PROJECT_FILE" -unattended -nop4 -NullRHI -nosplash \
  -ExecutePythonScript="$PROJECT_ROOT/Scripts/Editor/import_shooter_presentation.py" \
  -log="$PROJECT_ROOT/Saved/Logs/ImportShooterPresentation.log"
