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
- Packaging correction: explicit Ember cook inclusion places all twelve weapon Primary Data Assets in the arm64 app. The current corrected build is `Releases/Development/20260714T005152Z`, SHA-256 `b569c950dcd0e444bf63a8d2b0b0730a618a60e687d20d6f674f6380ce8f3d04`.
- Combat feedback correction: aim is a capture-safe toggle with a 68-degree FOV transition; the functional weapon silhouette, muzzle flash, tracer, impact light, hostile reticle and hit marker are implemented. Three visible marked hostiles form the insertion patrol.
- Packaged keyboard/mouse evidence: a fresh extraction loaded the harbor, then synthetic macOS HID events produced runtime records `Player aim toggled: AIM`, `Player weapon fired; magazine=29 reserve=180`, and `Player reload requested: STARTED`. This proves the packaged bindings/state transitions without mislabeling it as a human playtest.
- Remaining M0 certification evidence: physical Xbox-layout and PlayStation-layout controller records.

## Remaining milestones

### M1–M2 — Player and Combat

- In progress. Six selectable weapon slots now preserve independent magazine/reserve state, preventing switch-to-refill ammunition duplication.
- Weapon definitions drive reload duration, recoil and aim/hip spread. Six functional silhouettes are visually differentiated and the packaged input path is proven.
- Remaining: licensed final meshes/rigs, combat montages, chambering edge cases, full surface routing and physical controller certification.

### M3–M4 — AI and Harbor Mission

- In progress. The insertion patrol now consists of three nearby, marked damageable tactical enemies; the first-patrol objective requires their elimination rather than player position alone.
- Versioned checkpoint capture/restore now includes player transform, health, armor, per-slot ammunition and objective state, with on-disk save/load at the midpoint.
- Remaining: full encounter-by-encounter packaged playthrough, cover/EQS validation, checkpoint migration/corruption recovery, extraction/results acceptance and navigation/HLOD/Data Layer validation.

M5–M9 remain incomplete. Prototype systems or configuration may exist, but none are certified until their acceptance tests pass in a packaged build.
