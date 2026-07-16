#!/usr/bin/env python3
"""Stage Meshy FBX/PBR outputs in Blender and render a review preview.

Run from Blender, for example:
  blender --background --python Scripts/Blender/stage_generated_assets.py -- \
    --input Assets/Imported --output Assets/Imported
"""

from __future__ import annotations

import argparse
import math
import random
import sys
from pathlib import Path
from typing import Iterable, List, Sequence

import bpy
from mathutils import Vector


COLLECTION_NAME = "ProjectEmber_GeneratedPreview"


def parse_args() -> argparse.Namespace:
    arguments = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    return parser.parse_args(arguments)


def remove_existing_collection() -> None:
    existing = bpy.data.collections.get(COLLECTION_NAME)
    if existing is None:
        return
    for obj in list(existing.all_objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    bpy.data.collections.remove(existing)


def import_fbx(path: Path) -> List[bpy.types.Object]:
    before = set(bpy.data.objects)
    try:
        bpy.ops.wm.fbx_import.get_rna_type()
        has_wm_importer = True
    except (AttributeError, RuntimeError):
        has_wm_importer = False
    if has_wm_importer:
        bpy.ops.wm.fbx_import(filepath=str(path))
    else:
        try:
            bpy.ops.import_scene.fbx.get_rna_type()
            has_legacy_importer = True
        except (AttributeError, RuntimeError):
            has_legacy_importer = False
        if not has_legacy_importer:
            raise RuntimeError("This Blender build does not expose an FBX importer")
        bpy.ops.import_scene.fbx(filepath=str(path))
    imported = [obj for obj in bpy.data.objects if obj not in before]
    if not imported:
        raise RuntimeError(f"FBX import created no objects: {path}")
    return imported


def image_node(nodes: bpy.types.Nodes, path: Path, label: str, colorspace: str) -> bpy.types.Node:
    node = nodes.new("ShaderNodeTexImage")
    node.label = label
    node.image = bpy.data.images.load(str(path), check_existing=True)
    node.image.colorspace_settings.name = colorspace
    return node


def create_material(asset_name: str, input_dir: Path) -> bpy.types.Material:
    texture_paths = {
        "albedo": input_dir / f"{asset_name}_albedo.png",
        "normal": input_dir / f"{asset_name}_normal.png",
        "roughness": input_dir / f"{asset_name}_roughness.png",
        "metallic": input_dir / f"{asset_name}_metallic.png",
    }
    for required in ("albedo", "normal", "roughness"):
        if not texture_paths[required].exists():
            raise FileNotFoundError(f"Missing {required} map for {asset_name}: {texture_paths[required]}")

    material = bpy.data.materials.new(f"M_{asset_name}")
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    shader = nodes.new("ShaderNodeBsdfPrincipled")
    shader.inputs["Specular IOR Level"].default_value = 0.45
    links.new(shader.outputs["BSDF"], output.inputs["Surface"])

    albedo = image_node(nodes, texture_paths["albedo"], "Albedo (sRGB)", "sRGB")
    links.new(albedo.outputs["Color"], shader.inputs["Base Color"])

    normal_texture = image_node(nodes, texture_paths["normal"], "Normal (Non-Color)", "Non-Color")
    normal_map = nodes.new("ShaderNodeNormalMap")
    normal_map.inputs["Strength"].default_value = 1.0
    links.new(normal_texture.outputs["Color"], normal_map.inputs["Color"])
    links.new(normal_map.outputs["Normal"], shader.inputs["Normal"])

    roughness = image_node(nodes, texture_paths["roughness"], "Roughness (Non-Color)", "Non-Color")
    links.new(roughness.outputs["Color"], shader.inputs["Roughness"])

    if texture_paths["metallic"].exists():
        metallic = image_node(nodes, texture_paths["metallic"], "Metallic (Non-Color)", "Non-Color")
        links.new(metallic.outputs["Color"], shader.inputs["Metallic"])
    return material


def move_to_collection(objects: Iterable[bpy.types.Object], collection: bpy.types.Collection) -> None:
    for obj in objects:
        for owner in list(obj.users_collection):
            owner.objects.unlink(obj)
        collection.objects.link(obj)


def make_group(
    name: str,
    objects: Sequence[bpy.types.Object],
    collection: bpy.types.Collection,
    location: Sequence[float],
    rotation_z: float = 0.0,
    scale: Sequence[float] = (1.0, 1.0, 1.0),
) -> bpy.types.Object:
    root = bpy.data.objects.new(name, None)
    collection.objects.link(root)
    root.location = location
    root.rotation_euler.z = rotation_z
    root.scale = scale
    for obj in objects:
        obj.parent = root
    return root


def duplicate_group(
    source_objects: Sequence[bpy.types.Object],
    name: str,
    collection: bpy.types.Collection,
    location: Sequence[float],
    rotation_z: float,
    scale: Sequence[float],
) -> bpy.types.Object:
    copies = []
    for source in source_objects:
        duplicate = source.copy()
        if source.data is not None:
            duplicate.data = source.data.copy()
        collection.objects.link(duplicate)
        copies.append(duplicate)
    return make_group(name, copies, collection, location, rotation_z, scale)


def apply_material(objects: Iterable[bpy.types.Object], material: bpy.types.Material) -> None:
    for obj in objects:
        if obj.type != "MESH":
            continue
        obj.data.materials.clear()
        obj.data.materials.append(material)


def add_ground(collection: bpy.types.Collection) -> None:
    bpy.ops.mesh.primitive_plane_add(size=42.0, location=(0.0, 1.5, -0.03))
    ground = bpy.context.object
    ground.name = "PreviewGround"
    move_to_collection([ground], collection)
    material = bpy.data.materials.new("M_PreviewGround")
    material.diffuse_color = (0.035, 0.045, 0.055, 1.0)
    material.roughness = 0.78
    ground.data.materials.append(material)


def point_camera(camera: bpy.types.Object, target: Sequence[float]) -> None:
    direction = Vector(target) - camera.location
    camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def add_lighting_and_camera(collection: bpy.types.Collection) -> None:
    world = bpy.context.scene.world or bpy.data.worlds.new("ProjectEmberPreviewWorld")
    bpy.context.scene.world = world
    world.use_nodes = True
    background = world.node_tree.nodes.get("Background")
    background.inputs["Color"].default_value = (0.012, 0.02, 0.032, 1.0)
    background.inputs["Strength"].default_value = 0.18

    sun_data = bpy.data.lights.new("PreviewSun", "SUN")
    sun_data.energy = 2.2
    sun_data.angle = math.radians(3.0)
    sun = bpy.data.objects.new("PreviewSun", sun_data)
    collection.objects.link(sun)
    sun.rotation_euler = (math.radians(34), math.radians(-18), math.radians(-35))

    for name, location, color, energy, size in (
        ("ColdKey", (7, -7, 10), (0.22, 0.55, 1.0), 1400, 6.0),
        ("WarmFill", (-8, 2, 5), (1.0, 0.24, 0.06), 900, 5.0),
    ):
        data = bpy.data.lights.new(name, "AREA")
        data.energy = energy
        data.color = color
        data.shape = "DISK"
        data.size = size
        light = bpy.data.objects.new(name, data)
        collection.objects.link(light)
        light.location = location
        point_camera(light, (0, 1, 1.5))

    camera_data = bpy.data.cameras.new("PreviewCamera")
    camera_data.lens = 42
    camera = bpy.data.objects.new("PreviewCamera", camera_data)
    collection.objects.link(camera)
    camera.location = (18.5, -23.0, 11.5)
    point_camera(camera, (0.0, 1.0, 2.2))
    bpy.context.scene.camera = camera


def configure_render(output_dir: Path) -> Path:
    scene = bpy.context.scene
    try:
        scene.render.engine = "BLENDER_EEVEE_NEXT"
    except TypeError:
        scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 1280
    scene.render.resolution_y = 720
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    scene.render.image_settings.color_mode = "RGBA"
    try:
        scene.view_settings.look = "AgX - Medium High Contrast"
    except TypeError:
        scene.view_settings.look = "Medium High Contrast"
    output_path = output_dir / "project_ember_asset_preview.png"
    scene.render.filepath = str(output_path)
    return output_path


def main() -> None:
    args = parse_args()
    input_dir = args.input.expanduser().resolve()
    output_dir = args.output.expanduser().resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    required_assets = ("ruined_concrete_pillar", "industrial_crane_ruin", "rubble_pile_debris")
    missing = [str(input_dir / f"{name}.fbx") for name in required_assets if not (input_dir / f"{name}.fbx").exists()]
    if missing:
        raise FileNotFoundError("Missing generated FBX files:\n" + "\n".join(missing))

    remove_existing_collection()
    collection = bpy.data.collections.new(COLLECTION_NAME)
    bpy.context.scene.collection.children.link(collection)

    imported = {}
    for asset_name in required_assets:
        objects = import_fbx(input_dir / f"{asset_name}.fbx")
        move_to_collection(objects, collection)
        apply_material(objects, create_material(asset_name, input_dir))
        imported[asset_name] = objects

    pillar_objects = imported["ruined_concrete_pillar"]
    make_group("Pillar_01", pillar_objects, collection, (-8.0, -4.0, 0.0), math.radians(4))
    duplicate_group(pillar_objects, "Pillar_02", collection, (-8.0, 0.0, 0.0), math.radians(-6), (0.92, 0.92, 1.08))
    duplicate_group(pillar_objects, "Pillar_03", collection, (-8.0, 4.0, 0.0), math.radians(11), (1.05, 1.05, 0.88))

    make_group(
        "Crane_Right",
        imported["industrial_crane_ruin"],
        collection,
        (9.0, 4.0, 0.0),
        math.radians(-18),
        (1.15, 1.15, 1.7),
    )

    rubble_objects = imported["rubble_pile_debris"]
    rubble_positions = ((-1.6, -6.0, 0), (1.2, -2.5, 0), (-0.8, 1.0, 0), (1.5, 4.5, 0), (-1.1, 8.0, 0))
    randomizer = random.Random(31057)
    first_scale = randomizer.uniform(0.75, 1.25)
    make_group(
        "Rubble_01",
        rubble_objects,
        collection,
        rubble_positions[0],
        randomizer.uniform(-math.pi, math.pi),
        (first_scale, first_scale, first_scale),
    )
    for index, position in enumerate(rubble_positions[1:], start=2):
        scale = randomizer.uniform(0.75, 1.25)
        duplicate_group(
            rubble_objects,
            f"Rubble_{index:02d}",
            collection,
            position,
            randomizer.uniform(-math.pi, math.pi),
            (scale, scale, scale),
        )

    add_ground(collection)
    add_lighting_and_camera(collection)
    output_path = configure_render(output_dir)
    blend_path = output_dir / "project_ember_asset_preview.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    bpy.ops.render.render(write_still=True)
    print(f"Rendered preview: {output_path}")
    print(f"Saved Blender scene: {blend_path}")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
