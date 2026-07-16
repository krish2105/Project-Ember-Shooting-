# Project Ember 0.3.2-dev.1 Combat Preview

## What changed

- Corrected the rifle/hand/shoulder presentation for player and enemies using Manny's authored grip socket and rifle animation graph.
- Added armed tactical enemies that patrol, acquire the player, advance, flank, retreat, fire spatial gunshots and respect world cover.
- Added visible enemy muzzle flashes and tracers, hit/miss behavior, armor and health damage, enemy health bars and player damage feedback.
- Strengthened recoil and retained authoritative automatic fire, ammunition, reload and six independent weapon states.
- Added interaction (`E`/`F`), melee (`V`), mouse-wheel weapon switching, crouch (`Left Ctrl`), mission intel (`Tab`), controls (`H`) and pause (`P`/`Escape`).
- Expanded the HUD and pause overlay with control guidance, mission state and combat readability.

## Verification

- UE 5.8 Mac Editor arm64 compile: passed.
- Automation: 8 passed, 0 failed, 0 warnings (`Saved/Automation/20260714T161653Z`).
- Content validation: 0 errors; one existing device-profile console-variable priority warning.
- Live native Metal combat: 15 enemies received `EmberAIController`; enemy fire produced blocked shots, open misses and target hits. The first recorded hit applied 8.4 armor plus 5.6 health damage.
- Package, fresh extraction, architecture, signing and hosted-download checks are recorded when the release artifact is published.

## Controls

- Move: `W A S D`; sprint: `Left Shift`; crouch: `C` or `Left Ctrl`; jump: `Space`
- Look: mouse; aim: right mouse; fire/automatic fire: left mouse/hold; reload: `R`
- Shoulder swap: `Q`; weapons: `1`–`6` or mouse wheel; melee: `V`; interact: `E` or `F`
- Mission intel: `Tab`; controls overlay: `H`; pause/release mouse: `P` or `Escape`

## Honest limitations

This is a functional native Mac development vertical slice, not a finished AAA game. The environment remains an original Editor-generated modular development kit; final commissioned character, weapon, animation, VFX, audio and photoreal environment production are incomplete. Physical Xbox/PlayStation controller and base-M2/M3-Pro certification remain pending. The app is ad-hoc signed and unnotarized.

## Gatekeeper

After extracting, Control-click the app and choose **Open**. If macOS still blocks it:

```bash
xattr -dr com.apple.quarantine "/path/to/ProjectEmber-Mac-Development.app"
open "/path/to/ProjectEmber-Mac-Development.app"
```
