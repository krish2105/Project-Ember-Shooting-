# Project Ember 0.3.1-dev.1 Development Preview

## Purpose

This build replaces the visibly empty prototype-grid release with a regenerated native macOS harbor combat slice. It is a development preview, not a finished AAA production or a browser game.

## Rebuilt presentation

- Removed the duplicate template lighting stack that caused the previous flat orange/white exposure.
- Added 15 original Editor-generated procedural PBR materials for wet asphalt, limestone, damaged concrete, rubble, steel, rust, burnt metal, containers, water, vegetation, emissive fixtures and windows.
- Brought the ruined civic and industrial façades into the playable corridor.
- Added a damaged insertion checkpoint, staggered concrete cover, wrecked vehicles, road puddles, rubble and rebar, pipe bundles, lamps, work lights, fire pockets, a ruined crane and a distant civic landmark.
- Preserved a third-person-only camera and the existing tactical HUD, radar, objective and hostile-count presentation.

## Combat and launch corrections

- The rifle now attaches to the Shooter Variant `HandGrip_R` socket and follows the authored two-handed animation basis.
- Six gameplay weapon slots retain independent magazine and reserve ammunition; licensed template rifle/pistol meshes remain disclosed stand-ins.
- Aim, automatic fire, reload, weapon selection, damage, hostile hit reactions and checkpoint state remain connected to the existing authoritative gameplay path.
- A macOS launch-focus guard ignores the initial cursor warp for 250 ms and resets the camera to the authored horizon before accepting look input.

## Verification before publication

- UE 5.8 Mac Editor build: passed.
- Editor map/material generation: passed with no Python errors.
- Content validation: 0 errors.
- Automation: 8 passed, 0 failed, 0 warnings (`Saved/Automation/20260714T132502Z`).
- Native Metal visual smoke: stable third-person horizon, rebuilt environment, tactical HUD/radar, patrol targets and gripped weapon visible.
- The Development package, arm64/signature checks, fresh extraction and packaged input smoke are release gates and must be recorded after the archive is produced.

## Controls

- Move: `W A S D`
- Look: mouse
- Hold aim: right mouse button
- Fire / automatic fire: left mouse button / hold
- Reload: `R`
- Shoulder swap: `Q`
- Select weapon: `1`–`6`
- Pause / release mouse: `Escape`

## Known limitations

- Environment geometry is an original Editor-generated development kit, not final photogrammetry or commissioned hero art.
- The six slots use rifle/pistol presentation stand-ins; six original final weapon models and bespoke animation sets are not complete.
- Niagara surface effects, layered production audio, final character/enemy art, HLODs and full accessibility/settings screens remain incomplete.
- Xbox-layout, PlayStation-layout, base-M2 and M3-Pro certification remain pending physical hardware runs.
- The app is ad-hoc signed and unnotarized, so macOS Gatekeeper requires the documented local approval flow.

## Gatekeeper opening flow

After extracting the ZIP, Control-click the app and choose **Open**. If macOS still blocks it, run:

```bash
xattr -dr com.apple.quarantine "/path/to/ProjectEmber-Mac-Development.app"
open "/path/to/ProjectEmber-Mac-Development.app"
```
