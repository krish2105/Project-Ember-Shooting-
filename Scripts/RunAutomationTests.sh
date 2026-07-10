#!/bin/bash
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/lib/Common.sh"
verify_turnkey
require_file "$EDITOR_BIN"
new_evidence_dir "$PROJECT_ROOT/Saved/Automation"
FILTER="${EMBER_TEST_FILTER:-ProjectEmber}"
"$EDITOR_BIN" "$PROJECT_FILE" -unattended -nop4 -NullRHI -nosplash \
  -ExecCmds="Automation RunTests $FILTER;Quit" \
  -TestExit="Automation Test Queue Empty" \
  -ReportExportPath="$EVIDENCE_DIR" \
  -log="$PROJECT_ROOT/Saved/Logs/Automation.log"
find "$EVIDENCE_DIR" -type f -print -quit | grep -q . || fail "Automation produced no report files."
grep -R -q 'ProjectEmber' "$EVIDENCE_DIR" || fail "Automation report contains no PROJECT EMBER tests."
if grep -R -E -q '"state"[[:space:]]*:[[:space:]]*"(Fail|Skipped|NotRun)"' "$EVIDENCE_DIR"; then
  fail "Automation report contains failed, skipped, or unrun tests."
fi
