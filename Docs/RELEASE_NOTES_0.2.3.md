# Project Ember 0.2.3 Development Preview

## What changed

- Restored the approved hold-to-aim contract for right mouse and left trigger. Toggle aim remains available as an explicit accessibility option.
- Added smooth exploration-to-aim spring-arm, shoulder-offset and field-of-view blending.
- Replaced per-shot tracer, impact-light and gunshot-wave allocation with reusable bounded components.
- Extended shot/damage results with impact direction, point, applied damage and kill state.
- Added a short directional enemy hit reaction; death still uses the existing bounded ragdoll path.
- Changed hit markers to confirm damage rather than any world collision.
- Cached encounter actors and count living health state so defeated enemies advance objectives without repeated mission world scans.
- Updated the in-game control guide to say `HOLD RMB / LT AIM`.

## Verified before packaging

- UE 5.8 Turnkey accepts the installed Mac SDK.
- `ProjectEmberEditor Mac Development` compiles successfully.
- Content validation: 0 errors.
- Automation report `Saved/Automation/20260714T024604Z`: 8 passed, 0 failed, 0 skipped, 0 warnings.

## Controls

- Move: `WASD` or left stick
- Look: mouse or right stick
- Aim: hold right mouse or left trigger
- Fire: left mouse or right trigger
- Reload: `R` or controller face-left
- Shoulder swap: `Q` or D-pad right
- Weapons: `1` through `6`
- Pause: `Esc` or Menu

## macOS opening instructions

This preview is ad-hoc signed and not notarized. After extracting the ZIP, first try Control-clicking `ProjectEmber.app`, choosing **Open**, then confirming **Open**. If macOS still blocks the verified archive, run:

```bash
xattr -dr com.apple.quarantine "$HOME/Downloads/ProjectEmber.app"
open "$HOME/Downloads/ProjectEmber.app"
```

## Honest limitations

- This remains a functional vertical-slice preview, not a completed AAA game.
- Only one approved rifle mesh and one approved pistol mesh are present; six slots use tracked stand-ins pending four additional approved models and weapon-specific rigs/montages.
- The harbor and stairwell are generated gameplay blockouts, not final rendered modular environments.
- Final Niagara, twelve-surface responses, layered MetaSounds, captions/dialogue, full Common UI, cover/EQS certification, HLOD/Data Layer certification and packaged insertion-to-extraction evidence remain incomplete.
- Physical Xbox-layout and PlayStation-layout testing, base-M2/M3-Pro certification, Developer ID signing and notarization remain outstanding external gates.
