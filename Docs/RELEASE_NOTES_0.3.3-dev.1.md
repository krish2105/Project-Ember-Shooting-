# Project Ember 0.3.3-dev.1 Combat Separation Preview

## What changed

- Fixed enemy/player capsule penetration that made the character arms, shoulders and rifles visually overlap.
- Replaced the close flank goal with a maintained 18.5 m tactical combat ring; enemies retreat below 11.5 m and do not fire below 9 m.
- Enabled pawn blocking, rifle visibility collision and local RVO avoidance for hostile combatants.
- Preserved advance, flank, retreat, patrol, cover-respecting gunfire, muzzle/tracer feedback, recoil, reload and six independent weapon slots.
- Fixed death recovery so residual movement is stopped and both health and armor are restored at the mission checkpoint.

## Verification

- UE 5.8 Mac Editor arm64 compile: passed.
- Automation: 9 passed, 0 failed (`Saved/Automation/20260714T170733Z`).
- Content validation: 0 errors; one existing device-profile console-variable priority warning.
- Live native Metal: after several minutes of engagement the nearest hostile remained separated at tactical range, the player retained a stable two-handed shoulder pose, the enemy health bar was visible, and incoming fire reduced player health from 100 to 94 plus visible armor loss.
- Clean Development cook/package/archive: passed (`Releases/Development/20260714T172801Z`).
- Fresh extraction: executable is thin arm64 and deep/strict ad-hoc-signature verification passed.
- Fresh packaged Metal run: enemies remained separated; logs recorded target hits applying 5.6 health plus 8.4 armor damage, full death, and health/armor checkpoint recovery without a blocking crash.
- ZIP SHA-256: `8f9c81c92f8c41456598b19f830acb72101a00fcda18b37f3a9ca7d624fba971`.
- Hosted artifact: [download the verified native Mac ZIP](https://github.com/krish2105/Project-Ember-Shooting-/releases/download/v0.3.3-dev.1/ProjectEmber-mac-arm64-Development.zip).
- Hosted verification: a fresh GitHub release download reproduced the local SHA-256 exactly.

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
