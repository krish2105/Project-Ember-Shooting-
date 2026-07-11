# PROJECT EMBER 0.1.1 — Harbor Visibility Fix

Native Apple Silicon Unreal Engine 5.8 tactical-shooter vertical slice.

## Important fix

This release replaces 0.1.0. It fixes the black gameplay view by adding a persistent packaged PlayerStart, persistent sky and directional lighting, atmospheric lighting, and harbor work lights. It also resolves the authored starter weapon directly in packaged builds instead of using the emergency fallback definition.

## Play

1. Download `ProjectEmber-mac-arm64-Shipping.zip` and its `.sha256` file.
2. Verify the ZIP with `shasum -a 256 ProjectEmber-mac-arm64-Shipping.zip`.
3. Extract the ZIP.
4. This build is ad-hoc signed and not notarized. Control-click the app, choose **Open**, then confirm **Open**.

If macOS still displays “Apple could not verify this app is free of malware,” verify the checksum first, then run:

```bash
xattr -dr com.apple.quarantine "$HOME/Downloads/ProjectEmber-Mac-Shipping.app"
codesign --verify --deep --strict "$HOME/Downloads/ProjectEmber-Mac-Shipping.app"
open "$HOME/Downloads/ProjectEmber-Mac-Shipping.app"
```

Expected ZIP SHA-256: `733e7613b0d1bc48ad262b62f7743b5af64abee0a1301e496354bcd09f21246c`.

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

- Third-person harbor mission blockout with insertion, warehouse, container lanes, crane route, optional manifest office, arena and extraction.
- Six selectable tuned weapons with firing, reload timing, ammunition, armor, damage and muzzle-obstruction handling.
- Native tactical enemies with perception, threat memory, advance, flank, retreat, suppression state, line-of-fire checks and attacks.
- Objective progression, optional objectives, checkpoint recovery, mission completion and HUD guidance.
- World Partition map, bounded dynamic navigation, Mac scalability profiles and arm64 packaging.

## Verification

- Editor C++ build: passed.
- Harbor map regeneration and content validation: passed with 0 errors.
- Corrected Development Metal build: visually verified with player, lighting, harbor geometry and mission objective visible.
- Shipping build/cook/stage/package/archive: passed.
- Extracted Shipping app: arm64 and ad-hoc signature verified.
- Extracted Shipping app sustained a native Metal launch without a blocking crash.

## Known limitations

- Gatekeeper approval is required because this build is not Developer-ID signed or notarized.
- Physical Xbox-layout and PlayStation-layout controller certification still requires representative hardware testing.
- Base-M2 and M3-Pro performance tiers remain uncertified; only the M4 Pro host was exercised.
- Art, audio and effects include functional UE template/placeholders and are not a final commercial-quality content pass.
