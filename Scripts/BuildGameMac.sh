#!/bin/bash
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/lib/Common.sh"
verify_turnkey
CONFIGURATION="${CONFIGURATION:-Development}"
printf 'Building ProjectEmber for Mac %s\n' "$CONFIGURATION"
build_game_binary "$CONFIGURATION"
