# Progress

## M0 — Audit and Foundation

- Status: Foundation complete for keyboard/mouse; compile, Editor generation, source automation, content validation, clean Development packaging and packaged combat-input verification pass. Physical controller certification remains pending hardware execution.
- Detected: M4 Pro arm64, 24 GB RAM, macOS 26.5.2.
- Toolchain: UE 5.8 at `~/Desktop/UE_5.8`; Xcode 26.6 and macOS SDK 26.5 accepted by Turnkey on 2026-07-14.
- Editor build: `ProjectEmberEditor Mac Development` succeeded after the player foundation correction.
- Tests: eight tests pass with no warnings or errors in `Saved/Automation/20260714T032907Z`, including walking, hold/toggle aim contracts, input mapping, bounded feedback components, damage direction, reload interruption and generated animation-reference assertions.
- Content validation: completed with 0 errors on 2026-07-14.
- Editor assets/maps: harbor map and Blueprint classes regenerated through Unreal Editor scripting. The generated GameMode explicitly selects `AEmberPlayerController` and the third-person pawn.
- Player foundation correction: removed forced flying, zero gravity, planar movement, forced possession and duplicate combat-input polling. Normal walking gravity, a single action-binding path and packaged game-focus controller are active.
- Animation correction: removed the missing template C++ parent dependency and single-node locomotion override. The generated mannequin now uses its available locomotion Animation Blueprint.
- Packaging correction: explicit Ember cook inclusion places all twelve weapon Primary Data Assets in the arm64 app. The current corrected build is `Releases/Development/20260714T010932Z`, SHA-256 `a003481244aa55c1f069d9c52a9ac09d116cbb67dc3afa1c085ec2f912257f69`.
- Combat feedback correction: aim is hold-to-aim by default with an optional accessibility toggle and smooth 86/70-degree FOV/shoulder-camera blending. Reusable muzzle, tracer, impact-light and gunshot-audio components replace per-shot object allocation. The hostile reticle and damage-only hit marker are implemented. Three visible marked hostiles form the insertion patrol.
- Packaged keyboard/mouse evidence: a fresh extraction loaded the harbor, then synthetic macOS HID events produced runtime records `Player aim toggled: AIM`, `Player weapon fired; magazine=29 reserve=180`, and `Player reload requested: STARTED`. This proves the packaged bindings/state transitions without mislabeling it as a human playtest.
- Packaged switching evidence: slot 1 fired to 29, slot 2 equipped/fired to 29, and slot 1 re-equipped at 29 before reload started. This proves that 1–6 selection does not refill a previously used slot.
- Remaining M0 certification evidence: physical Xbox-layout and PlayStation-layout controller records.
- Presentation correction (2026-07-14): the capsule-mounted primitive silhouette was removed. Epic's UE 5.8 Shooter Variant third-person rifle graph and modeled rifle/pistol resources are installed through an Editor script, the weapon follows Manny's `hand_r` bone, and a live Metal preview confirmed two-handed combat locomotion with a forward-oriented weapon. The camera now uses a tighter 310 cm exploration / 215 cm aim boom and 86/70 degree FOV pair.
- Night-harbor presentation pass (2026-07-14): the generated mission now uses an original moonlit teal/orange lighting direction with volumetric fog, 8 skyline towers, facade strips, 14 route lights, five objective beacons, barriers, pipe stacks and sparse edge vegetation. The responsive HUD includes health, armor, ammo, phase/objective, living-hostile count, six-slot selection, timed reload feedback, hostile reticle, hit marker and player-relative radar.
- Harbor replacement pass (2026-07-14): removed the duplicated Open World template sun/sky/fog/post-process stack, replaced prototype grid assignments with 15 Editor-generated procedural PBR materials, and rebuilt the insertion corridor with closer ruined façades, a damaged checkpoint, road cover, wrecks, puddles, rubble/rebar, pipes, street fixtures, a crane silhouette and a civic skyline anchor. The generated rifle now uses the Shooter Variant `HandGrip_R` socket rather than the raw hand bone. A 250 ms launch-focus guard rejects the first macOS cursor warp and resets the mission to a level third-person horizon before look input is enabled.
- Verification for the harbor replacement pass: Mac Editor compiled, the World Partition map regenerated through the Editor with no Python errors, content validation completed with 0 errors, and `Saved/Automation/20260714T132502Z` contains 8 passed / 0 failed / 0 warnings. A native Metal frame confirmed a stable horizon, dense insertion composition, tactical HUD/radar, three visible patrol targets and the modeled weapon in the two-handed pose. Development package `Releases/Development/20260714T133626Z` cooked and archived successfully; a fresh extraction passed arm64 and ad-hoc-signature checks. Synthetic packaged input recorded aim, full-magazine reload rejection, slot-2 selection, firing and successful reload start. This is automation evidence, not a human or physical-controller playtest.

## Remaining milestones

### M1–M2 — Player and Combat

- In progress. Six selectable weapon slots now preserve independent magazine/reserve state, preventing switch-to-refill ammunition duplication.
- Weapon definitions drive reload duration, recoil and aim/hip spread. Six functional slots use licensed UE-only template rifle/pistol presentation, and the packaged input path is proven in the prior build.
- Shared licensed rifle/pistol fire and reload sequences are now data-driven and triggered through dynamic montages; visible recoil and a timed reload pose remain synchronized with authoritative ammunition state.
- Remaining: original final weapon art and bespoke animation sets for all six slots, chambering edge cases, full surface routing and physical controller certification.

### M3–M4 — AI and Harbor Mission

- In progress. The insertion patrol now consists of three nearby, marked damageable tactical enemies; the first-patrol objective requires their elimination rather than player position alone.
- Versioned checkpoint capture/restore now includes player transform, health, armor, per-slot ammunition and objective state, with on-disk save/load at the midpoint.
- Remaining: full encounter-by-encounter packaged playthrough, cover/EQS validation, checkpoint migration/corruption recovery, extraction/results acceptance and navigation/HLOD/Data Layer validation.

M5–M9 remain incomplete. Prototype systems or configuration may exist, but none are certified until their acceptance tests pass in a packaged build.

## External environment asset pipeline

- Added a dry-run-by-default Meshy Text-to-3D v2 client for the approved ruined pillar, crane ruin and rubble pile prompts. It implements preview/refine polling, PBR/HD texture requests, resumable task IDs, FBX/texture downloads and SHA-256 manifests without adding a runtime dependency.
- Added a Blender 5.1 staging script that imports the three generated FBX/PBR sets, builds Principled materials, instances the requested layout and renders a 1280 × 720 review frame while preserving unrelated scene collections.
- Validation on 2026-07-14: both Python files compile; the dry-run emitted all three bounded requests; the real execution gate correctly rejected a missing `MESHY_API_KEY`; and Blender 5.1.2 completed an offline end-to-end import/material/placement/save/render smoke test using disposable primitives under `/tmp`. A separate missing-input run correctly returned non-zero. Unreal Turnkey accepted the Mac SDK and content validation completed with 0 errors (one existing console-variable priority warning). The disposable smoke fixtures are test evidence only and are not project or Meshy assets.
- Connection status: no Blender, Unity, Unreal or Meshy MCP tool is registered in the active Codex session. A Blender 5.1.2 GUI process is open but has no Codex MCP tool/listening socket, Unity Editor is not running, and host `blender-mcp` processes belong to a separate Claude session. No live scene object list, Unity hierarchy, paid Meshy task, downloaded model, real-asset preview render or Unreal import is therefore claimed.
- Full procedure and intake gates are recorded in `Docs/GENERATED_ASSET_PIPELINE.md`.

## Reference-quality combat slice request

- Planned on 2026-07-14 as six work packages in `Docs/IMPLEMENTATION_PLAN.md`: camera/targeting, weapon animation, shot feedback, enemy reactions, an original indoor encounter, and release validation.
- The supplied cinematic firefight image is used only to identify interaction and presentation qualities. No proprietary character, room, animation, audio, dialogue or UI from the reference has been imported or approved.
- Current status: the known-good rifle input/damage path and two-handed template stance are foundations only. The new indoor encounter, six final weapon presentations, full Niagara/audio stack, physical hit reactions, localized captions and complete device/performance evidence are not implemented and must not be reported as complete.
- Stairwell/weapon architecture work started: the generated harbor replaces the solid security-office block with a collidable two-flight stairwell shell, landing, rails, exit marker and bounded lights. Runtime presentation now accepts a separate mesh and transform for every weapon slot. Because only one approved rifle and one pistol mesh are currently available, the generated array intentionally uses tracked stand-ins and does not satisfy the six-final-weapon art gate.
- Verification for this increment: canonical Mac Editor build passed, Editor generation completed with 0 errors, content validation completed with 0 errors, and automation report `Saved/Automation/20260714T021955Z` contains 8 passed / 0 failed / 0 warnings. A live Metal teleport preview reached the interior, but the center position forced very tight camera collision; entrance-to-exit traversal, normal-route framing and AI navigation remain uncertified.
- Combat-runtime pass (2026-07-14): stable shot/damage contracts now carry impact point, incoming direction, applied damage and kill state. Tactical enemies play a bounded directional flinch for damaging hits and retain ragdoll only for death. Mission/HUD encounter counts use living health state and cached actor sets so defeated enemies advance objectives immediately without per-frame world scans.
- Verification for the combat-runtime pass: Mac Editor compiled, content validation completed with 0 errors, and `Saved/Automation/20260714T024604Z` contains 8 passed / 0 failed / 0 warnings. A fresh committed Development package and packaged interaction smoke test are required before publishing the next download.
