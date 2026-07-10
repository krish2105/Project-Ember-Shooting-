#!/bin/bash
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/lib/Common.sh"
verify_turnkey
require_file "$EDITOR_BIN"
"$EDITOR_BIN" "$PROJECT_FILE" -unattended -nop4 -NullRHI -nosplash \
  -run=EmberImportThirdPerson -log="$PROJECT_ROOT/Saved/Logs/ImportThirdPerson.log"
