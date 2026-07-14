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

## Reference-quality over-the-shoulder combat slice

The supplied cinematic third-person firefight is a presentation reference, not a source of reusable characters, environments, animations, audio, UI or other proprietary content. Project Ember will reproduce the underlying interaction qualities with original or individually approved UE-licensed assets: a readable shoulder camera, convincing two-handed weapon handling, immediate shot feedback, reactive enemies, a deliberately lit indoor encounter and synchronized captions. The sequence remains interactive gameplay; it will not become a first-person camera or a copied scripted scene.

### Step 1 — Camera, targeting and input contract (M1)

- Keep exploration and combat strictly third person. Blend from the 310 cm exploration boom to an approximately 180–215 cm aim boom, with configurable shoulder offset and collision-safe framing.
- Hold right mouse/LT to aim and release to return to exploration. Toggle aim remains an accessibility option, not the only input path.
- Project the reticle through the camera, then solve the actual shot from the weapon muzzle toward the camera aim point. Reject or redirect a shot when the muzzle is obstructed.
- Add target-aware reticle states without hard aim-lock: neutral, valid hostile, obstructed and confirmed hit. Aim assist for controllers is a separate configurable system with friction and magnetism limits.
- Preserve input focus through window capture, pause, menus and device switching. Keyboard/mouse, Xbox-layout and PlayStation-layout mappings must share the same gameplay actions.
- Exit gate: aim-in/out, shoulder swap, camera collision, close-wall obstruction and focus restoration pass automation and packaged manual tests. The player, weapon and intended target remain readable at 16:9 and 16:10.

### Step 2 — Weapon presentation and animation synchronization (M2/M7)

- Replace shared template presentation with six distinct, fictional, license-cleared mission weapon meshes, materials, sockets and first-class data definitions. No real-world trademarks are required.
- Drive hands through authored rifle/pistol locomotion, aim offsets, turn-in-place, recoil additive poses and left-hand IK. Use weapon-specific grip and muzzle sockets; never attach presentation to the capsule.
- Implement equip, fire, empty-fire, staged reload, interrupted reload and weapon-swap montages. Gameplay ammunition changes occur at named notifies and are reconciled if an animation is cancelled.
- Add camera recoil, weapon recoil, procedural sway and recovery as separate bounded curves so accessibility can reduce camera motion without changing ballistic behavior.
- Exit gate: the weapon never intersects the torso in idle, locomotion, aim, fire or reload coverage; hands stay on their grips; ammunition, animation, audio and HUD remain synchronized through interruption and switching.

### Step 3 — Shot, ballistics and immediate feedback (M2/M6)

- Route every trigger pull through `FEmberShotRequest` and return `FEmberShotResult`, including origin, aim direction, muzzle obstruction, hit actor, surface, distance, damage and effect-routing identifiers.
- Support semi-automatic and automatic cadence, deterministic spread, range falloff, chamber state, tactical/empty reloads and per-slot ammunition persistence.
- Build pooled Niagara muzzle flash, smoke, tracer and impact effects. Add bounded point-light flashes and decals, with reduced-effects and low-quality fallbacks.
- Route layered gunshot, mechanical, tail and impact audio through `IEmberAudioRouter`; select indoor/outdoor tails using registered audio volumes rather than unrestricted world searches.
- Present a short hit marker and optional damage confirmation only when the authoritative damage result succeeds. Keep blood/gore optional; use non-gory sparks, cloth puffs and force reactions as the default presentation.
- Exit gate: one click produces one validated shot, held fire respects cadence, obstruction prevents wall shots, effects use the returned surface type, and no unbounded decals, components, projectiles or voices accumulate.

### Step 4 — Enemy targeting, damage and reaction chain (M2/M3/M7)

- Add capsule/body hit zones and optional head/armor zones using physical materials or validated bone mappings. Resolve armor, damage modifiers and death exactly once through `IEmberDamageable`.
- React to valid hits with directional additive hit reactions, brief suppression, threat-memory update and an interruption policy appropriate to the current tactical state.
- Blend authored reactions with physical animation for heavy impacts; reserve full ragdoll for death or explicitly budgeted states. Prevent repeated impulses from destabilizing navigation or collision.
- Enemies must perceive the shooter, seek cover, avoid ally fire lanes, return fire, flank and search the last-known position. Close-range targets remain readable and do not stand inert for presentation purposes.
- Exit gate: torso, armor and critical-zone tests return the expected damage; hit reactions face the impulse direction; death fires once; squad state changes are observable; friendly-fire and muzzle-obstruction checks pass.

### Step 5 — Authored indoor encounter and cinematic readability (M4/M5/M6)

- Build an original harbor-office/warehouse interior encounter with entrance, readable cover lanes, destructible-prop candidates, alternate flank route, objective target and an exit back into the harbor. Do not reproduce the reference room, characters or dialogue.
- Use approved modular environment assets, physically based materials, reflection captures, fog and motivated practical lighting. Maintain exposure continuity from exterior to interior and provide a conventional Mac rendering fallback.
- Add objective dialogue and subtitle/caption events through localized string tables. Captions include speaker, dialogue and important non-speech combat cues when enabled.
- Place bounded navigation, cover points, encounter volumes, audio volumes, checkpoint trigger and Data Layer membership. Include the encounter in HLOD/streaming and content validators.
- Exit gate: the room is fully collidable and lit, no player or AI falls through geometry, the objective can be completed by combat or the specified alternate route, subtitles scale correctly, checkpoint restore recreates the encounter deterministically, and exterior/interior transitions do not expose an unloaded void.

The indoor route includes an original two-flight stairwell inspired only by the supplied navigation/readability qualities: constrained shoulder-camera clearance, a landing that breaks line of sight, motivated green exit guidance, warm/cool light contrast, close-range pursuit and a transition into the office encounter. It must support traversal in both directions and cannot reuse the reference character, uniform, architecture, signage or encounter scripting.

### Step 6 — End-to-end verification and releasable evidence (M5–M9)

- Add functional tests for aim/fire/reload/swap, target damage, hit reaction, enemy response, captions, checkpoint restoration and encounter completion. Record explicit failures; do not convert missing evidence into a skip.
- Run packaged boot-to-results routes with keyboard/mouse plus physical Xbox-layout and PlayStation-layout controllers. Record model, build ID, macOS version and observed defects.
- Profile the encounter on the M4 Pro for game, render, GPU, animation, AI, Niagara, audio, streaming, hitch and peak-memory budgets. `MAC_BASELINE` must retain simpler lighting, shadows and effects rather than removing gameplay feedback.
- Perform a clean arm64 Development and Shipping cook, architecture check, ad-hoc signature verification, fresh extraction and insertion-to-extraction smoke test.
- Publish only after the downloaded archive checksum matches and the same archive passes the documented Gatekeeper launch flow. Release notes must list every placeholder, uncertified hardware tier and incomplete acceptance gate.
- Exit gate: all named automation passes, the packaged encounter is completed on every required input device, no blocking crash or progression failure occurs, M4 Pro measurements are attached, and the hosted artifact is the tested archive.

### Cross-step budgets and rollback

- Camera and target assistance, each recoil layer, physical animation, decals, smoke, dynamic lights and captions must have independent data/config switches.
- Maintain the current known-good rifle, basic damage path and blockout encounter as a rollback profile until the replacement path passes the same packaged tests.
- At 1080p `MAC_BASELINE`, combat presentation may reduce effect density and light duration but must preserve muzzle flash readability, shot direction, hit confirmation and enemy reaction.
- Work requiring unapproved marketplace content remains blocked at the asset gate. Functional placeholders stay visibly recorded in `Docs/ASSET_GAPS.md` and cannot satisfy the final-art exit gate.
