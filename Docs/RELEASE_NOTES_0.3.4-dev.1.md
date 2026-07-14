# Project Ember 0.3.4-dev.1 Input and Enemy-Facing Fix

## What changed

- Fixed the packaged-startup mouse-look regression. `BeginPlay` and `OnPossess` could each add a stack-based look-input lock while the delayed activation removed only one; startup now normalizes the lock stack before and after the focus guard.
- Kept permanent game-only mouse capture and added runtime evidence when mouse delta reaches the controlled character.
- Fixed backwards hostiles by explicitly aligning controller yaw, actor forward, rifle direction and the standard Manny mesh basis toward the combat target on every tactical decision.
- Preserved tactical spacing, player/enemy damage, armor, death recovery, recoil, reload, automatic fire and all six persistent weapon slots.

## Verification

- UE 5.8 Mac Editor arm64 compile: passed.
- Automation: 9 passed, 0 failed (`Saved/Automation/20260714T181322Z`), including mouse-axis mappings and the hostile controller/mesh orientation contract.
- Content validation: 0 errors; one existing device-profile console-variable priority warning.
- Live native Metal: gameplay activation recorded `ignored=false`; real mouse delta reached the character; four spawned enemies independently recorded a target-facing dot product of `1.000`.
- Clean Development cook/package/archive: passed (`Releases/Development/20260714T183000Z`).
- Fresh extraction: executable is thin arm64 and deep/strict ad-hoc-signature verification passed.
- Fresh packaged Metal run: the harbor loaded, gameplay activation recorded `ignored=false`, post-activation mouse movement recorded a yaw delta, and all three initial patrol enemies recorded target-facing dot products of `1.000`; no fatal error or assertion was recorded.
- ZIP SHA-256: `2dd901c9acdb7fba8e31c614da4f678f54a23258b54a7d6d66cf8aefd8b95801`.
- Hosted artifact: [download the verified native Mac ZIP](https://github.com/krish2105/Project-Ember-Shooting-/releases/download/v0.3.4-dev.1/ProjectEmber-mac-arm64-Development.zip).

## Controls

- Move: `W A S D`; sprint: `Left Shift`; crouch: `C` or `Left Ctrl`; jump: `Space`
- Look: mouse; aim: right mouse; fire/automatic fire: left mouse/hold; reload: `R`
- Shoulder swap: `Q`; weapons: `1`–`6` or mouse wheel; melee: `V`; interact: `E` or `F`
- Mission intel: `Tab`; controls overlay: `H`; pause/release mouse: `P` or `Escape`

## Honest limitations

This is a functional native Mac development vertical slice, not a finished AAA game. Manny, Shooter Variant rifles, generated modular architecture and procedural effects are disclosed development assets. Original final characters, six bespoke weapon rigs, production animation/VFX/audio, photoreal environment art, physical controller certification and base-M2/M3-Pro performance evidence remain incomplete.

## Gatekeeper

After extracting, Control-click the app and choose **Open**. If macOS still blocks it:

```bash
xattr -dr com.apple.quarantine "/path/to/ProjectEmber-Mac-Development.app"
open "/path/to/ProjectEmber-Mac-Development.app"
```
