# Generated Asset Staging

This directory is the local destination for the optional Meshy-to-Blender environment pipeline. Generated FBX, texture, Blender and preview files are ignored by Git until their Meshy generation records and redistribution rights have been reviewed.

Dry-run the three requests without spending credits:

```bash
python3 Scripts/generate_assets.py --dry-run
```

Submit real jobs only from a shell that has a valid Meshy key:

```bash
export MESHY_API_KEY='set-this-locally-never-in-Git'
python3 Scripts/generate_assets.py --execute
```

After all three FBX/PBR sets exist, stage and render them with Blender:

```bash
/Applications/Blender.app/Contents/MacOS/Blender --background \
  --python Scripts/Blender/stage_generated_assets.py -- \
  --input Assets/Imported --output Assets/Imported
```

The output remains source material, not an Unreal asset. Import into `Content/` only through Unreal Editor after visual, collision, performance and license review. Never hand-create `.uasset` files.
