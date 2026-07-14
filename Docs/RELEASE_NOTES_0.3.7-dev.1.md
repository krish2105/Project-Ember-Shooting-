# Project Ember 0.3.7-dev.1 Input Focus Hotfix

## Fixed

- Removed the launch-time stack-based look lock that could leave an Editor-hosted session visible but unable to accept movement or mouse look.
- Normalized both movement and look ignore stacks on startup, character possession, vehicle possession and vehicle exit.
- Explicitly registers and focuses the Unreal game viewport, uses permanent mouse capture with lock-on-capture, and does not consume the first capture click.
- Added one-time movement-input diagnostics alongside the existing mouse-look and vehicle-input diagnostics.
- Retained the complete 0.3.6 vehicle feature: proximity entry, seated visible driver, Chaos acceleration/braking/reverse/steering/handbrake, chase camera, engine feedback and safe exit.

## Controls

Click once inside the game window after launching.

On foot: `W A S D` move, mouse look, right mouse aim, left mouse fire, `R` reload, `E` or `F` enter a nearby car.

Driving: `W` accelerate, `S` brake/reverse, `A D` steer, mouse orbit, `Space` handbrake, `E` or `F` exit.

## Verification

- UE 5.8 Mac Editor arm64 compile passed.
- Automation `Saved/Automation/20260714T234238Z`: 10 passed, 0 failed.
- Content validation: 0 errors and one existing device-profile priority warning.
- Fresh packaged startup reported `lookIgnored=false`, `moveIgnored=false`, `viewportFocused=true` after vehicle possession and again after exit.
- Fresh packaged vehicle route entered, accepted throttle and steering, moved 279.6 cm at 4.0 KPH, captured the correct chase camera and exited without a fatal error or assertion.
- The extracted app is thin arm64 and passes deep/strict ad-hoc-signature verification.
- ZIP SHA-256: `36d4f9c0a739abb91ac5787158e05039f5be6f512f91ab483b729b257c7aa5a0`.

## Gatekeeper

After extracting, Control-click `ProjectEmber.app` and select **Open**. If macOS still blocks it:

```bash
xattr -dr com.apple.quarantine "/path/to/ProjectEmber.app"
open "/path/to/ProjectEmber.app"
```

This remains an ad-hoc-signed Development build. Physical controller certification and an authored vehicle door animation remain pending.
