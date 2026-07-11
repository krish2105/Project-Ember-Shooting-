import math
import unreal


ROOT = "/Game/Ember"
BLUEPRINT_PATH = f"{ROOT}/Blueprints"
MAP_PATH = f"{ROOT}/Maps"
CHARACTER_NAME = "BP_EmberCharacter"
GAMEMODE_NAME = "BP_EmberGameMode"
HARBOR_ASSET = f"{MAP_PATH}/L_HarborVerticalSlice"
ACTOR_SUBSYSTEM = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def log(message):
    unreal.log(f"[PROJECT EMBER] {message}")


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
        None, "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"
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
        "relative_rotation", unreal.Rotator(0.0, -90.0, 0.0)
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


def spawn_mesh(mesh, label, location, scale, rotation=None):
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
    component.set_collision_profile_name("BlockAll")
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    generated(actor, label)
    # The vertical slice is intentionally small enough to keep its authored
    # blockout resident. Without this, a packaged World Partition game can
    # spawn the pawn before the insertion cell (and its collision) streams in,
    # causing the player to fall forever through an otherwise valid harbor.
    persistent(actor)
    actor.set_actor_scale3d(scale)
    return actor


def remove_previous_generated_actors():
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if unreal.Name("EmberGenerated") in actor.get_editor_property("tags"):
            unreal.EditorLevelLibrary.destroy_actor(actor)


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

    cube = unreal.load_asset("/Game/LevelPrototyping/Meshes/SM_Cube")
    plane = unreal.load_asset("/Game/LevelPrototyping/Meshes/SM_Plane")
    ramp = unreal.load_asset("/Game/LevelPrototyping/Meshes/SM_Ramp")
    if not cube or not plane or not ramp:
        raise RuntimeError("Prototype mesh pack is incomplete")

    # One-kilometre district base and waterfront.
    spawn_mesh(cube, "Harbor_Ground", unreal.Vector(0, 0, -150), unreal.Vector(1000, 1000, 1))
    # A thick local insertion slab provides deterministic collision immediately
    # beneath PlayerStart even before navigation and distant systems initialize.
    spawn_mesh(cube, "Insertion_Foundation", unreal.Vector(-45000, 0, -100), unreal.Vector(160, 160, 2))
    spawn_mesh(plane, "Harbor_Waterfront", unreal.Vector(0, -47000, 0), unreal.Vector(500, 80, 1))

    # Tutorial insertion lane and security office.
    spawn_mesh(cube, "Insertion_Wall_A", unreal.Vector(-43000, 6000, 250), unreal.Vector(120, 3, 5))
    spawn_mesh(cube, "Insertion_Wall_B", unreal.Vector(-43000, -6000, 250), unreal.Vector(120, 3, 5))
    spawn_mesh(cube, "Security_Office", unreal.Vector(-30000, 12000, 400), unreal.Vector(90, 70, 8))

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

    # Create a persistent, explicit insertion spawn. The duplicated template map
    # may not retain its external PlayerStart when regenerated unattended.
    starts = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.PlayerStart)
    player_start = starts[0] if starts else ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(-45000, 0, 150),
        unreal.Rotator(0, 0, 0),
        transient=False,
    )
    if player_start:
        generated(player_start, "Harbor_PlayerStart")
        persistent(player_start)
        player_start.set_actor_location(unreal.Vector(-45000, 0, 150), False, False)
        player_start.set_actor_rotation(unreal.Rotator(0, 0, 0), False)

    # Runtime-safe outdoor lighting. These actors are persistent so World
    # Partition never streams the world into an unlit black frame.
    sun = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 18000),
        unreal.Rotator(-42, -28, 0),
        transient=False,
    )
    if sun:
        generated(sun, "Harbor_Sun")
        persistent(sun)
        sun.get_editor_property("directional_light_component").set_editor_property("intensity", 10.0)

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
        sky_component.set_editor_property("intensity", 1.2)
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

    fog = ACTOR_SUBSYSTEM.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(0, 0, -200),
        unreal.Rotator(0, 0, 0),
        transient=False,
    )
    if fog:
        generated(fog, "Harbor_Fog")
        persistent(fog)
        fog.get_editor_property("component").set_editor_property("fog_density", 0.006)

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
            work_component.set_editor_property("intensity", 18000.0)
            work_component.set_editor_property("attenuation_radius", 18000.0)

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
                persistent(enemy)

    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unable to save harbor map")
    log("Harbor vertical-slice blockout saved")


def main():
    unreal.EditorAssetLibrary.make_directory(BLUEPRINT_PATH)
    unreal.EditorAssetLibrary.make_directory(MAP_PATH)
    character_class = configure_character_blueprint()
    game_mode_class = configure_game_mode(character_class)
    assemble_harbor(game_mode_class)
    log("Blueprint and map assembly complete")


main()
