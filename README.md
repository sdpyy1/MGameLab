# MGameLab

This is a clean Unreal Engine 5.6 C++ project. Its original project module is unchanged. All imported systems live in project plugins.

`Content/Thirdparty` is copied locally from the original GameLab project. It is intentionally ignored by Git because it is approximately 7 GB. A fresh clone needs those assets copied into the same path before opening levels that reference them.

| Plugin | Purpose | Entry map |
| --- | --- | --- |
| `MGameLabCore` | Shared Blueprint screen log and `AMGCharacterBase` | N/A |
| `ThirdPersonTemplate` | UE third-person C++ template, content, and variants | `/ThirdPersonTemplate/ThirdPerson/Lvl_ThirdPerson` |
| `FirstPersonTemplate` | UE first-person C++ template, content, and variants | `/FirstPersonTemplate/FirstPerson/Lvl_FirstPerson` |
| `AuraSample` | Aura C++ module and content from GameLab | `/AuraSample/Aura/Maps/StartupMap` |
| `TemplateSharedAssets` | Character, input, weapon, and level assets used by both UE templates | N/A |

Enable **Show Plugin Content** in the Content Browser to see the maps. The default project map remains blank; select a plugin map when working on that sample. Each template plugin declares `TemplateSharedAssets` as a dependency, so copy that plugin with either template when reusing it in another project. Their content redirects are in each plugin's `Config/Base*.ini` file.

`MGameLabCore` exposes **Print Screen Log** under `MGameLab|Log` in Blueprints. `AMGCharacterBase` is an otherwise empty `ACharacter` subclass in `Source/MGameLabCore/Public/Character`.

The Aura plugin is enabled for this project. Aura-specific engine settings are in `Config/DefaultEngine.ini`, `DefaultInput.ini`, `DefaultGame.ini`, and `DefaultGameplayTags.ini`.

Verification: build `MGameLabEditor` with `Engine/Build/BatchFiles/Build.bat`, then run `Tools/verify_plugin_assets.py` through Unreal's `PythonScript` commandlet with `-EnablePlugins=PythonScriptPlugin`. It loads representative assets and opens all imported maps. The Python plugin is only needed for this check.
