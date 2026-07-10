#!/bin/bash
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/lib/Common.sh"
verify_turnkey
new_evidence_dir "${EMBER_ARCHIVE_DIR:-$PROJECT_ROOT/Releases/Development}"
OUT="$EVIDENCE_DIR"
TEMP_OUT="$(mktemp -d /tmp/projectember-development.XXXXXX)"
trap 'rm -rf "$TEMP_OUT"' EXIT
"$UAT" BuildCookRun -project="$PROJECT_FILE" -platform=Mac \
  -clientconfig=Development -architecture=arm64 -cook -stage -pak -package -archive \
  -stagingdirectory="$TEMP_OUT/Stage" -archivedirectory="$TEMP_OUT/Archive" -unattended -utf8output
validate_and_archive_app "$TEMP_OUT/Archive" "ProjectEmber-mac-arm64-Development"
cp "$TEMP_OUT/Archive"/*.txt "$TEMP_OUT/Archive"/*.zip "$TEMP_OUT/Archive"/*.sha256 "$OUT/"
