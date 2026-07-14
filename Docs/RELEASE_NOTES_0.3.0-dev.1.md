# Project Ember 0.3.0-dev.1 Development Preview

This is a native Apple Silicon Unreal Engine 5.8 Development preview of the original Project Ember harbor vertical slice. It is not a browser game and it is not a claim of final AAA art completion.

## New in this build

- Original moonlit industrial-harbor presentation with teal/orange combat lighting, fog, skyline silhouettes, illuminated route markers, denser cover, pipes and edge vegetation.
- Responsive tactical HUD with health, armor, ammunition, mission phase, objective, living-hostile counter, six-slot selector, reload progress, contextual reticle, hit marker and local radar.
- Shared licensed UE template rifle/pistol fire and reload animation sequences driven from weapon Primary Data Assets.
- Visible weapon recoil and timed reload pose synchronized to authoritative ammunition state.
- Tactical reload interruption when ammunition remains, while empty reloads keep firing blocked until completion.
- Existing third-person-only camera, six persistent ammunition slots, damage, enemy flinch, checkpoints, mission route and extraction remain active.

## Verified before packaging

- UE Turnkey reports the Mac SDK valid.
- ProjectEmberEditor arm64 Development compile passes.
- Editor-only data and map generation complete with zero errors.
- Content validation completes with zero errors.
- Eight Project Ember automation tests pass in `Saved/Automation/20260714T032907Z` with zero failed, skipped or unrun tests.

## Controls

- Move: `W A S D` or left stick
- Look: mouse or right stick
- Aim: hold right mouse or left trigger
- Fire: left mouse or right trigger
- Reload: `R` or controller face-left
- Shoulder swap: `Q` or D-pad right
- Weapons: `1`–`6`
- Pause/resume: `Escape` or Menu

## macOS Gatekeeper

The archive is ad-hoc signed and not notarized. After extracting, control-click the app and choose **Open**. If macOS still blocks it, run:

```bash
xattr -dr com.apple.quarantine "/path/to/ProjectEmber-Mac-Development.app"
open "/path/to/ProjectEmber-Mac-Development.app"
```

## Known limitations

- The six gameplay slots currently share one approved UE rifle mesh/animation set and one pistol mesh/animation set; six original final weapon rigs remain an explicit asset gap.
- Environment geometry uses original generated composition with engine/template materials; final modular art, surface-specific Niagara, authored audio layers, HLODs and complete accessibility/settings UI remain later milestones.
- Physical Xbox-layout and PlayStation-layout end-to-end certification is pending owner hardware execution.
- Performance is measured only on the M4 Pro host; base-M2 and M3-Pro certification is not claimed.
- This Development preview is ad-hoc signed, Developer-ID-unnotarized and expected to require the documented Gatekeeper flow.
