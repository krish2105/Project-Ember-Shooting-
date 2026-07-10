# PROJECT EMBER Repository Instructions

## Scope

These instructions apply to the entire repository.

## Safety and truthfulness

- Never fabricate, hand-author, or claim success for Unreal binary assets (`.uasset`, `.umap`).
- Create binary assets only through Unreal Editor, a commandlet, an Editor factory, or a documented manual Editor operation.
- Never claim that a build, test, package, controller run, performance capture, signing check, or hosted download passed unless its output was observed.
- Preserve user work and do not use destructive source-control operations.
- Do not add telemetry, network services, secrets, or unreviewed third-party dependencies.

## Architecture

- Keep the runtime dependency direction acyclic: `EmberCore` → `EmberGameplay` → (`EmberAI`, `EmberMission`, `EmberUI`).
- `EmberEditor` and `EmberTests` may depend on runtime modules; runtime modules must not depend on them.
- Prefer components, interfaces, Gameplay Tags, Primary Data Assets, soft references, events, and timers.
- Do not add hard-coded content paths to gameplay logic or introduce unrestricted actor searches/ticking.
- All player cameras must remain third-person.

## Workflow

- Re-read the relevant files in `Docs/` at the start of each milestone.
- Compile and run relevant tests before marking a milestone complete.
- Update `Docs/PROGRESS.md`, `Docs/ASSET_GAPS.md`, and `Docs/RISK_REGISTER.md` after each milestone.
- Keep generated directories (`Binaries`, `DerivedDataCache`, `Intermediate`, `Saved`, staged builds) out of Git.
