# Architecture

## Module direction

`EmberCore` → `EmberGameplay` → `EmberAI`, `EmberMission`, `EmberUI`. Editor and test modules depend on runtime modules only.

`EmberCore` owns logging, shared value types, interfaces, settings, asset management and save contracts. `EmberGameplay` owns player/combat components. `EmberAI` owns perception and tactical decisions. `EmberMission` owns objectives/checkpoints. `EmberUI` observes event-driven state and never searches the world for gameplay actors.

## Runtime rules

- Immutable tuning uses Primary Data Assets and soft references.
- Transient weapon state belongs to runtime weapon instances/components.
- Damage and shot calculations use request/result value structs.
- Actor communication uses interfaces or delegates.
- Saves store stable identifiers and serializable values, never pointers.
- Timers and events are preferred to Tick; camera interpolation is the primary expected measured Tick user.
- Future multiplayer boundaries use authority-aware mutation entry points without enabling networking for the slice.

## Asset rules

Text source never fabricates `.uasset` or `.umap`. The Editor module will provide factories/commandlets for deterministic placeholder creation and validators. Unsupported automation is recorded in `MANUAL_EDITOR_STEPS.md`.

