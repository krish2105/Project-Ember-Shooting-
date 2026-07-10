# Performance Budget

## Frame targets

- `MAC_BASELINE`: 33.3 ms total at 1080p; certification pending base-M2 hardware.
- `MAC_HIGH`: 16.7 ms at 1080p/1440p where measured; M4 Pro is the available proxy.
- `MAC_ULTRA`: 16.7 ms at 1440p where practical; certification pending representative high-tier hardware.

Track game, render, GPU, audio, animation, AI, physics, Niagara, UI and async-loading time independently. Record average, 1% low, p95/p99, hitches over 50/100 ms, peak memory and streaming stalls.

Initial safety limits: 12/20/28 active AI by profile, 128 active projectiles, 256 persistent decals and 48 physical casings. These are provisional until profiling.

Benchmark routes: harbor exploration, warehouse, container combat, maximum AI, waterfront long view, interior/exterior transition, explosive stress and UI/map stress.

