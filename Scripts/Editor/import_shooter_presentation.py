"""Install Epic's UE 5.8 Shooter Variant presentation assets through the Editor.

The source files ship with this Unreal installation. Copying is performed from
inside an Editor Python session, followed by an Asset Registry scan and load,
so no Unreal binary asset is fabricated by repository scripts.
"""

import os
import shutil
import unreal


PROJECT_CONTENT = unreal.Paths.project_content_dir()
ENGINE_ROOT = os.path.abspath(os.path.join(unreal.Paths.engine_dir(), ".."))
TEMPLATE_ROOT = os.path.join(ENGINE_ROOT, "Templates", "TemplateResources", "Standard")


def install_tree(source_relative, destination_relative):
    source = os.path.join(TEMPLATE_ROOT, source_relative)
    destination = os.path.join(PROJECT_CONTENT, destination_relative)
    if not os.path.isdir(source):
        raise RuntimeError(f"Missing Unreal template presentation directory: {source}")
    os.makedirs(destination, exist_ok=True)
    copied = 0
    for root, _, files in os.walk(source):
        relative = os.path.relpath(root, source)
        output = destination if relative == "." else os.path.join(destination, relative)
        os.makedirs(output, exist_ok=True)
        for filename in files:
            if not filename.endswith((".uasset", ".umap")):
                continue
            shutil.copy2(os.path.join(root, filename), os.path.join(output, filename))
            copied += 1
    unreal.log(f"[PROJECT EMBER] Installed {copied} assets into {destination_relative}")


def install_files(source_relative, destination_relative, filenames):
    source = os.path.join(TEMPLATE_ROOT, source_relative)
    destination = os.path.join(PROJECT_CONTENT, destination_relative)
    os.makedirs(destination, exist_ok=True)
    for filename in filenames:
        source_file = os.path.join(source, filename)
        if not os.path.isfile(source_file):
            raise RuntimeError(f"Missing Unreal template presentation asset: {source_file}")
        shutil.copy2(source_file, os.path.join(destination, filename))
    unreal.log(f"[PROJECT EMBER] Installed {len(filenames)} assets into {destination_relative}")


# Preserve the package paths authored by Epic's template assets.
for stale_directory in ("Weapons", "Variant_Shooter"):
    destination = os.path.join(PROJECT_CONTENT, stale_directory)
    if os.path.isdir(destination):
        shutil.rmtree(destination)

install_tree(os.path.join("Weapons", "Content", "Rifle"), os.path.join("Weapons", "Rifle"))
install_tree(os.path.join("Weapons", "Content", "Pistol"), os.path.join("Weapons", "Pistol"))
install_files(
    os.path.join("Variant_Shooter", "Content", "Anims"),
    os.path.join("Variant_Shooter", "Anims"),
    ("ABP_TP_Rifle.uasset", "ABP_TP_Pistol.uasset"),
)

registry = unreal.AssetRegistryHelpers.get_asset_registry()
registry.scan_paths_synchronous(["/Game/Weapons", "/Game/Variant_Shooter"], True, False)

required = [
    "/Game/Weapons/Rifle/Meshes/SM_Rifle.SM_Rifle",
    "/Game/Weapons/Pistol/Meshes/SM_Pistol.SM_Pistol",
    "/Game/Variant_Shooter/Anims/ABP_TP_Rifle.ABP_TP_Rifle_C",
    "/Game/Variant_Shooter/Anims/ABP_TP_Pistol.ABP_TP_Pistol_C",
]
for object_path in required:
    loaded = unreal.load_object(None, object_path)
    if not loaded:
        raise RuntimeError(f"Shooter presentation asset did not load: {object_path}")
    unreal.log(f"[PROJECT EMBER] Verified shooter presentation asset: {object_path}")

unreal.EditorAssetLibrary.save_directory("/Game/Weapons", only_if_is_dirty=True, recursive=True)
unreal.EditorAssetLibrary.save_directory("/Game/Variant_Shooter", only_if_is_dirty=True, recursive=True)
