# Project Ember 0.3.9-dev.1

## Vehicle control fix

- Stops the car/camera from rotating on vehicle entry.
- Clears retained throttle, steering, reverse, brake and chassis velocity before giving control to the player.
- Prevents the example vehicle Blueprint from overwriting Ember's W/S/A/D input.
- Keeps held throttle and steering authoritative throughout the physics tick.
- Holds the handbrake at entry until the player presses W or S.
- Bounds the third-person chase-camera orbit to the rear hemisphere.

## Controls

- `W` / right trigger: accelerate
- `S` / left trigger: brake, then reverse
- `A` / `D` / left stick: steer left/right
- `Space` / controller bottom button: handbrake
- Mouse / right stick: orbit chase camera
- `E` or `F` / controller top button: enter or exit

## Verification

- Mac arm64 game target compiled and linked.
- 10 Project Ember automation tests passed with zero failures or warnings in `Saved/Automation/20260715T122435Z`.
- Native Metal vehicle smoke: zero idle yaw drift, right/left/center steering commands accepted, 11.2 m forward travel, safe exit and on-foot input restoration.

This Development build is ad-hoc signed and not notarized. Physical Xbox and PlayStation controller certification is still pending owner hardware testing.
