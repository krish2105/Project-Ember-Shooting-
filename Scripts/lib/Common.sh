#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PROJECT_FILE="$PROJECT_ROOT/ProjectEmber.uproject"
if [[ -n "${UE_ROOT:-}" ]]; then
  UE_ROOT="$UE_ROOT"
elif [[ -d "/Users/Shared/Epic Games/UE_5.8" ]]; then
  UE_ROOT="/Users/Shared/Epic Games/UE_5.8"
elif [[ -d "$HOME/Desktop/UE_5.8" ]]; then
  UE_ROOT="$HOME/Desktop/UE_5.8"
else
  UE_ROOT="/Users/Shared/Epic Games/UE_5.8"
fi
UAT="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"
BUILD_SH="$UE_ROOT/Engine/Build/BatchFiles/Mac/Build.sh"
EDITOR_BIN="$UE_ROOT/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"

fail() { printf 'ERROR: %s\n' "$*" >&2; exit 1; }
require_file() { [[ -f "$1" ]] || fail "Required file not found: $1"; }
require_dir() { [[ -d "$1" ]] || fail "Required directory not found: $1"; }

require_project() {
  require_file "$PROJECT_FILE"
  require_dir "$UE_ROOT"
}

require_build_tools() {
  require_project
  require_file "$BUILD_SH"
  require_file "$UAT"
  command -v xcodebuild >/dev/null 2>&1 || fail "Full Xcode is required."
  xcodebuild -version >/dev/null 2>&1 || fail "xcodebuild is not configured; select full Xcode."
  xcrun --sdk macosx --show-sdk-version >/dev/null 2>&1 || fail "A macOS SDK is not available through Xcode."
  [[ "$(uname -m)" == "arm64" ]] || fail "PROJECT EMBER requires an Apple Silicon build host."
}

verify_turnkey() {
  require_build_tools
  printf 'Verifying Unreal Mac SDK with Turnkey\n'
  "$UAT" Turnkey -command=VerifySdk -platform=Mac -utf8output
}

new_evidence_dir() {
  local root="$1"
  local build_id="${EMBER_BUILD_ID:-$(date -u +%Y%m%dT%H%M%SZ)}"
  EVIDENCE_DIR="$root/$build_id"
  [[ ! -e "$EVIDENCE_DIR" ]] || fail "Evidence directory already exists: $EVIDENCE_DIR"
  mkdir -p "$EVIDENCE_DIR"
}

validate_and_archive_app() {
  local out="$1"
  local label="$2"
  local app_count
  app_count="$(find "$out" -type d -name '*.app' -prune | wc -l | tr -d ' ')"
  [[ "$app_count" == "1" ]] || fail "Expected exactly one app bundle under $out, found $app_count."
  local app
  app="$(find "$out" -type d -name '*.app' -prune -print | head -n 1)"
  local executable_name
  executable_name="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$app/Contents/Info.plist")"
  local executable="$app/Contents/MacOS/$executable_name"
  require_file "$executable"

  /usr/bin/codesign --force --deep --sign - --timestamp=none "$app"
  /usr/bin/plutil -lint "$app/Contents/Info.plist"
  /usr/bin/codesign --verify --deep --strict --verbose=4 "$app"
  local archs
  archs="$(/usr/bin/lipo -archs "$executable")"
  [[ "$archs" == "arm64" ]] || fail "Expected arm64-only executable, found: $archs"
  /usr/bin/file "$executable" > "$out/${label}-file.txt"
  /usr/bin/codesign -dv --verbose=4 "$app" > "$out/${label}-codesign.txt" 2>&1
  /usr/sbin/spctl --assess --type execute -vv "$app" > "$out/${label}-gatekeeper.txt" 2>&1 || true

  local zip="$out/${label}.zip"
  /usr/bin/ditto -c -k --sequesterRsrc --keepParent "$app" "$zip"
  (cd "$out" && /usr/bin/shasum -a 256 "$(basename "$zip")") > "$zip.sha256"
  printf 'Created validated arm64 archive: %s\n' "$zip"
}

build_game_binary() {
  local configuration="$1"
  "$BUILD_SH" ProjectEmber Mac "$configuration" "$PROJECT_FILE" -WaitMutex -architecture=arm64 -SkipDeploy
}
