# PROJECT EMBER 0.1.0 — Harbor Vertical Slice

Native Apple Silicon Unreal Engine 5.8 tactical-shooter vertical slice.

## Play

1. Download `ProjectEmber-mac-arm64-Shipping.zip` and its `.sha256` file.
2. Verify it with `shasum -a 256 ProjectEmber-mac-arm64-Shipping.zip`.
3. Extract the ZIP.
4. Because this build is ad-hoc signed and not notarized, Control-click the app, choose **Open**, then confirm **Open**. If macOS still blocks it, open **System Settings → Privacy & Security** and choose **Open Anyway**.

## Controls

- Move: WASD or left stick
- Look: mouse or right stick
- Aim: right mouse or left trigger
- Fire: left mouse or right trigger
- Reload: R or controller face-left
- Shoulder swap: Q or D-pad right
- Sprint: Left Shift or left-stick click
- Crouch: C or controller face-right
- Weapons: number keys 1–6
- Pause: Escape or controller Menu/Options

## Included

- Third-person-only harbor mission blockout with insertion, warehouse, container lanes, crane route, optional manifest office, arena and extraction.
- Six selectable tuned weapons, automatic/semi fire handling, reload timing, ammunition, armor, damage and muzzle-obstruction validation.
- Native damageable enemies with perception, threat memory, advance, flank, retreat, suppression state, line-of-fire checks and ranged attacks.
- Objective progression, optional objectives, checkpoint recovery, mission-complete state and HUD guidance.
- World Partition map, bounded dynamic navigation, Mac scalability profiles and arm64 packaging.

## Verification

- UE 5.8 Turnkey Mac SDK verification: passed.
- Editor C++ build: passed.
- Automation: 6 passed, 0 failed.
- Content validation: 0 errors.
- Development build/cook/stage/package/archive: passed.
- Development NullRHI packaged startup: passed with no runtime error lines.
- Development Metal packaged startup: passed on Apple M4 Pro.
- Shipping build/cook/stage/package/archive: passed.
- Extracted Shipping app: arm64 verified; ad-hoc signature verified; sustained Metal launch completed without a blocking crash.

## Known limitations

- This is an ad-hoc-signed, unnotarized build. Gatekeeper approval is required.
- Physical Xbox-layout and PlayStation-layout controller certification still requires owner hardware testing.
- Base-M2 and M3-Pro performance tiers remain uncertified; only the M4 Pro host was exercised.
- Art, audio and effects include functional UE template/placeholders and are not a final commercial-quality content pass.
- Developer ID signing, notarization and App Store distribution are not included.
