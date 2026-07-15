# Risk Register

| Risk | Severity | Current mitigation |
|---|---|---|
| UE 5.8 absent | Blocker | Owner installs latest 5.8 hotfix before build validation |
| Xcode 26.6 compatibility with UE 5.8 unverified | High | Full Xcode is installed; run UE Turnkey and a native link after UE installation |
| macOS 26/Xcode compatibility with UE 5.8 unknown | High | Run Turnkey VerifySdk and first native link before broad implementation |
| No binary assets or maps | Blocker for playability | Generate only through a compiled Editor module/commandlet or documented Editor steps |
| No approved art/audio library | High | Use tracked functional placeholders; license-review each Fab candidate |
| Previous packaged player used flying/zero-gravity workaround | High | Replaced with normal walking foundation; require fresh packaged movement smoke test before M0 acceptance |
| Duplicate input routes obscured failures and could double-fire | High | Consolidated to one action-binding path; keep automated mapping checks and physical device test records |
| First mouse click can be consumed by packaged-window capture | High | The packaged controller requests game-only focus; aim follows hold-to-aim by default with a separate accessibility toggle; require fresh keyboard/mouse verification |
| Combat actions lacked visible presentation feedback | High | Added deterministic weapon silhouette, muzzle/tracer/impact feedback, hostile labels and hit marker; final licensed presentation remains gated |
| Weapon floated through torso with unarmed pose | High | Replaced capsule attachment with `hand_r`, imported UE 5.8 Shooter Variant rifle locomotion and modeled weapon resources through the Editor, and confirmed the corrected stance in a live Metal preview |
| Weapon switching could refill ammunition | Critical | Each of six slots now stores/restores magazine and reserve state; automation covers restored state |
| Checkpoint previously restored only position in memory | High | Versioned snapshot now persists transform, vitals, weapon ammunition and objectives to a SaveGame slot; packaged migration/corruption tests remain |
| Imported Third Person Blueprint references missing template C++ module | Medium | Generator now assigns the mannequin and available Animation Blueprint directly |
| AAA presentation requested without asset budget or approved content | Critical | Do not mislabel placeholders; obtain approved licensed/commissioned environment, weapon, character, animation and audio content |
| 24 GB below Epic recommendation | Medium | Monitor DDC, shader compile, cook memory and free disk space |
| Base-M2/M3-Pro hardware unavailable | High | Report M4 Pro measurements only; leave tier certification pending |
| Unnotarized public build | High | Document Gatekeeper flow; never claim notarization |
| Tactical AI plus World Partition navigation | High | Use bounded navigation regions for all critical encounters |
| Controller hardware results not yet recorded | High | Require physical Xbox-layout and PlayStation-layout boot-to-results tests |
| Reference image could be mistaken for permission to copy a commercial game | Critical | Reproduce only general interaction qualities; require original or individually licensed characters, room, weapons, animation, audio, dialogue and UI |
| Cinematic effects can hide shot-alignment or state bugs | High | Validate camera/muzzle rays and authoritative damage separately; require packaged interaction evidence, not screenshots alone |
| Physical hit reactions can destabilize AI navigation and collision | High | A 0.12-second bounded skeletal flinch is active; reserve ragdoll for death and keep full physical-animation profiles behind later validation |
| Indoor effects exceed Mac frame/audio budgets | High | Pool effects, cap decals/lights/voices, provide independent scalability fallbacks and profile the authored encounter |
| Six weapon slots accidentally presented as six finished models | Critical | Runtime now supports six independent mesh entries; keep rifle/pistol stand-ins recorded until four additional approved models and all six animation sets pass the final-art gate |
| Generated concept art mistaken for a shipped asset or guaranteed AAA target | Critical | Treat references as composition only; ship original code/Editor-generated content, disclose template stand-ins and verify the packaged result rather than claiming visual parity |
| Night lighting hides traversal, targets or reticle state | High | Use bounded cyan/orange route pools, emissive beacons, live radar and hostile reticle; run a fresh Metal visibility and input smoke test before release |
| Template and generated lighting stacks coexist | High | Generator now removes all owned and untagged template environment lights, atmosphere, cloud, fog and post-process actors before rebuilding a single deterministic lighting rig |
| macOS cursor capture applies an extreme first-frame camera pitch | High | Player controller ignores look input for 250 ms, resets to the authored horizon, then enables normal game-only input; retain a packaged launch-camera smoke check |
| Requested editor MCP connections are unavailable in the active Codex session | Blocker for remote scene editing | Do not infer connectivity from another application's MCP process; configure each server for Codex, reopen the relevant editor, reload the Codex session and prove it with read-only hierarchy queries before mutation |
| Meshy generation could spend credits or leak credentials | High | Generation is dry-run by default, requires explicit `--execute` plus `MESHY_API_KEY`, never prints/stores the key, persists task IDs to prevent duplicate submissions and uses no runtime dependency |
| AI-generated meshes may have unusable topology, scale, UVs or unclear redistribution rights | Critical | Keep generated files out of Git and Unreal Content until per-task license evidence, Blender cleanup, LOD/collision review and packaged Mac profiling pass |
| Placed World Partition enemies retain stale AI possession settings | High | Enemy BeginPlay guarantees its native tactical controller; automation validates AI subobjects and live Metal logs prove all 15 combatants receive controllers |
| Visibility collision presets let accurate enemy rays pass beside the player capsule | High | Enemy fire separates cover blocking from an unobstructed accuracy roll; blocked world traces never damage the target and live logs prove armor/health application |
| Enemy flank fallback drives a hostile into the player camera/rig | Critical | Player/enemy capsules explicitly block Pawn; enemies use RVO, an 11.5 m emergency separation threshold and an 18.5 m target-relative combat ring; automation asserts collision and range ordering |
| Player death returns with empty armor or residual velocity | High | Checkpoint recovery stops pawn movement and restores both health and armor before teleporting to the latest safe location |
| Rifle recoil/reload offsets separate the weapon from animated hands | High | Weapon presentation uses the authored `HandGrip_R` basis at uniform scale; reload no longer moves the mesh independently and recoil is bounded with automatic camera recovery |
| Detailed example vehicle is mistaken for bespoke final art | High | Register the Epic UE-only source; disclose that Chaos driving is functional but the car, door motion and driver animation are not bespoke final assets |
| Experimental Chaos Modular Vehicle behavior changes between UE hotfixes | High | Pin UE 5.8, validate the exact vehicle Blueprint through source automation plus a packaged Metal drive route, and retain an independently removable vehicle module |
| Example vehicle Blueprint and Ember controller both publish driving input | Critical | Disable only the possessed vehicle pawn's Blueprint input component, clear all buffered controls on entry, republish Ember-owned values each vehicle tick, and verify zero idle yaw plus right/left steering phases in the packaged Metal route |
| Floaty jump undermines grounded third-person locomotion | Medium | Use bounded human-scale gravity, launch velocity, air control and falling braking; require authored takeoff/landing animation and packaged controller testing before final certification |
