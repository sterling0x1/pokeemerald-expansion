# Changelog

This changelog covers the custom project milestones. It does not list every
individual upstream pokeemerald-expansion commit included through upstream
merges.

## Unreleased

## 2.9.7 - 2026-08-15

### Battle UI

- Added status badges to the compact Bench Attacker Pokémon selector.
- Added a compact evolution indicator showing levels remaining or the general evolution method.
- Added safe cleanup for the new selector status sprites.
- Fixed the Poké Ball quick shortcut showing an invalid item after the final ball was used.

### Project maintenance

- Integrated the latest upstream Expansion changes and fixes.
- Added source credits for the expanded Game Corner, Gacha system and modern Shop UI.
- Enabled compressed overworld graphics to reduce ROM usage.
- Removed obsolete packaged plugin copies that duplicated systems already present in the project.

## 2.9.6 - 2026-08-15

### Battle fixes

- Fixed independent move selection for both player battlers in double battles.
- Fixed the wrong Pokémon's moves appearing during double-battle selection.
- Fixed selectors becoming stuck after a Pokémon had already acted.
- Added explicit tracking for committed Bench Attacker turns.
- Fixed Gorilla Tactics incorrectly locking Bench Attacker moves.
- Improved Bench Attacker support in double battles.
- Added modular half-damage support for Bench Attacker attacks.
- Added per-battler floating damage and healing number state.
- Reset temporary Active Battle state when starting a battle.
- Added crit-timer support for compatible multi-hit moves such as Pin Missile.
- Improved Active Battle support in Battle Frontier battles.

### Compatibility fixes

- Fixed the Move Relearner reporting a move as learned without replacing one.
- Fixed randomized Move Relearner mappings and duplicate moves.
- Updated the Wally tutorial and Oak/Old Man scripted battles to use the modern battle menu safely.
- Improved Game Corner prize and Gacha handling.
- Improved normal, variable-price, coin, point and decoration shop routing.

### Modules

- Added independent modules for Bench Attacker damage, Game Corner, Key Item Dock, modern Shop UI and Shared Transfer Box.
- Added module-aware runtime checks so disabled features do not affect vanilla behaviour.

## 2.9.5 - 2026-08-13

- Merged a newer pokeemerald-expansion upstream version and its official fixes.
- Integrated the expanded Game Corner and Gacha feature branch.
- Integrated the modern Shop UI feature branch.
- Added coin, Battle Point and variable-price shop support.
- Added seller graphics, improved shop navigation and expanded purchase handling.
- Fixed Game Corner prize sellers.
- Fixed the Thunder Stone seller immediately attempting a purchase.
- Fixed Free Shop compatibility with the modern Shop UI.
- Added floating damage numbers.
- Added green healing numbers.
- Added a configurable Bench Attacker damage reduction.
- Updated the project feature list.

## 2.9.4 - 2026-08-09

- Completed the Key Item Dock.
- Added a macOS Dock-style key item selector.
- Added scrolling between registered key items.
- Enlarged the selected key item.
- Added the selected item's name above the dock.
- Wired item activation to the selector.
- Modularised the Key Item Dock so it can be removed independently.

## 2.9.3 - 2026-08-07

- Added the first Key Item Bar/Dock prototype.
- Added an overworld item carousel for registered key items.
- Added initial selection, scrolling and item-name presentation.

## 2.9.2 - 2026-08-07

- Extended Active Battle critical and dodge actions to double battles.
- Added independent Active Battle handling for multiple battlers.
- Preserved Bench Attacker behaviour during double battles.

## 2.9.1 - 2026-08-07

- Fixed Bench Attacker selection in double battles.
- Fixed move-list ownership when switching between the player's two battlers.
- Improved double-battle selector navigation and turn progression.

## 2.9c - 2026-08-05

- Replaced the dodge timing bar with an on-battler dodge ring.
- Positioned the dodge action directly in the battle scene.
- Added separate presentation for critical and dodge actions.
- Reduced leftover window and background artefacts after timing actions.

## 2.9b - 2026-08-05

- Added the Active Battle critical-hit timing system.
- Added the Active Battle dodge timing system.
- Assigned the R Button as the action trigger.
- Added success, miss and perfect-dodge handling.
- Separated timing behaviour from normal battle-speed settings.

## 2.9a - 2026-08-05

- Audited and tightened module isolation across the project.
- Fixed module-dependent menu entries appearing when their module was disabled.
- Improved the Cheat Menu layout and selection display.
- Fixed Nuzlocke encounter tracking so running from the first encounter does not consume the area catch.
- Prepared the modular base for the 2.9 Active Battle development cycle.

## 2.8.16 - 2026-08-05

- Polished the game-mode selector layout.
- Realigned mode names, labels and descriptions.
- Improved the Continue/Save Slot information layout.
- Polished the Shared Transfer Box screens and prompts.
- Improved selector hints and button labels.
- Standardised several menu panels and borders.

## 2.8.15 - 2026-08-04

- Confirmed transfers between the two save slots through the Shared Transfer Box.
- Added a save confirmation before finalising a shared transfer.
- Prevented the Shared Transfer Box from appearing in Nuzlocke mode.
- Improved shared-box status and capacity information.

## 2.8.14 - 2026-08-04

- Added a Shared Transfer Box between the two save slots.
- Added a dedicated shared PC box.
- Added access to the Shared Box from the Pokémon selection flow.
- Kept the system modular so its reserved save space can be reclaimed when disabled.

## 2.8.13 - 2026-08-03

- Added the New Game game-mode selector.
- Added Vanilla, Nuzlocke and Carnage choices.
- Added save-slot mode labels.
- Added Carnage-only Pokémon level randomisation.
- Prevented Carnage settings from being changed through the normal option controls.
- Improved the selector background, descriptions and navigation.

## 2.8.12 - 2026-08-03

- Polished the Pokémon party and PC selectors.
- Replaced the party cursor with the modern battle-menu arrow.
- Removed cursor-movement screen flashing.
- Added the R Button shortcut from the party selector to the PC.
- Added L Button pick-up, drop and party-replacement controls in the PC.
- Added compact in-game hints for the new PC controls.

## 2.8.11 - 2026-08-02

- Extracted overworld options into an independent module.
- Added module-aware defaults for overworld features.
- Kept disabled modules from changing vanilla overworld behaviour.

## 2.8.10 - 2026-08-02

- Extracted Difficulty into an independent module/API.
- Added modular Easy, Normal and Hard difficulty handling.
- Connected trainer scaling to the selected difficulty.
- Fixed trainer-escape rematch behaviour after the difficulty integration.

## 2.8.9 - 2026-08-02

- Added the modular icon-based Start Menu.
- Allowed the Start Menu package to be enabled or removed independently.
- Preserved vanilla menu behaviour when the module is disabled.

## 2.8.8 - 2026-08-01

- Extracted Progression into an independent module.
- Extracted Pokémon Rules into an independent module.
- Added independent Level Cap handling.
- Added module-safe defaults when progression or rule modules are disabled.

## 2.8.7 - 2026-07-31

- Extracted Battle Pacing into an independent module.
- Renamed the Instant pacing option to Faster.
- Added L/R page indicators to the settings menu.
- Hid page indicators when the modular Options Menu is disabled.

## 2.8.6 - 2026-07-31

- Completed Quality-of-Life module isolation.
- Made QoL features independently removable.
- Ensured disabled QoL modules return to vanilla defaults.
- Fixed script-macro compatibility caused by runtime checks in event scripts.

## 2.8.5 - 2026-07-31

- Began modularising the Quality-of-Life settings.
- Added proper trainer-battle escape behaviour.
- Allowed escaped trainers to be challenged again.
- Prevented an escaped trainer from immediately retriggering the battle.

## 2.8.4 - 2026-07-31

- Renamed the combined randomizer preset to Carnage Mode.
- Added Carnage as an exclusive preset that locks conflicting randomizer settings.
- Exposed compatible randomizer features as separate switches outside Carnage.
- Added modular randomizer controls and vanilla-safe defaults.

## 2.8.3 - 2026-07-30

- Added and isolated the Cheat Menu module.
- Added Money and EXP multipliers.
- Added Catch Rate controls.
- Added Egg Hatch Speed controls.
- Added EV Gain controls.
- Added Mart Price controls.
- Added Infinite Repel with an in-game requirement note.
- Kept the Expansion Debug Menu separate from the Cheat Menu.

## 2.8.2 - 2026-07-30

- Isolated Nuzlocke as a removable game-mode module.
- Added module-aware Nuzlocke entry points and runtime checks.
- Verified both enabled and disabled builds.
- Fixed conflicting New Game menu cases introduced during extraction.

## 2.8.1 - 2026-07-30

- Saved the Kanto campaign prototype as a separate checkpoint.
- Added an early Hoenn/Kanto region selector.
- Added initial Kanto intro, map and battle compatibility work.
- Preserved this as a prototype branch rather than part of the stable modular base.

## 2.8 - 2026-07-30

- Added two independent save slots.
- Added save-slot switching on the main menu.
- Added left/right slot indicators and slot descriptions.
- Preserved full save data for both slots.
- Included the corrected low-level EXP-bar handling.

## 2.7.6n - 2026-07-30

- Added Nuzlocke as a separate New Game mode.
- Added Nuzlocke presets, custom rules and rule descriptions.
- Added one encounter per area.
- Added fainted Pokémon handling and dead-box restrictions.
- Added level-cap and continue/game-over rules.
- Added Nuzlocke-aware encounter, party and item restrictions.
- Added a return path from the Randomizer Menu to Nuzlocke settings.
- Added the first Cheat Menu implementation.
- Fixed Nuzlocke game-over behaviour overwriting an existing non-Nuzlocke save.

## 2.7.5 - 2026-07-29

- Added Fast Intro.
- Fixed Fast Intro damaging or clearing intro backgrounds.
- Preserved the corrected low-level EXP-bar handling.

## 2.7.4 - 2026-07-29

- Added Fast HP bars.
- Fixed the Fast EXP freeze affecting low-level Pokémon.
- Corrected low-level EXP-bar rendering.

## 2.7.3 - 2026-07-29

- Added selectable 1x, 2x, 3x and 4x EXP multipliers.
- Stored the multiplier in the save options.
- Applied the multiplier without changing unrelated EXP behaviour.

## 2.7.2 - 2026-07-29

- Added the R Button shortcut from the Pokémon selector to the PC Box.
- Added an on-screen `R: Box` hint.

## 2.7 - 2026-07-26

- Stabilised the Bench Attacker runtime system.
- Cleaned up reserve-attacker formatting and battle integration.
- Improved reserve selection and move display handling.

## 2.6 - 2026-07-23

- Added the New Game Randomizer Menu.
- Added persistent randomizer seeds and in-game seed rerolling.
- Added wild, trainer, gift, static, starter and item randomizer hooks.
- Added move, ability and evolution randomizer options.
- Added randomizer-aware New Game setup.
- Expanded the Options Menu with scrolling support.
- Added the initial Bench Attacker integration.

## 2.5 - 2026-07-23

- Added the first full Bench Attacker battle implementation.
- Added reserve Pokémon selection during battle.
- Added reserve move selection and execution.
- Added party-menu and battle-controller support for reserve attacks.
- Added initial compact battle integration for the new system.

## 2.0 - 2026-07-22

- Added the reusable Modern Battle Menu package.
- Replaced the standard fight interface with the compact modern layout.
- Added move information directly to the battle command area.
- Added type and effectiveness presentation.
- Added reusable battle UI configuration and assets.

## 1.5 - 2026-07-22

- Added visible overworld encounters.
- Added the compact font package.
- Integrated the compact font into the new battle presentation.

## 1.0 - 2026-07-22

- Added the first compact battle move UI.
- Established the initial visual foundation for the modern battle system.
