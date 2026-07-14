# Project Ember 0.3.5-dev.1 Vehicle and Combat-Motion Pass

## What changed

- Replaced four primitive vehicle stand-ins with detailed, material-complete sports-car set dressing from Epic's UE 5.8 Chaos Modular Vehicle Examples. The cars are static cover/environment props; they are not presented as drivable or original final vehicle art.
- Corrected the player and enemy rifle presentation transform at `HandGrip_R`, removed the independent procedural reload offset that could separate the gun from the authored two-handed animation, and normalized weapon scale.
- Added recoverable camera recoil and restrained weapon-component kick so automatic fire settles instead of permanently drifting the camera or tearing the gun away from the hands.
- Rebuilt the procedural player and enemy rifle reports as layered 44.1 kHz crack/body/sub/tail sounds. Enemy reports retain spatial attenuation.
- Replaced the floaty jump setup with a heavier third-person profile: higher gravity, lower launch velocity, bounded air control, falling friction and falling braking. Jump now exits crouch and sprint before takeoff.
- Preserved the packaged mouse-capture correction, enemy-facing alignment, six persistent weapon slots, health/armor damage, enemy attacks and checkpoint recovery.

## Verification

- UE 5.8 Mac Editor arm64 compile: passed.
- Editor map regeneration: passed with zero Python errors; the licensed PBR vehicle assets are cooked through an Editor-authored map reference.
- Automation: 9 passed, 0 failed (`Saved/Automation/20260714T193412Z`), including the player grip transform and human jump profile.
- Content validation: 0 errors; one existing device-profile console-variable priority warning.
- Native Metal review: confirmed the detailed sports car, corrected forward weapon hold, active enemies and live health/armor damage.
- Clean Development cook/package/archive: passed (`Releases/Development/20260714T194452Z`).
- Fresh ZIP extraction: executable is thin arm64 and deep/strict ad-hoc-signature verification passed.
- Fresh packaged Metal run: mouse/look activation recorded `ignored=false`; synthetic HID input produced sustained automatic fire; all initial enemies initialized, target-facing alignment recorded `dot=1.000`, and enemy fire applied armor/health damage. This is automated runtime evidence, not a human or physical-controller playtest.
- ZIP SHA-256: `856fd59d0e2ca40394c8522ec63a2825fb0a967991efba732857cefb7f915fa0`.

## Controls

- Move: `W A S D`; sprint: `Left Shift`; crouch: `C` or `Left Ctrl`; jump: `Space`
- Look: mouse; aim: right mouse; fire/automatic fire: left mouse/hold; reload: `R`
- Shoulder swap: `Q`; weapons: `1`–`6` or mouse wheel; melee: `V`; interact: `E` or `F`
- Mission intel: `Tab`; controls overlay: `H`; pause/release mouse: `P` or `Escape`

## Honest limitations

This is a functional native Mac development vertical slice, not a finished AAA game. The four cars are licensed UE-only high-quality set-dressing assets, not bespoke destroyed vehicles and not drivable systems. Manny, Shooter Variant rifles, generated modular architecture, procedural audio and procedural effects remain disclosed development assets. Original final characters, weapon-specific hand/shoulder animation, production-recorded audio, authored jump/landing animation, photoreal environment art and physical controller certification remain incomplete.

## Gatekeeper

After extracting, Control-click the app and choose **Open**. If macOS still blocks it:

```bash
xattr -dr com.apple.quarantine "/path/to/ProjectEmber.app"
open "/path/to/ProjectEmber.app"
```
