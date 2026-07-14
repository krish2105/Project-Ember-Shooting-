# Progress

## M0 — Audit and Foundation

- Status: Foundation complete for keyboard/mouse; compile, Editor generation, source automation, content validation, clean Development packaging and packaged combat-input verification pass. Physical controller certification remains pending hardware execution.
- Detected: M4 Pro arm64, 24 GB RAM, macOS 26.5.2.
- Toolchain: UE 5.8 at `~/Desktop/UE_5.8`; Xcode 26.6 and macOS SDK 26.5 accepted by Turnkey on 2026-07-14.
- Editor build: `ProjectEmberEditor Mac Development` succeeded after the player foundation correction.
- Tests: seven tests pass with no warnings or errors in `Saved/Automation/20260714T004341Z`, including walking, aim-state, input mapping and weapon-visual assertions.
- Content validation: completed with 0 errors on 2026-07-14.
- Editor assets/maps: harbor map and Blueprint classes regenerated through Unreal Editor scripting. The generated GameMode explicitly selects `AEmberPlayerController` and the third-person pawn.
- Player foundation correction: removed forced flying, zero gravity, planar movement, forced possession and duplicate combat-input polling. Normal walking gravity, a single action-binding path and packaged game-focus controller are active.
- Animation correction: removed the missing template C++ parent dependency and single-node locomotion override. The generated mannequin now uses its available locomotion Animation Blueprint.
- Packaging correction: explicit Ember cook inclusion places all twelve weapon Primary Data Assets in the arm64 app. The current corrected build is `Releases/Development/20260714T010932Z`, SHA-256 `a003481244aa55c1f069d9c52a9ac09d116cbb67dc3afa1c085ec2f912257f69`.
- Combat feedback correction: aim is a capture-safe toggle with a 68-degree FOV transition; the functional weapon silhouette, muzzle flash, tracer, impact light, hostile reticle and hit marker are implemented. Three visible marked hostiles form the insertion patrol.
- Packaged keyboard/mouse evidence: a fresh extraction loaded the harbor, then synthetic macOS HID events produced runtime records `Player aim toggled: AIM`, `Player weapon fired; magazine=29 reserve=180`, and `Player reload requested: STARTED`. This proves the packaged bindings/state transitions without mislabeling it as a human playtest.
- Packaged switching evidence: slot 1 fired to 29, slot 2 equipped/fired to 29, and slot 1 re-equipped at 29 before reload started. This proves that 1–6 selection does not refill a previously used slot.
- Remaining M0 certification evidence: physical Xbox-layout and PlayStation-layout controller records.
- Presentation correction (2026-07-14): the capsule-mounted primitive silhouette was removed. Epic's UE 5.8 Shooter Variant third-person rifle graph and modeled rifle/pistol resources are installed through an Editor script, the weapon follows Manny's `hand_r` bone, and a live Metal preview confirmed two-handed combat locomotion with a forward-oriented weapon. The camera now uses a tighter 310 cm exploration / 215 cm aim boom and 86/70 degree FOV pair.

## Remaining milestones

### M1–M2 — Player and Combat

- In progress. Six selectable weapon slots now preserve independent magazine/reserve state, preventing switch-to-refill ammunition duplication.
- Weapon definitions drive reload duration, recoil and aim/hip spread. Six functional slots use licensed UE-only template rifle/pistol presentation, and the packaged input path is proven in the prior build.
- Remaining: original final weapon art for all six slots, fire/reload montage synchronization, chambering edge cases, full surface routing and physical controller certification.

### M3–M4 — AI and Harbor Mission

- In progress. The insertion patrol now consists of three nearby, marked damageable tactical enemies; the first-patrol objective requires their elimination rather than player position alone.
- Versioned checkpoint capture/restore now includes player transform, health, armor, per-slot ammunition and objective state, with on-disk save/load at the midpoint.
- Remaining: full encounter-by-encounter packaged playthrough, cover/EQS validation, checkpoint migration/corruption recovery, extraction/results acceptance and navigation/HLOD/Data Layer validation.

M5–M9 remain incomplete. Prototype systems or configuration may exist, but none are certified until their acceptance tests pass in a packaged build.

## Reference-quality combat slice request

- Planned on 2026-07-14 as six work packages in `Docs/IMPLEMENTATION_PLAN.md`: camera/targeting, weapon animation, shot feedback, enemy reactions, an original indoor encounter, and release validation.
- The supplied cinematic firefight image is used only to identify interaction and presentation qualities. No proprietary character, room, animation, audio, dialogue or UI from the reference has been imported or approved.
- Current status: the known-good rifle input/damage path and two-handed template stance are foundations only. The new indoor encounter, six final weapon presentations, full Niagara/audio stack, physical hit reactions, localized captions and complete device/performance evidence are not implemented and must not be reported as complete.
- Stairwell/weapon architecture work started: the generated harbor replaces the solid security-office block with a collidable two-flight stairwell shell, landing, rails, exit marker and bounded lights. Runtime presentation now accepts a separate mesh and transform for every weapon slot. Because only one approved rifle and one pistol mesh are currently available, the generated array intentionally uses tracked stand-ins and does not satisfy the six-final-weapon art gate.
- Verification for this increment: canonical Mac Editor build passed, Editor generation completed with 0 errors, content validation completed with 0 errors, and automation report `Saved/Automation/20260714T021955Z` contains 8 passed / 0 failed / 0 warnings. A live Metal teleport preview reached the interior, but the center position forced very tight camera collision; entrance-to-exit traversal, normal-route framing and AI navigation remain uncertified.
