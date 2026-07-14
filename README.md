# PROJECT EMBER

PROJECT EMBER is a native Apple Silicon, third-person tactical shooter vertical slice targeting Unreal Engine 5.8.

The repository contains the buildable source, Editor-generated content and packaged-release workflow. UE 5.8, Xcode 26.6 and the Apple Metal Toolchain have been verified on the M4 Pro build host.

The current mouse-input/enemy-facing correction and Gatekeeper instructions are documented in [Docs/RELEASE_NOTES_0.3.4-dev.1.md](Docs/RELEASE_NOTES_0.3.4-dev.1.md).

[Download Project Ember v0.3.4-dev.1 for Apple Silicon macOS](https://github.com/krish2105/Project-Ember-Shooting-/releases/download/v0.3.4-dev.1/ProjectEmber-mac-arm64-Development.zip)

SHA-256: `2dd901c9acdb7fba8e31c614da4f678f54a23258b54a7d6d66cf8aefd8b95801`

## Bootstrap

1. Install the matching Unreal Engine 5.8 build.
2. Verify full Xcode with `xcodebuild -version` and `xcrun --sdk macosx --show-sdk-version`.
3. Set `UE_ROOT` if the engine is not at `/Users/Shared/Epic Games/UE_5.8`.
4. Run `Scripts/BuildEditorMac.sh`.
5. Follow `Docs/MANUAL_EDITOR_STEPS.md` to generate and validate Editor-owned assets.

No Unreal binary asset in this repository may be fabricated by a text-generation tool.
