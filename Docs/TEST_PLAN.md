# Test Plan

## Automation groups

- `ProjectEmber.Core`: value validation, versioning and deterministic calculations.
- `ProjectEmber.Player`: movement states, aim, shoulder swap, collision and interaction.
- `ProjectEmber.Combat`: definitions, ammunition, fire cadence, reload stages, damage and obstruction.
- `ProjectEmber.AI`: perception, cover, friendly-fire avoidance, suppression and search timeout.
- `ProjectEmber.UI`: focus, glyphs, remapping, modals and settings persistence.
- `ProjectEmber.Save`: new/load/migrate/missing/corrupt/checkpoint cases.
- `ProjectEmber.Map`: required actors, navigation, streaming, HLOD, surfaces and audio zones.

## Manual/package matrix

Run keyboard/mouse, Xbox-layout and PlayStation-layout from boot through results. Record controller model, macOS, UE version, build ID and observed failures. Run all eight benchmark routes in packaged Development and representative smoke tests in Shipping.

Skipped tests remain visible failures of evidence and block a release claim.

## Over-the-shoulder combat acceptance matrix

| Area | Automated evidence | Packaged/manual evidence |
|---|---|---|
| Aim camera | Aim state, boom/FOV target, shoulder side, collision recovery | Player, weapon and target readable at 16:9 and 16:10 |
| Shot alignment | Camera ray, muzzle ray, obstruction and single-shot accounting | Reticle and muzzle converge on near, medium and long targets |
| Weapon state | Cadence, chamber, magazine/reserve, staged reload and swap persistence | Fire, empty, reload, cancel and switch remain synchronized |
| Presentation | Effect routing IDs, pool limits and reduced-effects selection | Muzzle flash, smoke, tracer, impact and hit marker are visible but bounded |
| Damage/reaction | Armor/body/critical modifiers, death-once and reaction direction | Target visibly reacts, enters tactical response and does not remain inert |
| Indoor encounter | Required volumes, collision, navigation, Data Layers, audio and checkpoint participants | Entry-to-exit route, alternate route, captions and restore complete without a void |
| Input devices | Action mappings, focus restore and binding conflict detection | Keyboard/mouse, Xbox-layout and PlayStation-layout each complete the encounter |
| Release | Asset validation, automation result parsing, arm64 and signature checks | Fresh hosted download extracts, launches and completes the smoke route |

## Driveable vehicle acceptance matrix

| Area | Automated evidence | Packaged/manual evidence |
|---|---|---|
| Access | Driveable tag, interaction mappings, possession and occupied-seat contract | `E`, `F` or controller top button enters only a nearby unoccupied car |
| Simulation | Modular chassis/engine/transmission/suspension/wheel component exists | Car accelerates, steers, brakes, reverses and handbrakes on the harbor road |
| Camera | Ember chase-camera component, bounded orbit pitch and collision probe | Low third-person view stays outside geometry and mouse/right-stick orbit remains responsive |
| Driver | Source mannequin is hidden while the seated pose clone is present | Driver is contained in the cabin and the original third-person character returns after exit |
| Exit | Input debounce and capsule-overlap candidate selection | `E`, `F` or controller top button exits beside the stopped car without clipping |
| Feedback | Speed, control-state and engine-load values are bounded | Vehicle HUD, speed, control guide and spatial engine tone respond while driving |
| Runtime | Native Metal smoke enters, applies Chaos inputs, moves, captures and exits | Fresh packaged build repeats the route without fatal error or below-map camera |

The reference-quality slice is not accepted from a screenshot alone. Evidence requires state assertions, packaged interaction, performance capture and a fresh-download run.
