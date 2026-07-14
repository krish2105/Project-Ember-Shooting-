# Generated Environment Asset Pipeline

## Purpose

This optional build-time pipeline turns three approved environment prompts into reviewable FBX/PBR source assets. It does not create Unreal binary assets and it is not part of the shipped runtime. Every output must pass visual, topology, collision, scale, performance and license review before Unreal Editor import.

## Live connection audit — 2026-07-14

The active Codex tool registry does not expose Blender, Unity, Unreal or Meshy MCP tools. Consequently no editor hierarchy or scene content was remotely read or changed in this session.

- Blender 5.1.2 is installed and a GUI process is currently open, but the active Codex registry has no Blender MCP tool and the Blender process exposes no MCP listening socket. The `blender-mcp` processes detected on the host belong to a separate Claude extension session and are not Codex connections.
- Unity Hub is installed, but no Unity Editor process or active Unity project is present. There is no hierarchy to query.
- The Project Ember Unreal project is present locally, but no Unreal MCP tool is registered in this Codex session.
- `MESHY_API_KEY` is not configured. No Meshy request, paid generation task or download was performed.

These connections are therefore **not working in the current Codex session**. Their state must not be reported as healthy based solely on another application's background process.

## Meshy generation

`Scripts/generate_assets.py` uses only the Python standard library. It implements Meshy's two-stage Text-to-3D v2 flow:

1. Submit a preview task with triangle topology, remeshing, bottom origin and a per-asset polygon budget.
2. Poll the task until it succeeds or reaches a terminal failure state.
3. Submit a refine task with PBR and HD textures enabled and baked lighting removed.
4. Poll the refine task.
5. Download FBX, Albedo, Normal and Roughness maps. Download Metallic and preview images when supplied.
6. Hash every downloaded file and write a resumable local task record plus an asset manifest.

The supported assets are:

| Asset | Triangle target | Art intent |
|---|---:|---|
| `ruined_concrete_pillar` | 12,000 | Fractured neo-classical limestone/concrete with rebar |
| `industrial_crane_ruin` | 30,000 | Corroded broken dock crane with hook and cable |
| `rubble_pile_debris` | 16,000 | Low mixed masonry/concrete/rebar rubble mound |

Dry-run is the default and never performs a network request:

```bash
python3 Scripts/generate_assets.py --dry-run
```

Real generation is deliberately gated by both an explicit flag and an environment variable:

```bash
export MESHY_API_KEY='set-locally-never-commit-it'
python3 Scripts/generate_assets.py --execute
```

Interrupted jobs resume from `Assets/Imported/meshy_tasks.json`. Never delete this state file while tasks are running because doing so can submit duplicate paid jobs.

## Blender staging and preview

After successful generation, run:

```bash
/Applications/Blender.app/Contents/MacOS/Blender --background \
  --python Scripts/Blender/stage_generated_assets.py -- \
  --input Assets/Imported --output Assets/Imported
```

The Blender script:

- imports all three FBX files;
- creates Principled BSDF materials from Albedo, Normal, Roughness and optional Metallic maps;
- places three pillar instances on the left at four-unit spacing;
- scales and places the ruined crane on the right;
- creates five deterministic rubble instances along the central route;
- adds a neutral review ground, cold key, warm fill, sun and camera;
- saves `project_ember_asset_preview.blend`; and
- renders `project_ember_asset_preview.png` at 1280 × 720.

It edits only a collection named `ProjectEmber_GeneratedPreview`, preserving unrelated scene collections. It exits non-zero if an expected source file is missing.

The staging path was exercised end-to-end on Blender 5.1.2 with disposable primitive FBX and three-map PBR fixtures under `/tmp`; import, material creation, requested instance layout, `.blend` save and PNG render succeeded. That smoke test validates the automation path only. It is not a substitute for a successful Meshy generation and its preview is not an approved art result.

## Unreal intake gate

Generated files remain outside `Content/` and are ignored by Git by default. Before intake:

1. Record the generation task IDs, Meshy account/license basis and attribution requirements.
2. Review silhouette and art direction against the approved concept target.
3. Repair topology, pivots, scale, UVs, shading and texture channel conventions in Blender.
4. Produce conventional LODs and simplified collision for Mac fallbacks.
5. Import through Unreal Editor; never fabricate `.uasset` files.
6. Create material instances, collision, cover metadata and validation records.
7. Profile each asset in a packaged Mac build before approving it for the mission.

The three generated props cover only a small part of the full environment manifest. They do not complete the modular architecture kit, hero buildings, vehicles, materials, VFX, navigation, mission art or final character/weapon pipeline.
