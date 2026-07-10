# Mac Release Checklist

- [ ] UE Turnkey accepts installed macOS SDK/Xcode.
- [ ] Clean Editor and Game targets compile arm64.
- [ ] Automation suite passes with report retained.
- [ ] Development and Shipping packages cook and launch.
- [ ] Insertion-to-extraction smoke route passes.
- [ ] Xbox-layout and PlayStation-layout controller runs pass.
- [ ] Keyboard layout, resolution, safe-zone and save-path tests pass.
- [ ] `file` and `lipo -archs` report arm64 for every native binary.
- [ ] Info.plist is valid.
- [ ] Ad-hoc code signature verifies.
- [ ] Expected Gatekeeper rejection is documented; notarization is not claimed.
- [ ] Fresh-download extraction and documented opening flow pass.
- [ ] Crash logs and symbols are retained.
- [ ] Licenses, notices, placeholder list and asset register are complete.
- [ ] ZIP SHA-256 matches the published checksum.

