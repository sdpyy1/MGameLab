# AuraSample

This plugin contains the Aura top-down RPG sample migrated from the original GameLab project. The source module remains named `Aura` to preserve serialized class paths used by existing assets.

All sample assets are stored in `Content/Aura` and are mounted at `/AuraSample/Aura` when the plugin is enabled.

General-purpose code belongs in `MGameLabCore`. Aura-specific abilities, attributes, combat interfaces, widgets, AI, and character classes stay here. This project's input component, asset manager, gameplay cue path, and collision channel are configured in `Config`.
