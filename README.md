# PROJECT EMBER

PROJECT EMBER is a native Apple Silicon, third-person tactical shooter vertical slice targeting Unreal Engine 5.8.

The repository currently contains the audited source foundation. Full Xcode 26.6 is installed; Unreal Engine 5.8 is still required before compilation, Editor asset generation, testing, or packaging can be claimed.

## Bootstrap

1. Install the latest Unreal Engine 5.8 hotfix through Epic Games Launcher.
2. Verify full Xcode with `xcodebuild -version` and `xcrun --sdk macosx --show-sdk-version`.
3. Set `UE_ROOT` if the engine is not at `/Users/Shared/Epic Games/UE_5.8`.
4. Run `Scripts/BuildEditorMac.sh`.
5. Follow `Docs/MANUAL_EDITOR_STEPS.md` to generate and validate Editor-owned assets.

No Unreal binary asset in this repository may be fabricated by a text-generation tool.
