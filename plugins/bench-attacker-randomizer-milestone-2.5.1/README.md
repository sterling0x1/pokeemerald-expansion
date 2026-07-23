# Bench Attacker + Randomizer — Milestone 2.5.1

This is the working snapshot after Milestone 2.5 and the first Randomizer 1.0 pass.

## Included

- Modern compact battle menu and move-info panels from Milestone 2.0.
- Bench-attacker battle prototype from Milestone 2.5: select any healthy party Pokémon to use one move while the lead remains visually active.
- Compact party selector, command menu, and opponent info card.
- Safe off-field attacker HP/faint handling.
- Randomizer 1.0: randomizes starters and ordinary wild Pokémon, including legendaries.
- Visible overworld encounters use the normal wild-generation path, so they use the randomized species too.

## Randomizer settings

The first pass is build-time configuration, deliberately kept separate from save data:

- `RANDOMIZER_WILD_POKEMON`
- `RANDOMIZER_STARTERS`
- `RANDOMIZER_SEED`

Edit them in `sources/include/config/randomizer.h`, then rebuild. The seed creates stable results per original species and map. It is not yet an in-game menu.

## Contents

`sources/` mirrors the current project paths. These are complete source-file snapshots; compare and merge them into another customised hack rather than blindly overwriting files.

## Next step

Randomizer 1.1 will add a New Game setup screen with saved Wild/Starter toggles and a player-selected seed.

## Provenance

Created on 22 July 2026. This is an addon-folder and zip snapshot, not a Git tag, because the project working tree contains uncommitted custom work.
