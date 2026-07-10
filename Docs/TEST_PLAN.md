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

