# Risk Register

| Risk | Severity | Current mitigation |
|---|---|---|
| UE 5.8 absent | Blocker | Owner installs latest 5.8 hotfix before build validation |
| Xcode 26.6 compatibility with UE 5.8 unverified | High | Full Xcode is installed; run UE Turnkey and a native link after UE installation |
| macOS 26/Xcode compatibility with UE 5.8 unknown | High | Run Turnkey VerifySdk and first native link before broad implementation |
| No binary assets or maps | Blocker for playability | Generate only through a compiled Editor module/commandlet or documented Editor steps |
| No approved art/audio library | High | Use tracked functional placeholders; license-review each Fab candidate |
| 24 GB below Epic recommendation | Medium | Monitor DDC, shader compile, cook memory and free disk space |
| Base-M2/M3-Pro hardware unavailable | High | Report M4 Pro measurements only; leave tier certification pending |
| Unnotarized public build | High | Document Gatekeeper flow; never claim notarization |
| Tactical AI plus World Partition navigation | High | Use bounded navigation regions for all critical encounters |
| Controller hardware results not yet recorded | High | Require physical Xbox-layout and PlayStation-layout boot-to-results tests |
