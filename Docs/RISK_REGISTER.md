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
| First mouse click can be consumed by packaged-window capture | High | Aim now toggles on press and shows explicit FOV/HUD state; require fresh keyboard/mouse verification |
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
| Physical hit reactions can destabilize AI navigation and collision | High | Use bounded additive reactions and physical-animation profiles; reserve ragdoll for validated death states |
| Indoor effects exceed Mac frame/audio budgets | High | Pool effects, cap decals/lights/voices, provide independent scalability fallbacks and profile the authored encounter |
| Six weapon slots accidentally presented as six finished models | Critical | Runtime now supports six independent mesh entries; keep rifle/pistol stand-ins recorded until four additional approved models and all six animation sets pass the final-art gate |
