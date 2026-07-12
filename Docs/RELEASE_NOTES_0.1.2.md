# PROJECT EMBER 0.1.2 — macOS Input and Gameplay Fix

This release replaces the earlier Project Ember macOS archives.

## Included fixes

- Native Apple Silicon arm64 Shipping application.
- Persistent harbor floor, lighting, geometry and player spawn.
- Upright third-person character with idle, walk and jog playback.
- Visible weapon proxy and six selectable weapon configurations.
- Aim, fire, automatic fire, reload and weapon-selection input routes.
- Standard Unreal gameplay viewport plus direct and axis-based input fallbacks.
- Deferred Player 0 possession and game-only input activation.
- Recoil, muzzle flash and procedural gunshot feedback.

## Install

Extract `ProjectEmber-mac-arm64-Shipping.zip`. If Gatekeeper blocks the app, verify the checksum and run:

```bash
xattr -dr com.apple.quarantine "$HOME/Downloads/ProjectEmber-Mac-Shipping.app"
open "$HOME/Downloads/ProjectEmber-Mac-Shipping.app"
```

SHA-256: `a47baba383b07598bbe7b8e3df4d8aef8db2b44897710c706c0c3df6d0824216`

## Controls

- Move: WASD
- Look: mouse
- Aim: right mouse
- Fire: left mouse
- Reload: R after firing
- Weapons: 1–6
- Sprint: Left Shift
- Pause: Escape

## Scope

This is a functional gray-box vertical slice using Unreal template and generated placeholder presentation. It is not a final commercial art release.
