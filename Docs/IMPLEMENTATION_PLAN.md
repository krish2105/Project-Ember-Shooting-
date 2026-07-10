# Implementation Plan

The approved execution plan is divided into milestones M0–M9. Each milestone must remain compilable and independently reviewable.

| Milestone | Deliverable | Exit gate |
|---|---|---|
| M0 | Audit, toolchain, source foundation, scripts and documentation | UE SDK verified; Editor/Game build and Development package pass |
| M1 | Third-person player, camera and Enhanced Input | Keyboard/mouse and both controller layouts pass functional tests |
| M2 | Data-driven weapons and damage | Six mission weapons pass fire/reload/swap/obstruction tests |
| M3 | Tactical AI | Four mission archetypes demonstrate patrol, cover, engage, flank and search |
| M4 | Harbor mission | Insertion-to-extraction and checkpoint restore pass |
| M5 | UI, settings, accessibility and saves | Boot-to-results works with controller and keyboard/mouse |
| M6 | Audio, VFX and surfaces | Indoor/outdoor and twelve surface responses pass budgets |
| M7 | Animation and combat polish | Animation, weapon state, audio and UI stay synchronized |
| M8 | Mac optimization | M4 Pro benchmark evidence recorded; other tiers clearly uncertified |
| M9 | Package and hosted delivery | Downloaded arm64 archive passes documented smoke route |

Each milestone will record dependencies, automated and manual tests, performance evidence, risks and rollback notes as it executes. Binary content work begins only after the UE 5.8 Editor build succeeds.
