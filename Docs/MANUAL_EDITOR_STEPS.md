# Manual Editor Steps

No Editor operation has been performed yet.

After UE 5.8 and Xcode are installed and the Editor target compiles:

1. Launch `ProjectEmber.uproject` with the compiled Editor target.
2. Create `/Game/Ember` folders, developer test assets and the functional test map through verified Editor operations. A bootstrap generator has not yet been implemented and must not be assumed to exist.
3. Confirm every generated asset was created by the Editor and appears under `/Game/Ember`.
4. Create the World Partition harbor map using Open World settings; enable One File Per Actor.
5. Create Data Layers for core route, alternate routes, stealth route, encounters, audio, optimization and debug content.
6. Import only license-approved assets whose register rows are complete.
7. Assign physical materials, collision, navigation, LOD/HLOD and streaming metadata.
8. Run content validation and save all only after errors are resolved.

These steps are pending and must not be reported as completed until observed.
