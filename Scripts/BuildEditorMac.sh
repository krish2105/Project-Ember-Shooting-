#!/bin/bash
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/lib/Common.sh"
verify_turnkey
printf 'Building ProjectEmberEditor for Mac Development\n'
"$BUILD_SH" ProjectEmberEditor Mac Development "$PROJECT_FILE" -WaitMutex -architecture=arm64
