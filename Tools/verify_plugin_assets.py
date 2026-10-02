import unreal


for class_path in (
    "/Script/MGameLabCore.MGCharacterBase",
    "/Script/MGameLabCore.MGLogBlueprintLibrary",
):
    if unreal.load_class(None, class_path) is None:
        raise RuntimeError("Class failed to load: " + class_path)
    print("VERIFY CLASS OK " + class_path)

unreal.MGLogBlueprintLibrary.print_screen_log("MGameLabCore verification", duration=0.1)


assets = [
    "/TemplateSharedAssets/Characters/Mannequins/Meshes/SKM_Manny_Simple",
    "/TemplateSharedAssets/Input/IMC_Default",
    "/TemplateSharedAssets/Weapons/Pistol/Meshes/SK_Pistol",
    "/ThirdPersonTemplate/ThirdPerson/Blueprints/BP_ThirdPersonCharacter",
    "/ThirdPersonTemplate/ThirdPerson/Blueprints/BP_ThirdPersonGameMode",
    "/ThirdPersonTemplate/ThirdPerson/Lvl_ThirdPerson",
    "/ThirdPersonTemplate/Variant_Combat/Lvl_Combat",
    "/ThirdPersonTemplate/Variant_Platforming/Lvl_Platforming",
    "/ThirdPersonTemplate/Variant_SideScrolling/Lvl_SideScrolling",
    "/FirstPersonTemplate/FirstPerson/Blueprints/BP_FirstPersonCharacter",
    "/FirstPersonTemplate/FirstPerson/Blueprints/BP_FirstPersonGameMode",
    "/FirstPersonTemplate/FirstPerson/Lvl_FirstPerson",
    "/FirstPersonTemplate/Variant_Horror/Lvl_Horror",
    "/FirstPersonTemplate/Variant_Shooter/Lvl_Shooter",
    "/AuraSample/Aura/Maps/StartupMap",
]

failed = []
for path in assets:
    asset = unreal.load_asset(path)
    if asset is None:
        failed.append(path)
        print("VERIFY FAIL " + path)
    else:
        print("VERIFY OK " + path)

if failed:
    raise RuntimeError("Assets failed to load: " + ", ".join(failed))

for path in (
    "/ThirdPersonTemplate/ThirdPerson/Lvl_ThirdPerson",
    "/ThirdPersonTemplate/Variant_Combat/Lvl_Combat",
    "/ThirdPersonTemplate/Variant_Platforming/Lvl_Platforming",
    "/ThirdPersonTemplate/Variant_SideScrolling/Lvl_SideScrolling",
    "/FirstPersonTemplate/FirstPerson/Lvl_FirstPerson",
    "/FirstPersonTemplate/Variant_Horror/Lvl_Horror",
    "/FirstPersonTemplate/Variant_Shooter/Lvl_Shooter",
    "/AuraSample/Aura/Maps/StartupMap",
    "/AuraSample/Aura/Maps/Dungeon",
    "/AuraSample/Aura/Maps/EnemyTest",
    "/AuraSample/Aura/Maps/EQS_TesingMap",
):
    world = unreal.EditorLoadingAndSavingUtils.load_map(path)
    if world is None:
        raise RuntimeError("Map failed to open: " + path)
    actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor)
    print("VERIFY MAP OK " + path + " actors=" + str(len(actors)))
    if not actors:
        raise RuntimeError("Map has no loaded actors: " + path)
