# Project Ember 0.3.6-dev.1 Driveable Vehicle Pass

## What changed

- Made all four detailed harbor sports cars enterable and driveable using Unreal Engine 5.8's Chaos Modular Vehicle simulation: chassis, engine, clutch, transmission, suspension, wheels and skeletal wheel animation remain physically simulated.
- Added proximity entry through `E`, `F` or controller top button, player-to-car possession, a seated visible driver, a short entry debounce and capsule-tested exit beside the car.
- Added direct acceleration, braking, automatic reverse, steering and handbrake controls for keyboard/mouse and gamepad.
- Added an Ember-owned low third-person chase camera with collision, camera lag and bounded mouse/right-stick orbit. The first automated capture exposed the example camera beneath the road; the released camera path corrects that defect.
- Added a speed/drive HUD, compact vehicle control guide and a continuous spatial procedural engine tone driven by speed and throttle load.
- Preserved the existing on-foot shooter, combat AI, damage/armor, missions, checkpoint recovery, mouse-capture correction and enemy-facing correction after vehicle exit.

## Controls

On foot:

- Move: `W A S D`; interact/enter nearby car: `E` or `F`
- Look: mouse; aim: right mouse; fire/automatic fire: left mouse/hold; reload: `R`
- Sprint: `Left Shift`; crouch: `C`/`Left Ctrl`; jump: `Space`; shoulder: `Q`

Driving:

- Accelerate: `W` / right trigger
- Brake, then reverse once nearly stopped: `S` / left trigger
- Steer: `A D` / left stick
- Orbit chase camera: mouse / right stick
- Handbrake: `Space` / controller bottom button
- Exit beside the car: `E` or `F` / controller top button

## Verification

- UE 5.8 Mac Editor arm64 compile: passed.
- Editor map generation: passed with four persistent driveable vehicles and zero Python errors.
- Automation: 10 passed, 0 failed (`Saved/Automation/20260714T223219Z`).
- Content validation: 0 errors; one existing device-profile console-variable priority warning.
- Native Metal drive route: entry succeeded, throttle and steering were accepted, the real vehicle moved 229.1 cm, current road height was 28.3 cm, the chase camera captured correctly, and safe exit/re-possession succeeded.
- Development build/cook/package/archive passed at `Releases/Development/20260715T030000Z`.
- A fresh archive extraction is thin arm64 and passes deep/strict ad-hoc signature verification.
- The fresh packaged Metal drive route entered the nearest car, accepted throttle and steering, moved 270.7 cm at 4.0 KPH, captured the Ember-owned rear chase view, exited safely and shut down without a fatal error or assertion.
- Release ZIP SHA-256: `dfcb13242fa86daca6cd364b06a6e2064dc119633cb8afaf7a042895177ee996`.

## Honest limitations

This is a functional development vehicle feature, not bespoke final vehicle production. The car and vehicle simulation assets are licensed Epic UE-only example content. The seated mannequin pose, engine sound and HUD are functional original runtime systems; there is no authored door-opening mesh animation or final enter/exit montage. Physical Xbox-layout and PlayStation-layout controller execution remains pending representative hardware even though their input mappings are present and source-tested.

## Gatekeeper

After extracting, Control-click the app and choose **Open**. If macOS still blocks it:

```bash
xattr -dr com.apple.quarantine "/path/to/ProjectEmber.app"
open "/path/to/ProjectEmber.app"
```
