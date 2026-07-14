import math
import unreal


ROOT = "/Game/Ember"
BLUEPRINT_PATH = f"{ROOT}/Blueprints"
MAP_PATH = f"{ROOT}/Maps"
CHARACTER_NAME = "BP_EmberCharacter"
GAMEMODE_NAME = "BP_EmberGameMode"
HARBOR_ASSET = f"{MAP_PATH}/L_HarborVerticalSlice"
ACTOR_SUBSYSTEM = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
MATERIAL_PATH = f"{ROOT}/Materials"
MATERIALS = {}


def log(message):
    unreal.log(f"[PROJECT EMBER] {message}")


def ensure_surface_material(name, base_color, roughness, metallic=0.0, emissive=None):
    """Create an original procedural PBR material through the Editor API."""
    object_path = f"{MATERIAL_PATH}/{name}"
    # These assets are generator-owned. Recreate them so art-direction changes
    # are deterministic instead of silently retaining an older flat material.
    if unreal.EditorAssetLibrary.does_asset_exist(object_path):
        if not unreal.EditorAssetLibrary.delete_asset(object_path):
            raise RuntimeError(f"Unable to replace material {object_path}")
    factory = unreal.MaterialFactoryNew()
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, MATERIAL_PATH, unreal.Material, factory
    )
    if not material:
        raise RuntimeError(f"Unable to create material {object_path}")

    dark = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant3Vector, -420, -40
    )
    dark_color = tuple(max(0.001, channel * 0.48) for channel in base_color)
    light_color = tuple(min(1.0, channel * 1.55 + 0.012) for channel in base_color)
    dark.set_editor_property("constant", unreal.LinearColor(*dark_color, 1.0))
    light = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant3Vector, -420, 45
    )
    light.set_editor_property("constant", unreal.LinearColor(*light_color, 1.0))
    world_position = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionWorldPosition, -680, 160
    )
    noise = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionNoise, -420, 160
    )
    noise.set_editor_property("scale", 0.0025)
    noise.set_editor_property("quality", 1)
    noise.set_editor_property("levels", 3)
    noise.set_editor_property("output_min", 0.08)
    noise.set_editor_property("output_max", 0.92)
    unreal.MaterialEditingLibrary.connect_material_expressions(
        world_position, "", noise, "Position"
    )
    variation = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionLinearInterpolate, -120, 20
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(dark, "", variation, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(light, "", variation, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(noise, "", variation, "Alpha")
    unreal.MaterialEditingLibrary.connect_material_property(
        variation, "", unreal.MaterialProperty.MP_BASE_COLOR
    )
    rough = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -420, 80
    )
    rough.set_editor_property("r", roughness)
    unreal.MaterialEditingLibrary.connect_material_property(
        rough, "", unreal.MaterialProperty.MP_ROUGHNESS
    )
    metal = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -420, 180
    )
    metal.set_editor_property("r", metallic)
    unreal.MaterialEditingLibrary.connect_material_property(
        metal, "", unreal.MaterialProperty.MP_METALLIC
    )
    if emissive:
        glow = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionConstant3Vector, -420, 290
        )
        glow.set_editor_property("constant", unreal.LinearColor(*emissive, 1.0))
        unreal.MaterialEditingLibrary.connect_material_property(
            glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR
        )
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def configure_material_library():
    unreal.EditorAssetLibrary.make_directory(MATERIAL_PATH)
    definitions = {
        "asphalt": ("M_WetAsphalt", (0.012, 0.018, 0.024), 0.24, 0.0, None),
        "limestone": ("M_WeatheredLimestone", (0.19, 0.18, 0.16), 0.72, 0.0, None),
        "concrete": ("M_DamagedConcrete", (0.075, 0.085, 0.09), 0.82, 0.0, None),
        "rubble": ("M_Rubble", (0.095, 0.075, 0.06), 0.88, 0.0, None),
        "steel": ("M_PaintedSteel", (0.018, 0.035, 0.045), 0.38, 0.88, None),
        "rust": ("M_RustedSteel", (0.12, 0.035, 0.012), 0.58, 0.78, None),
        "burnt": ("M_BurntMetal", (0.008, 0.008, 0.009), 0.7, 0.82, None),
        "container_red": ("M_ContainerRed", (0.18, 0.012, 0.008), 0.5, 0.72, None),
        "container_blue": ("M_ContainerBlue", (0.008, 0.055, 0.12), 0.46, 0.72, None),
        "container_orange": ("M_ContainerOrange", (0.24, 0.045, 0.004), 0.52, 0.7, None),
        "water": ("M_HarborWater", (0.002, 0.025, 0.045), 0.08, 0.15, None),
        "vegetation": ("M_DarkVegetation", (0.008, 0.045, 0.018), 0.86, 0.0, None),
        "cyan": ("M_EmissiveCyan", (0.005, 0.08, 0.11), 0.2, 0.25, (0.0, 7.0, 12.0)),
        "orange": ("M_EmissiveOrange", (0.16, 0.018, 0.002), 0.28, 0.15, (14.0, 1.6, 0.08)),
        "window": ("M_DarkWindow", (0.002, 0.008, 0.014), 0.06, 0.35, None),
    }
    for key, values in definitions.items():
        MATERIALS[key] = ensure_surface_material(*values)


def choose_material(label):
    if label in ("Harbor_Ground", "Insertion_Foundation", "Warehouse_Floor") or "Road" in label:
        return MATERIALS.get("asphalt")
    if label == "Harbor_Waterfront":
        return MATERIALS.get("water")
    if label.startswith("Container_Red"):
        return MATERIALS.get("container_red")
    if label.startswith("Container_Blue"):
        return MATERIALS.get("container_blue")
    if label.startswith("Container_Orange"):
        return MATERIALS.get("container_orange")
    if label.startswith("Neon_") or label.endswith("ExitMarker") or "CyanFixture" in label:
        return MATERIALS.get("cyan")
    if "FireCore" in label or "OrangeFixture" in label:
        return MATERIALS.get("orange")
    if any(token in label for token in ("Window", "InteriorVoid")):
        return MATERIALS.get("window")
    if any(token in label for token in ("Column", "Palace", "Cornice", "Arcade", "Civic", "SeaWall")):
        return MATERIALS.get("limestone")
    if any(token in label for token in ("Rubble", "Debris", "Brick")):
        return MATERIALS.get("rubble")
    if any(token in label for token in ("Crane", "Pipe", "Rail", "Truss", "Bollard", "Lamp")):
        return MATERIALS.get("rust")
    if any(token in label for token in ("Wreck", "Burnt")):
        return MATERIALS.get("burnt")
    if any(token in label for token in ("Tree", "Canopy", "Vegetation")):
        return MATERIALS.get("vegetation")
    if any(token in label for token in ("Warehouse", "Stairwell", "Cover", "Barrier", "Facade", "Wall", "Floor")):
        return MATERIALS.get("concrete")
    return MATERIALS.get("steel")


def create_blueprint(name, package_path, parent_class):
    object_path = f"{package_path}/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(object_path)
    if existing:
        return existing
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, package_path, unreal.Blueprint, factory
    )
    if not asset:
        raise RuntimeError(f"Unable to create Blueprint {object_path}")
    return asset


def configure_character_blueprint():
    parent = unreal.load_class(None, "/Script/EmberGameplay.EmberCharacter")
    if not parent:
        raise RuntimeError("AEmberCharacter class is unavailable")
    blueprint = create_blueprint(CHARACTER_NAME, BLUEPRINT_PATH, parent)
    generated = blueprint.generated_class()
    cdo = unreal.get_default_object(generated)
    mesh_component = cdo.get_editor_property("mesh")
    mannequin = unreal.load_asset(
        "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"
    )
    anim_class = unreal.load_class(
        None, "/Game/Variant_Shooter/Anims/ABP_TP_Rifle.ABP_TP_Rifle_C"
    )
    if mannequin:
        mesh_component.set_skeletal_mesh_asset(mannequin)
    if anim_class:
        mesh_component.set_editor_property("anim_class", anim_class)
        mesh_component.set_editor_property(
            "animation_mode", unreal.AnimationMode.ANIMATION_BLUEPRINT
        )
    mesh_component.set_editor_property(
        "relative_location", unreal.Vector(0.0, 0.0, -90.0)
    )
    mesh_component.set_editor_property(
        "relative_rotation",
        unreal.Rotator(pitch=0.0, yaw=-90.0, roll=0.0),
    )
    mesh_component.set_collision_profile_name("CharacterMesh")
    mesh_component.set_simulate_physics(False)
    weapon_component = cdo.get_editor_property("weapon_body_visual")
    weapon_component.set_editor_property(
        "relative_rotation", unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0)
    )
    primary_weapon = unreal.load_asset("/Game/Weapons/Rifle/Meshes/SM_Rifle")
    sidearm_weapon = unreal.load_asset("/Game/Weapons/Pistol/Meshes/SM_Pistol")
    if not primary_weapon or not sidearm_weapon:
        raise RuntimeError("Shooter presentation meshes are unavailable")
    cdo.set_editor_property("primary_weapon_mesh", primary_weapon)
    cdo.set_editor_property("sidearm_weapon_mesh", sidearm_weapon)
    # Runtime presentation is now slot-specific. The approved UE template only
    # provides a rifle and pistol, so these remain explicitly tracked stand-ins
    # until six original/approved models are available; gameplay no longer
    # assumes that all primary slots must share a single mesh or transform.
    cdo.set_editor_property(
        "weapon_presentation_meshes",
        [primary_weapon, primary_weapon, primary_weapon, primary_weapon, primary_weapon, sidearm_weapon],
    )
    cdo.set_editor_property(
        "weapon_presentation_transforms",
        [
            unreal.Transform(location=unreal.Vector(0.0, 0.0, 0.0), rotation=unreal.Rotator(0.0, 0.0, 0.0), scale=unreal.Vector(1.0, 1.0, 1.0)),
            unreal.Transform(location=unreal.Vector(0.0, 0.0, 0.0), rotation=unreal.Rotator(0.0, 0.0, 0.0), scale=unreal.Vector(1.0, 1.0, 1.0)),
            unreal.Transform(location=unreal.Vector(0.0, 0.0, 0.0), rotation=unreal.Rotator(0.0, 0.0, 0.0), scale=unreal.Vector(1.0, 1.0, 1.0)),
            unreal.Transform(location=unreal.Vector(0.0, 0.0, 0.0), rotation=unreal.Rotator(0.0, 0.0, 0.0), scale=unreal.Vector(1.0, 1.0, 1.0)),
            unreal.Transform(location=unreal.Vector(0.0, 0.0, 0.0), rotation=unreal.Rotator(0.0, 0.0, 0.0), scale=unreal.Vector(1.0, 1.0, 1.0)),
            unreal.Transform(location=unreal.Vector(0.0, 0.0, 0.0), rotation=unreal.Rotator(0.0, 0.0, 0.0), scale=unreal.Vector(1.0, 1.0, 1.0)),
        ],
    )
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
    return blueprint.generated_class()


def configure_game_mode(character_class):
    parent = unreal.load_class(None, "/Script/EmberGameplay.EmberGameMode")
    if not parent:
        raise RuntimeError("AEmberGameMode class is unavailable")
    blueprint = create_blueprint(GAMEMODE_NAME, BLUEPRINT_PATH, parent)
    cdo = unreal.get_default_object(blueprint.generated_class())
    cdo.set_editor_property("default_pawn_class", character_class)
    player_controller_class = unreal.load_class(
        None, "/Script/EmberGameplay.EmberPlayerController"
    )
    if not player_controller_class:
        raise RuntimeError("AEmberPlayerController class is unavailable")
    cdo.set_editor_property("player_controller_class", player_controller_class)
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
    return blueprint.generated_class()


def generated(actor, label):
    actor.set_actor_label(label)
    actor.set_editor_property("tags", [unreal.Name("EmberGenerated")])
    return actor


def persistent(actor):
    """Keep critical runtime infrastructure loaded in a World Partition game."""
    try:
        actor.set_is_spatially_loaded(False)
    except Exception:
        try:
            actor.set_editor_property("is_spatially_loaded", False)
        except Exception:
            pass
    return actor


def spawn_mesh(mesh, label, location, scale, rotation=None, collision=True, material=None):
    rotation = rotation or unreal.Rotator(0.0, 0.0, 0.0)
    actor = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.StaticMeshActor, location, rotation, transient=False
    )
    if not actor:
        raise RuntimeError(f"Unable to spawn {label}")
    component = actor.get_editor_property("static_mesh_component")
    component.set_static_mesh(mesh)
    # Prototype assets are not guaranteed to retain a blocking collision
    # profile after unattended duplication/cooking. Set it explicitly so the
    # packaged pawn cannot fall through the harbor floor.
    component.set_collision_profile_name("BlockAll" if collision else "NoCollision")
    component.set_collision_enabled(
        unreal.CollisionEnabled.QUERY_AND_PHYSICS if collision else unreal.CollisionEnabled.NO_COLLISION
    )
    material = material or choose_material(label)
    if material:
        component.set_material(0, material)
    generated(actor, label)
    # The vertical slice is intentionally small enough to keep its authored
    # blockout resident. Without this, a packaged World Partition game can
    # spawn the pawn before the insertion cell (and its collision) streams in,
    # causing the player to fall forever through an otherwise valid harbor.
    persistent(actor)
    actor.set_actor_scale3d(scale)
    return actor


def spawn_skeletal_set_dressing(mesh, label, location, scale, rotation=None, collision=True):
    """Place an approved, fully authored skeletal asset as inert set dressing."""
    rotation = rotation or unreal.Rotator(0.0, 0.0, 0.0)
    actor = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.SkeletalMeshActor, location, rotation, transient=False
    )
    if not actor:
        raise RuntimeError(f"Unable to spawn {label}")
    component = actor.get_editor_property("skeletal_mesh_component")
    component.set_skeletal_mesh_asset(mesh)
    component.set_simulate_physics(False)
    component.set_collision_profile_name("BlockAll" if collision else "NoCollision")
    component.set_collision_enabled(
        unreal.CollisionEnabled.QUERY_AND_PHYSICS if collision else unreal.CollisionEnabled.NO_COLLISION
    )
    generated(actor, label)
    persistent(actor)
    actor.set_actor_scale3d(scale)
    return actor


def spawn_driveable_vehicle(vehicle_class, label, location, rotation=None):
    """Place Epic's complete modular sports-car pawn as an Ember vehicle."""
    rotation = rotation or unreal.Rotator(0.0, 0.0, 0.0)
    actor = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        vehicle_class, location, rotation, transient=False
    )
    if not actor:
        raise RuntimeError(f"Unable to spawn driveable vehicle {label}")
    generated(actor, label)
    actor.set_editor_property("auto_possess_ai", unreal.AutoPossessAI.DISABLED)
    actor.set_editor_property("auto_possess_player", unreal.AutoReceiveInput.DISABLED)
    actor.set_editor_property(
        "tags", [unreal.Name("EmberGenerated"), unreal.Name("EmberDriveableVehicle")]
    )
    persistent(actor)
    actor.set_actor_scale3d(unreal.Vector(1.0, 1.0, 1.0))
    return actor


def remove_previous_generated_actors():
    # The source Open World template contains its own sun, sky, fog and post
    # process actors. Keeping those actors while also creating the authored
    # Ember rig doubles the exposure and was the reason the packaged map was
    # flat orange/white. Environment actors are fully owned by this generator,
    # so remove both tagged instances and untagged template defaults.
    environment_classes = (
        unreal.DirectionalLight,
        unreal.SkyLight,
        unreal.SkyAtmosphere,
        unreal.ExponentialHeightFog,
        unreal.VolumetricCloud,
        unreal.PostProcessVolume,
        unreal.PointLight,
    )
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        is_generated = unreal.Name("EmberGenerated") in actor.get_editor_property("tags")
        is_environment = isinstance(actor, environment_classes)
        if is_generated or is_environment:
            unreal.EditorLevelLibrary.destroy_actor(actor)


def spawn_practical_light(label, location, color, intensity=18000.0, radius=6500.0):
    light = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.PointLight, location, unreal.Rotator(0, 0, 0), transient=False
    )
    if not light:
        return None
    generated(light, label)
    persistent(light)
    component = light.get_editor_property("point_light_component")
    component.set_editor_property("intensity", intensity)
    component.set_editor_property("attenuation_radius", radius)
    component.set_editor_property("light_color", color)
    try:
        component.set_editor_property("cast_shadows", False)
    except Exception:
        pass
    return light


def assemble_foreground_city(cube, cylinder, sphere, chamfer):
    """Build a dense, original ruined waterfront foreground around insertion."""
    # Dark paved combat road and raised sidewalks replace the visible white grid.
    spawn_mesh(cube, "MainCombatRoad", unreal.Vector(-4000, 0, 5), unreal.Vector(920, 82, 0.18))
    for side in (-1, 1):
        spawn_mesh(
            cube, f"Arcade_Sidewalk_{side}", unreal.Vector(-9000, side * 9200, 38),
            unreal.Vector(820, 95, 0.75),
        )

    # Left neo-classical ruin: three storeys, repeated bays, columns and a
    # collapsed corner. The route remains open down the middle for combat.
    palace_y = 10500
    for floor in range(4):
        spawn_mesh(
            cube, f"Palace_Floor_{floor:02d}",
            unreal.Vector(-27500, palace_y, 80 + floor * 850),
            unreal.Vector(185, 62, 1.4), collision=floor == 0,
        )
    for bay in range(8):
        x = -43000 + bay * 4500
        damage = bay in (1, 6)
        for storey in range(3):
            z = 480 + storey * 850
            if damage and storey >= 1:
                continue
            spawn_mesh(
                cube, f"Palace_Facade_{bay:02d}_{storey:02d}",
                unreal.Vector(x, palace_y - 2700, z),
                unreal.Vector(17, 3, 7.2), collision=True,
            )
            spawn_mesh(
                cube, f"Palace_Window_{bay:02d}_{storey:02d}",
                unreal.Vector(x, palace_y - 3005, z + 55),
                unreal.Vector(6.5, 0.35, 4.5), collision=False,
            )
        if bay % 2 == 0 and not damage:
            spawn_mesh(
                cylinder, f"Palace_Column_{bay:02d}",
                unreal.Vector(x - 1150, palace_y - 3550, 930),
                unreal.Vector(2.8, 2.8, 18.0), collision=True,
            )
            spawn_mesh(
                cylinder, f"Palace_ColumnCap_{bay:02d}",
                unreal.Vector(x - 1150, palace_y - 3550, 1820),
                unreal.Vector(4.2, 4.2, 1.2), collision=False,
            )
    for level in (840, 1690, 2540):
        spawn_mesh(
            cube, f"Palace_Cornice_{level}", unreal.Vector(-27500, palace_y - 3080, level),
            unreal.Vector(190, 4.5, 0.65), collision=False,
        )

    # Right industrial frontage provides close camera parallax and cover.
    industrial_y = -10500
    for bay in range(9):
        x = -43000 + bay * 5200
        height = 1450 + (bay % 3) * 360
        spawn_mesh(
            cube, f"Warehouse_Facade_{bay:02d}", unreal.Vector(x, industrial_y, height * 0.5),
            unreal.Vector(22, 22, height / 100.0), collision=True,
        )
        spawn_mesh(
            cube, f"Warehouse_InteriorVoid_{bay:02d}",
            unreal.Vector(x, industrial_y + 2210, 520),
            unreal.Vector(11, 0.4, 7.5), collision=False,
        )
        for rail in range(3):
            spawn_mesh(
                cube, f"Warehouse_Truss_{bay:02d}_{rail:02d}",
                unreal.Vector(x, industrial_y + 2350, 1100 + rail * 280),
                unreal.Vector(20, 0.45, 0.45), collision=False,
                material=MATERIALS.get("rust"),
            )

    # Epic's UE-only Chaos Modular Vehicle example provides the complete PBR
    # car, physics asset, suspension/wheels, modular simulation and vehicle
    # animation Blueprint. Ember adds its own possession, driver-seat, exit,
    # HUD and audio bridge in C++ while retaining Epic's real wheel simulation.
    sports_car_class = unreal.load_class(
        None,
        "/ChaosModularVehicleExamples/Vehicles/ModularVehicle/SportsCar/"
        "BP_ModularVehicleSimplifiedSkeletalSetup."
        "BP_ModularVehicleSimplifiedSkeletalSetup_C",
    )
    if not sports_car_class:
        raise RuntimeError("Chaos Modular Vehicle sports-car pawn is unavailable")

    # The first car is within interaction range of insertion; the remaining
    # cars support optional traversal later in the harbor. They are intact
    # driveable vehicles, so the earlier misleading "wrecked" labels are gone.
    vehicles = [(-44500, 0, 0), (-18500, 3200, 12), (2500, -2400, -15), (14500, 3400, 5)]
    for index, (x, y, yaw) in enumerate(vehicles):
        spawn_driveable_vehicle(
            sports_car_class, f"Ember_DriveableCar_{index:02d}",
            unreal.Vector(x, y, 120), unreal.Rotator(0, yaw, 0),
        )
        # Small roadside debris visually integrates the clean licensed car
        # while leaving adequate clearance for suspension and safe entry.
        for debris in range(3):
            spawn_mesh(
                chamfer, f"DriveableCar_Debris_{index:02d}_{debris:02d}",
                unreal.Vector(x - 280 + debris * 260, y + 310, 42 + debris * 9),
                unreal.Vector(1.8 + debris * 0.4, 1.2, 0.55),
                unreal.Rotator(debris * 9, yaw + debris * 31, debris * 5),
                collision=False, material=MATERIALS.get("burnt"),
            )

    rubble_centers = [
        (-40500, 6900), (-35000, 8200), (-29200, -7200), (-23000, 7600),
        (-14500, -6900), (-6500, 7000), (4500, -7100), (12500, 7200),
    ]
    for pile, (x, y) in enumerate(rubble_centers):
        for chunk in range(7):
            angle = math.radians((pile * 47 + chunk * 71) % 360)
            radius = 180 + (chunk % 3) * 210
            spawn_mesh(
                chamfer, f"Rubble_{pile:02d}_{chunk:02d}",
                unreal.Vector(x + math.cos(angle) * radius, y + math.sin(angle) * radius, 65 + (chunk % 2) * 45),
                unreal.Vector(2.2 + (chunk % 3), 1.8 + ((chunk + 1) % 3), 0.8 + (chunk % 2)),
                unreal.Rotator((chunk * 13) % 24, (chunk * 57) % 180, (chunk * 9) % 20),
                collision=chunk < 3,
            )
        spawn_mesh(
            cylinder, f"Rubble_Rebar_{pile:02d}", unreal.Vector(x + 100, y - 80, 180),
            unreal.Vector(0.22, 0.22, 7.0), unreal.Rotator(62, pile * 31, 0),
            collision=False, material=MATERIALS.get("rust"),
        )

    # The insertion must read as a combat space on the first frame rather than
    # an empty runway. A broken checkpoint, staggered cover and wet patches
    # create close parallax while preserving a 6 m-wide traversal channel.
    for side in (-1, 1):
        gate_y = side * 5150
        spawn_mesh(
            cube, f"InsertionGate_Pier_{side}", unreal.Vector(-43200, gate_y, 1150),
            unreal.Vector(15, 15, 23), collision=True,
        )
        spawn_mesh(
            cylinder, f"InsertionGate_Column_{side}", unreal.Vector(-42300, gate_y - side * 1550, 1050),
            unreal.Vector(3.8, 3.8, 21), collision=True,
        )
        spawn_mesh(
            cube, f"InsertionGate_BrokenBeam_{side}", unreal.Vector(-42100, gate_y - side * 900, 2260),
            unreal.Vector(24, 3, 2.2), unreal.Rotator(0, side * 8, side * 5),
            collision=False,
        )

    cover_layout = [
        (-40700, -2700, 7), (-39200, 3100, -11), (-36700, -1800, 4),
        (-34400, 2600, -8), (-31800, -3200, 12), (-28800, 2200, -5),
        (-25500, -2500, 6), (-22000, 3100, -10),
    ]
    for index, (x, y, yaw) in enumerate(cover_layout):
        spawn_mesh(
            chamfer, f"RoadCover_Barrier_{index:02d}", unreal.Vector(x, y, 105),
            unreal.Vector(19, 4.2, 2.1), unreal.Rotator(0, yaw, 0), collision=True,
            material=MATERIALS.get("concrete"),
        )
        if index % 2 == 0:
            for barrel in range(3):
                spawn_mesh(
                    cylinder, f"RoadCover_Barrel_{index:02d}_{barrel}",
                    unreal.Vector(x + 360 + barrel * 125, y + 520, 92),
                    unreal.Vector(1.15, 1.15, 1.85), collision=True,
                    material=MATERIALS.get("rust"),
                )

    for index, (x, y, sx, sy) in enumerate([
        (-41800, 600, 28, 16), (-38200, -4100, 42, 13),
        (-32800, 4200, 34, 18), (-27100, -3900, 48, 14),
        (-20500, 1200, 55, 20),
    ]):
        spawn_mesh(
            cube, f"Road_Puddle_{index:02d}", unreal.Vector(x, y, 24),
            unreal.Vector(sx, sy, 0.035), collision=False,
            material=MATERIALS.get("water"),
        )

    # Close wreck and pipe bundles break the silhouette of the entry lane.
    spawn_mesh(
        chamfer, "Insertion_WreckedTruck", unreal.Vector(-41400, 3550, 190),
        unreal.Vector(32, 13, 3.4), unreal.Rotator(0, -13, 0), collision=True,
        material=MATERIALS.get("burnt"),
    )
    spawn_mesh(
        cube, "Insertion_WreckedTruckCab", unreal.Vector(-39900, 3550, 520),
        unreal.Vector(10, 11, 6.2), unreal.Rotator(0, -13, 0), collision=True,
        material=MATERIALS.get("rust"),
    )
    for pipe in range(5):
        spawn_mesh(
            cylinder, f"Insertion_PipeBundle_{pipe:02d}",
            unreal.Vector(-37500 + pipe * 55, -4550, 150 + pipe * 25),
            unreal.Vector(1.25, 1.25, 24), unreal.Rotator(90, 0, 0),
            collision=pipe < 2, material=MATERIALS.get("rust"),
        )

    # Street lights, fire pockets and flooded cyan fixtures establish the
    # cold/warm lighting language in the approved concept target.
    for index, x in enumerate(range(-41000, 24000, 8000)):
        side = -1 if index % 2 else 1
        y = side * 7200
        spawn_mesh(
            cylinder, f"StreetLamp_Post_{index:02d}", unreal.Vector(x, y, 650),
            unreal.Vector(0.45, 0.45, 13.0), collision=False,
        )
        spawn_mesh(
            cube, f"CyanFixture_{index:02d}", unreal.Vector(x, y - side * 120, 1350),
            unreal.Vector(2.4, 0.8, 0.35), collision=False,
        )
        spawn_practical_light(
            f"StreetLamp_Light_{index:02d}", unreal.Vector(x, y, 1270),
            unreal.Color(90, 180, 255, 255), 1100.0, 3600.0,
        )

    for index, (x, y) in enumerate(((-36500, 10100), (-12000, -7600), (9000, 7900), (22500, -6400))):
        spawn_mesh(
            sphere, f"FireCore_{index:02d}", unreal.Vector(x, y, 165),
            unreal.Vector(1.2, 1.2, 2.1), collision=False,
        )
        spawn_practical_light(
            f"FireLight_{index:02d}", unreal.Vector(x, y, 320),
            unreal.Color(255, 92, 38, 255), 900.0, 2400.0,
        )

    # A recognizable ruined crane and distant civic dome anchor the skyline.
    crane_x, crane_y = 26500, -25500
    spawn_mesh(cube, "Crane_Base", unreal.Vector(crane_x, crane_y, 260), unreal.Vector(28, 28, 5.2))
    for side in (-1, 1):
        spawn_mesh(
            cube, f"Crane_Mast_{side}", unreal.Vector(crane_x + side * 750, crane_y, 3650),
            unreal.Vector(3.2, 3.2, 68), unreal.Rotator(0, side * 7, 0),
        )
    spawn_mesh(cube, "Crane_Jib", unreal.Vector(crane_x - 3800, crane_y, 7000), unreal.Vector(88, 3.2, 3.2), unreal.Rotator(0, -8, 4))
    spawn_mesh(cylinder, "Crane_HookCable", unreal.Vector(crane_x - 7800, crane_y, 5200), unreal.Vector(0.18, 0.18, 36), collision=False)
    spawn_mesh(sphere, "Crane_Hook", unreal.Vector(crane_x - 7800, crane_y, 3350), unreal.Vector(1.2, 1.2, 1.8), collision=False)

    dome_x, dome_y = 33000, 36000
    spawn_mesh(cylinder, "Civic_DomeDrum", unreal.Vector(dome_x, dome_y, 3200), unreal.Vector(42, 42, 62), collision=False)
    spawn_mesh(sphere, "Civic_Dome", unreal.Vector(dome_x, dome_y, 6900), unreal.Vector(45, 45, 28), collision=False)
    spawn_mesh(cylinder, "Civic_DomeSpire", unreal.Vector(dome_x, dome_y, 9100), unreal.Vector(2, 2, 25), collision=False)


def assemble_harbor(game_mode_class):
    if not unreal.EditorAssetLibrary.does_asset_exist(HARBOR_ASSET):
        if not unreal.EditorAssetLibrary.duplicate_asset(
            "/Game/ThirdPerson/Lvl_ThirdPerson", HARBOR_ASSET
        ):
            raise RuntimeError("Unable to duplicate the Third Person template map")

    unreal.EditorLevelLibrary.load_level(HARBOR_ASSET)
    remove_previous_generated_actors()

    world = unreal.EditorLevelLibrary.get_editor_world()
    world_settings = world.get_world_settings()
    world_settings.set_editor_property("default_game_mode", game_mode_class)

    # Engine basic cube has deterministic simple collision in cooked builds.
    # The LevelPrototyping cube rendered correctly but did not block the
    # packaged CharacterMovement capsule on this UE 5.8 Mac build.
    cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
    cylinder = unreal.load_asset("/Engine/BasicShapes/Cylinder.Cylinder")
    sphere = unreal.load_asset("/Engine/BasicShapes/Sphere.Sphere")
    plane = unreal.load_asset("/Game/LevelPrototyping/Meshes/SM_Plane")
    ramp = unreal.load_asset("/Game/LevelPrototyping/Meshes/SM_Ramp")
    chamfer = unreal.load_asset("/Game/LevelPrototyping/Meshes/SM_ChamferCube")
    if not cube or not cylinder or not sphere or not plane or not ramp or not chamfer:
        raise RuntimeError("Prototype mesh pack is incomplete")

    # One-kilometre district base and waterfront.
    spawn_mesh(cube, "Harbor_Ground", unreal.Vector(0, 0, -50), unreal.Vector(1000, 1000, 1))
    # A thick local insertion slab provides deterministic collision immediately
    # beneath PlayerStart even before navigation and distant systems initialize.
    spawn_mesh(cube, "Insertion_Foundation", unreal.Vector(-45000, 0, -50), unreal.Vector(160, 160, 1))
    spawn_mesh(plane, "Harbor_Waterfront", unreal.Vector(0, -47000, 0), unreal.Vector(500, 80, 1))
    assemble_foreground_city(cube, cylinder, sphere, chamfer)

    # Tutorial insertion lane and an original security-office stairwell. The
    # previous office was one solid cube and could not support interior play.
    spawn_mesh(cube, "Insertion_Wall_A", unreal.Vector(-43000, 6000, 250), unreal.Vector(120, 3, 5))
    spawn_mesh(cube, "Insertion_Wall_B", unreal.Vector(-43000, -6000, 250), unreal.Vector(120, 3, 5))
    office = unreal.Vector(-30000, 12000, 0)
    spawn_mesh(cube, "Stairwell_Floor_Lower", office + unreal.Vector(0, 0, 25), unreal.Vector(90, 70, 0.5))
    spawn_mesh(cube, "Stairwell_Floor_Upper", office + unreal.Vector(0, 0, 2025), unreal.Vector(90, 70, 0.5))
    spawn_mesh(cube, "Stairwell_Wall_West", office + unreal.Vector(-4450, 0, 1100), unreal.Vector(1, 70, 22))
    spawn_mesh(cube, "Stairwell_Wall_East", office + unreal.Vector(4450, 0, 1100), unreal.Vector(1, 70, 22))
    spawn_mesh(cube, "Stairwell_Wall_North_A", office + unreal.Vector(-2600, 3450, 1100), unreal.Vector(37, 1, 22))
    spawn_mesh(cube, "Stairwell_Wall_North_B", office + unreal.Vector(2600, 3450, 1100), unreal.Vector(37, 1, 22))
    spawn_mesh(cube, "Stairwell_Wall_South_A", office + unreal.Vector(-2600, -3450, 1100), unreal.Vector(37, 1, 22))
    spawn_mesh(cube, "Stairwell_Wall_South_B", office + unreal.Vector(2600, -3450, 1100), unreal.Vector(37, 1, 22))
    # Two walkable flights and a landing create a close-quarters descent route.
    spawn_mesh(ramp, "Stairwell_Ramp_Upper", office + unreal.Vector(-2000, 900, 1050), unreal.Vector(32, 18, 18), unreal.Rotator(0, 0, 0))
    spawn_mesh(cube, "Stairwell_Landing", office + unreal.Vector(0, 900, 1025), unreal.Vector(16, 22, 0.5))
    spawn_mesh(ramp, "Stairwell_Ramp_Lower", office + unreal.Vector(2000, 900, 50), unreal.Vector(-32, 18, 18), unreal.Rotator(0, 0, 0))
    # Rail silhouettes and a luminous exit marker preserve depth/readability.
    for side in (-1, 1):
        spawn_mesh(cube, f"Stairwell_Rail_Upper_{side}", office + unreal.Vector(-2000, 900 + side * 1050, 1450), unreal.Vector(32, 0.08, 0.08))
        spawn_mesh(cube, f"Stairwell_Rail_Lower_{side}", office + unreal.Vector(2000, 900 + side * 1050, 500), unreal.Vector(32, 0.08, 0.08))
    spawn_mesh(cube, "Stairwell_ExitMarker", office + unreal.Vector(0, 3370, 1550), unreal.Vector(5, 0.2, 1.2))
    for index, location in enumerate([
        office + unreal.Vector(-2300, 900, 1750),
        office + unreal.Vector(0, 900, 1250),
        office + unreal.Vector(2300, 900, 700),
    ]):
        stair_light = ACTOR_SUBSYSTEM.spawn_actor_from_class(
            unreal.PointLight, location, unreal.Rotator(0, 0, 0), transient=False
        )
        if stair_light:
            generated(stair_light, f"Stairwell_Light_{index:02d}")
            persistent(stair_light)
            light_component = stair_light.get_editor_property("point_light_component")
            light_component.set_editor_property("intensity", 6500.0 if index != 1 else 9000.0)
            light_component.set_editor_property("attenuation_radius", 4200.0)
            light_component.set_editor_property(
                "light_color", unreal.Color(120, 190, 150, 255) if index == 1 else unreal.Color(255, 205, 155, 255)
            )

    # Warehouse shell with a readable entrance and interior cover lanes.
    warehouse_center = unreal.Vector(18000, 18000, 0)
    spawn_mesh(cube, "Warehouse_Floor", warehouse_center + unreal.Vector(0, 0, 50), unreal.Vector(260, 180, 1))
    spawn_mesh(cube, "Warehouse_Back", warehouse_center + unreal.Vector(0, 17500, 700), unreal.Vector(260, 5, 14))
    spawn_mesh(cube, "Warehouse_Left", warehouse_center + unreal.Vector(-25500, 0, 700), unreal.Vector(5, 180, 14))
    spawn_mesh(cube, "Warehouse_Right", warehouse_center + unreal.Vector(25500, 0, 700), unreal.Vector(5, 180, 14))
    for index in range(8):
        x = 7000 + (index % 4) * 7500
        y = 9000 + (index // 4) * 9000
        spawn_mesh(cube, f"Warehouse_Cover_{index:02d}", unreal.Vector(x, y, 150), unreal.Vector(18, 8, 3))

    # Container maze, deliberately alternating openings for two routes.
    colors = ["Red", "Blue", "Orange"]
    for row in range(7):
        for column in range(6):
            if (row + column) % 4 == 0:
                continue
            x = -12000 + column * 6000
            y = -22000 + row * 5000
            rotation = unreal.Rotator(0, 90 if row % 2 else 0, 0)
            spawn_mesh(
                cube,
                f"Container_{colors[(row + column) % len(colors)]}_{row:02d}_{column:02d}",
                unreal.Vector(x, y, 160),
                unreal.Vector(28, 12, 3.2),
                rotation,
            )

    # Elevated crane/traversal route with ramps and guard rails.
    for index in range(7):
        spawn_mesh(cube, f"Crane_Walkway_{index:02d}", unreal.Vector(-15000 + index * 5000, -7000, 1700), unreal.Vector(25, 5, 1))
    spawn_mesh(ramp, "Crane_Ramp_West", unreal.Vector(-19000, -7000, 500), unreal.Vector(30, 12, 10))
    spawn_mesh(ramp, "Crane_Ramp_East", unreal.Vector(19000, -7000, 500), unreal.Vector(-30, 12, 10))
    spawn_mesh(cube, "Crane_Tower", unreal.Vector(0, -9000, 4200), unreal.Vector(18, 18, 84))

    # Vehicle yard and final arena cover.
    for index in range(12):
        x = 23000 + (index % 4) * 6000
        y = -26000 + (index // 4) * 7000
        spawn_mesh(cube, f"VehicleYard_Cover_{index:02d}", unreal.Vector(x, y, 130), unreal.Vector(20, 10, 2.6))
    for index in range(10):
        angle = index * 36.0
        radius = 12000.0
        location = unreal.Vector(25000 + radius * math.cos(math.radians(angle)), -5000 + radius * math.sin(math.radians(angle)), 180)
        spawn_mesh(cube, f"FinalArena_Cover_{index:02d}", location, unreal.Vector(14, 6, 3.6), unreal.Rotator(0, angle, 0))

    # Extraction pad and optional manifest office.
    spawn_mesh(cube, "Extraction_Pad", unreal.Vector(43000, -38000, 30), unreal.Vector(90, 90, 0.6))
    spawn_mesh(cube, "Optional_Manifest_Office", unreal.Vector(36000, 36000, 350), unreal.Vector(65, 50, 7))

    # Dense original industrial skyline and route dressing. These silhouettes
    # create layered combat depth while keeping geometry deterministic and
    # license-safe for the packaged vertical slice.
    skyline = [
        (-42000, 42000, 72, 48, 210), (-25000, 45500, 50, 38, 145),
        (-5000, 44000, 64, 46, 250), (14000, 46000, 42, 34, 165),
        (30000, 43000, 58, 42, 225), (44500, 30000, 46, 36, 180),
        (46500, 9000, 34, 30, 135), (45000, -12000, 52, 38, 205),
    ]
    for index, (x, y, sx, sy, sz) in enumerate(skyline):
        spawn_mesh(
            cube, f"Skyline_Tower_{index:02d}", unreal.Vector(x, y, sz * 25.0),
            unreal.Vector(sx, sy, sz), collision=False,
        )
        spawn_mesh(
            cube, f"Neon_Skyline_{index:02d}",
            unreal.Vector(x - sx * 38.0, y - sy * 48.0, sz * 28.0),
            unreal.Vector(0.5, 0.35, max(10.0, sz * 0.58)), collision=False,
        )

    # Foreground barriers, pipe stacks and sparse wind-bent trees break up the
    # old empty plane and make each firing lane readable at human scale.
    for index in range(18):
        x = -41000 + index * 4800
        y = -32000 if index % 2 == 0 else 31000
        spawn_mesh(
            cube, f"Route_Barrier_{index:02d}", unreal.Vector(x, y, 110),
            unreal.Vector(16, 5, 2.2), unreal.Rotator(0, 8 if index % 2 else -8, 0),
        )
        spawn_mesh(
            cylinder, f"PipeStack_{index:02d}",
            unreal.Vector(x + 900, y + (700 if y < 0 else -700), 95),
            unreal.Vector(2.2, 2.2, 9.0), unreal.Rotator(90, 0, 0),
        )
    for index, (x, y) in enumerate([
        (-38000, 27000), (-29000, -35000), (-18000, 36000), (-4000, -39000),
        (9000, 35000), (20000, -36000), (31000, 33000), (39000, -27000),
    ]):
        spawn_mesh(
            cylinder, f"Harbor_TreeTrunk_{index:02d}", unreal.Vector(x, y, 420),
            unreal.Vector(1.5, 1.5, 8.0), collision=False,
        )
        spawn_mesh(
            sphere, f"Harbor_Canopy_{index:02d}", unreal.Vector(x, y, 1050),
            unreal.Vector(8.5, 8.5, 5.5), collision=False,
        )

    for index, location in enumerate([
        unreal.Vector(-36000, 0, 1800), unreal.Vector(-12000, -12000, 1900),
        unreal.Vector(12000, 12000, 2100), unreal.Vector(28000, -16000, 2100),
        unreal.Vector(43000, -38000, 1900),
    ]):
        spawn_mesh(
            cube, f"Neon_RouteBeacon_{index:02d}", location,
            unreal.Vector(0.6, 0.6, 14.0), collision=False,
        )

    # Create a persistent, explicit insertion spawn. The duplicated template map
    # may not retain its external PlayerStart when regenerated unattended.
    starts = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.PlayerStart)
    player_start = starts[0] if starts else ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(-45000, 0, 110),
        unreal.Rotator(0, 0, 0),
        transient=False,
    )
    if player_start:
        generated(player_start, "Harbor_PlayerStart")
        persistent(player_start)
        player_start.set_actor_location(unreal.Vector(-45000, 0, 110), False, False)
        player_start.set_actor_rotation(unreal.Rotator(0, 0, 0), False)

    # Runtime-safe outdoor lighting. These actors are persistent so World
    # Partition never streams the world into an unlit black frame.
    sun = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 18000),
        unreal.Rotator(roll=0.0, pitch=-52.0, yaw=-32.0),
        transient=False,
    )
    if sun:
        generated(sun, "Harbor_Sun")
        persistent(sun)
        sun_component = sun.get_editor_property("directional_light_component")
        sun_component.set_editor_property("intensity", 1.35)
        sun_component.set_editor_property("light_color", unreal.Color(190, 215, 255, 255))
        try:
            sun_component.set_editor_property("atmosphere_sun_light", True)
            sun_component.set_editor_property("use_temperature", True)
            sun_component.set_editor_property("temperature", 9800.0)
            sun_component.set_editor_property("source_angle", 3.0)
            sun_component.set_editor_property("volumetric_scattering_intensity", 1.35)
        except Exception:
            pass

    skylight = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 12000),
        unreal.Rotator(0, 0, 0),
        transient=False,
    )
    if skylight:
        generated(skylight, "Harbor_Skylight")
        persistent(skylight)
        sky_component = skylight.get_editor_property("light_component")
        sky_component.set_editor_property("intensity", 0.9)
        sky_component.set_editor_property("real_time_capture", True)

    atmosphere = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.SkyAtmosphere,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0),
        transient=False,
    )
    if atmosphere:
        generated(atmosphere, "Harbor_Atmosphere")
        persistent(atmosphere)

    cloud = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.VolumetricCloud,
        unreal.Vector(0, 0, 10000),
        unreal.Rotator(0, 0, 0),
        transient=False,
    )
    if cloud:
        generated(cloud, "Harbor_StormClouds")
        persistent(cloud)

    fog = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(0, 0, -200),
        unreal.Rotator(0, 0, 0),
        transient=False,
    )
    if fog:
        generated(fog, "Harbor_Fog")
        persistent(fog)
        fog_component = fog.get_editor_property("component")
        fog_component.set_editor_property("fog_density", 0.012)
        try:
            fog_component.set_editor_property("volumetric_fog", True)
            fog_component.set_editor_property("volumetric_fog_scattering_distribution", 0.75)
        except Exception:
            pass

    # Bounded gameplay grade: suppress the template's washed-out auto exposure
    # while retaining readable silhouettes and practical lights.
    post_process = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(0, 0, 1000),
        unreal.Rotator(0, 0, 0),
        transient=False,
    )
    if post_process:
        generated(post_process, "Harbor_PostProcess")
        persistent(post_process)
        try:
            post_process.set_editor_property("unbound", True)
            settings = post_process.get_editor_property("settings")
            settings.set_editor_property("override_auto_exposure_bias", True)
            settings.set_editor_property("auto_exposure_bias", 0.15)
            settings.set_editor_property("override_bloom_intensity", True)
            settings.set_editor_property("bloom_intensity", 0.28)
            settings.set_editor_property("override_vignette_intensity", True)
            settings.set_editor_property("vignette_intensity", 0.18)
            settings.set_editor_property("override_motion_blur_amount", True)
            settings.set_editor_property("motion_blur_amount", 0.12)
            post_process.set_editor_property("settings", settings)
        except Exception as error:
            unreal.log_warning(f"[PROJECT EMBER] Post process tuning fallback: {error}")

    for index, location in enumerate([
        unreal.Vector(-36000, 0, 1800),
        unreal.Vector(-12000, -12000, 2200),
        unreal.Vector(12000, 12000, 2400),
        unreal.Vector(28000, -16000, 2400),
        unreal.Vector(40000, -34000, 2200),
    ]):
        work_light = ACTOR_SUBSYSTEM.spawn_actor_from_class(
            unreal.PointLight, location, unreal.Rotator(0, 0, 0), transient=False
        )
        if work_light:
            generated(work_light, f"Harbor_WorkLight_{index:02d}")
            persistent(work_light)
            work_component = work_light.get_editor_property("point_light_component")
            work_component.set_editor_property("intensity", 950.0)
            work_component.set_editor_property("attenuation_radius", 6200.0)
            work_component.set_editor_property(
                "light_color",
                unreal.Color(65, 175, 255, 255)
                if index % 2 == 0 else unreal.Color(145, 190, 255, 255),
            )

    # Teal/orange pools lead the player across the authored combat route and
    # keep the player and enemies separated from the moonlit skyline.
    for index in range(14):
        route_light = ACTOR_SUBSYSTEM.spawn_actor_from_class(
            unreal.PointLight,
            unreal.Vector(-42000 + index * 6500, -25000 + (index % 3) * 19000, 1450),
            unreal.Rotator(0, 0, 0), transient=False,
        )
        if route_light:
            generated(route_light, f"Harbor_RouteLight_{index:02d}")
            persistent(route_light)
            component = route_light.get_editor_property("point_light_component")
            component.set_editor_property("intensity", 750.0)
            component.set_editor_property("attenuation_radius", 3600.0)
            component.set_editor_property(
                "light_color",
                unreal.Color(45, 170, 255, 255)
                if index % 3 else unreal.Color(120, 185, 255, 255),
            )

    nav = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.NavMeshBoundsVolume,
        unreal.Vector(0, 0, 1500),
        unreal.Rotator(0, 0, 0),
        transient=False,
    )
    if nav:
        generated(nav, "Harbor_NavigationBounds")
        persistent(nav)
        # The default brush is 200 uu. Scale 500 covers the intended 1 km x 1 km
        # harbor without generating tiles for the unimplemented 4 km region.
        nav.set_actor_scale3d(unreal.Vector(500, 500, 30))

    mission_director_class = unreal.load_class(None, "/Script/EmberMission.EmberMissionDirector")
    if mission_director_class:
        director = ACTOR_SUBSYSTEM.spawn_actor_from_class(
            mission_director_class,
            unreal.Vector(0, 0, 200),
            unreal.Rotator(0, 0, 0),
            transient=False,
        )
        if director:
            generated(director, "Harbor_MissionDirector")
            persistent(director)

    # Native Ember enemies use the project's damage and tactical-state contracts.
    enemy_class = unreal.load_class(None, "/Script/EmberAI.EmberEnemyCharacter")
    if enemy_class:
        enemy_locations = [
            unreal.Vector(-42000, 500, 250),
            unreal.Vector(-40500, -1800, 250),
            unreal.Vector(-39200, 2200, 250),
            unreal.Vector(-24000, 5000, 250),
            unreal.Vector(-8000, -18000, 250),
            unreal.Vector(8000, -12000, 250),
            unreal.Vector(16000, 12000, 250),
            unreal.Vector(26000, 18000, 250),
            unreal.Vector(25000, -16000, 250),
            unreal.Vector(33000, -4000, 250),
            unreal.Vector(41000, -33000, 250),
            unreal.Vector(-12000, 25000, 250),
            unreal.Vector(5000, 28000, 250),
            unreal.Vector(30000, 30000, 250),
            unreal.Vector(38000, -22000, 250),
        ]
        for index, location in enumerate(enemy_locations):
            enemy = ACTOR_SUBSYSTEM.spawn_actor_from_class(
                enemy_class,
                location,
                unreal.Rotator(0, 180, 0),
                transient=False,
            )
            if enemy:
                generated(enemy, f"Harbor_Enemy_{index:02d}")
                if index < 3:
                    enemy.set_editor_property(
                        "tags", [unreal.Name("EmberGenerated"), unreal.Name("EmberEnemy"), unreal.Name("EmberPatrolOne")]
                    )
                persistent(enemy)

    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unable to save harbor map")
    log("Harbor vertical-slice environment saved")


def main():
    unreal.EditorAssetLibrary.make_directory(BLUEPRINT_PATH)
    unreal.EditorAssetLibrary.make_directory(MAP_PATH)
    configure_material_library()
    character_class = configure_character_blueprint()
    game_mode_class = configure_game_mode(character_class)
    assemble_harbor(game_mode_class)
    log("Blueprint and map assembly complete")


main()
