# Bench Attacker Battle System — Milestone 2.5

Reusable add-on for a `pokeemerald-expansion` hack using the Modern Battle Menu Milestone 2.0 package.

## What it adds

- In supported single battles, any non-fainted party Pokémon can be selected to use one move for the turn.
- The lead Pokémon remains visually active: this is an **attacker selection**, not a conventional switch.
- The chosen Pokémon supplies its own moves, PP, battle stats, and damage for that action.
- Pressing **B** on the move list returns to the attacker picker; pressing **B** there returns to the battle-command menu.
- The party picker is displayed alongside the battle commands rather than opening the normal full-screen party menu.
- The command screen contains a compact opponent card with HP, type, and weakness information.
- A reserve attack temporarily has no ability, by design for this first prototype.
- Recoil/self-damage from an off-field attacker no longer overwrites the lead Pokémon's health box.
- A reserve attacker that faints is removed from selection without freezing the battle; the lead remains on screen.

## Supported scope

This first version is intentionally limited to normal one-versus-one player battles:

- standard wild battles;
- standard single trainer battles.

It deliberately excludes doubles, link battles, raids, Safari/tutorial battles, Frontier and partner formats, recordings, secret-base/trainer-hill variants, and other special battle types. Those formats need their own turn-order and target-selection work.

## Package contents

`sources/` mirrors the project paths of the current Milestone 2.5 source files.

| File | Purpose |
| --- | --- |
| `include/battle.h` | Stores temporary selected-attacker state and the saved lead battler state. |
| `include/battle_util.h` | Declares the reserve-attacker faint handler. |
| `include/constants/party_menu.h` | Adds the attacker-selection party action constant. |
| `src/battle_main.c` | Adds the supported-battle gate, selection state, and move/PP lookup from the selected party slot. |
| `src/battle_util.c` | Temporarily applies a reserve attack, restores the lead, and handles an off-field faint safely. |
| `src/battle_script_commands.c` | Routes reserve-attacker faints away from the normal on-field faint flow. |
| `src/battle_controllers.c` | Prevents an off-field HP update from drawing over the lead health box. |
| `src/battle_controller_player.c` | Draws the compact attacker picker, command menu, opponent card, and move UI navigation. |
| `src/battle_bg.c` | Defines the compact command/opponent-panel layout. |
| `src/party_menu.c` | Supports the dedicated attacker-choice action. |

## Installation into another hack

1. Start with a compatible `pokeemerald-expansion` project that already has the **Modern Battle Menu — Milestone 2.0** add-on installed.
2. Back up every destination file first.
3. Compare each file under `sources/` with the matching target file and merge the changes. The files are complete source snapshots, so do not blindly overwrite unrelated custom work.
4. Build the ROM.
5. Run the checklist in [TESTING.md](TESTING.md).

## Important implementation notes

- This system swaps the chosen party member into the battle calculation data only for its selected action, then restores the visible lead immediately afterwards.
- It is not a traditional switch: the lead sprite, persistent active slot, and normal command flow stay in place.
- The prototype intentionally suppresses an off-field HP-bar animation rather than trying to render two simultaneous player health boxes.
- Party HP and PP are still the authoritative data. Always test damage, recoil, fainting, and post-battle persistence after merging into a different hack.
- This package is a source snapshot, not a one-click patch. That is safer for heavily customised ROM hacks.

## Relationship to earlier milestones

- **Milestone 2.0**: compact modern move menu, font, move details, and effectiveness marker.
- **Milestone 2.5**: attacker picker, bench-attacker action flow, compact command/opponent panels, and off-field HP/faint protection.

## Deliberately excluded

Overworld Pokémon and random-encounter changes are separate project features and are not dependencies of this addon.

## Provenance

This package is the Milestone 2.5 working snapshot created on 22 July 2026. It is intentionally kept as a folder and zip archive so it can be copied into future hacks even before a Git release commit is made.
