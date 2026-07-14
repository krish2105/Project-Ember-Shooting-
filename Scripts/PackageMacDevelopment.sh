#!/bin/bash
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/lib/Common.sh"
verify_turnkey
new_evidence_dir "${EMBER_ARCHIVE_DIR:-$PROJECT_ROOT/Releases/Development}"
OUT="$EVIDENCE_DIR"
TEMP_ROOT="$(mktemp -d /private/tmp/projectember-development.XXXXXX)"
trap 'status=$?; if (( status == 0 )); then rm -rf "$TEMP_ROOT"; else printf "Packaging workspace retained for diagnosis: %s\n" "$TEMP_ROOT"; fi' EXIT
TEMP_PROJECT="$TEMP_ROOT/ProjectEmber"
mkdir -p "$TEMP_PROJECT"
git -C "$PROJECT_ROOT" archive HEAD | tar -x -C "$TEMP_PROJECT"
TEMP_PROJECT_FILE="$TEMP_PROJECT/ProjectEmber.uproject"
"$UAT" BuildCookRun -project="$TEMP_PROJECT_FILE" -platform=Mac \
  -clientconfig=Development -architecture=arm64 -build -cook -CookDir="$TEMP_PROJECT/Content/Ember" -stage -pak -package -archive \
  -stagingdirectory="$TEMP_ROOT/Stage" -archivedirectory="$TEMP_ROOT/Archive" -unattended -utf8output
validate_and_archive_app "$TEMP_ROOT/Archive" "ProjectEmber-mac-arm64-Development"
cp "$TEMP_ROOT/Archive"/*.txt "$TEMP_ROOT/Archive"/*.zip "$TEMP_ROOT/Archive"/*.sha256 "$OUT/"
