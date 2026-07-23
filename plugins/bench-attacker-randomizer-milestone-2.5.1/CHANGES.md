# Milestone 2.5.1 changes

## Added since Milestone 2.5

1. `src/randomizer.c` provides deterministic species selection without consuming the normal game RNG.
2. `src/wild_encounter.c` applies the randomizer immediately before ordinary wild Pokémon are created.
3. `src/starter_choose.c` uses the same randomizer for starter previews, labels, cries, and the final received starter.
4. `include/config/randomizer.h` exposes two enable toggles and an editable seed.

## Scope

- Included: starters, ordinary wild encounters, legendaries, and visible overworld encounters that generate through the normal wild path.
- Excluded: trainers, gifts/static encounters, moves, abilities, evolutions, items, a seed-entry screen, and an in-game settings menu.
