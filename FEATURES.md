# What features are included?

## Table of Contents
- [Custom project features](#custom-project-features)
  - [Game modes and saves](#game-modes-and-saves)
  - [Randomizer](#randomizer)
  - [Battle systems](#battle-systems)
  - [Menus and controls](#menus-and-controls)
  - [Progression and quality of life](#progression-and-quality-of-life)
  - [Game Corner and shops](#game-corner-and-shops)
  - [Modular architecture](#modular-architecture)
- [Upstream pokeemerald-expansion features](#upstream-pokeemerald-expansion-features)
  - [Configuration files](#configuration-files)
  - [Upgraded Battle Engine](#upgraded-battle-engine)
  - [Full Trainer customization](#full-trainer-customization)
  - [Pokémon data](#pokémon-data)
  - [Interface improvements](#interface-improvements)
  - [Engine improvements](#engine-improvements)
  - [Overworld improvements](#overworld-improvements)
  - [Developer tools](#developer-tools)

## Custom project features

### Game modes and saves

- **Vanilla:** the standard Hoenn adventure with optional project settings.
- **Nuzlocke:** a separate New Game mode with presets, custom rules, level caps, encounter limits, fainted-Pokémon handling and Nuzlocke-specific save protection.
- **Carnage:** an exclusive randomizer mode with locked randomizer settings and randomized Pokémon levels.
- **Two save slots:** two independent full saves selectable from the main menu.
- **Shared Transfer Box:** an optional shared Pokémon box between supported save slots, with transfer confirmation and Nuzlocke restrictions.

### Randomizer

- Persistent save-specific seeds with in-game seed rerolling.
- Separate controls for wild Pokémon, trainers, starters, gifts, static encounters, items, moves, abilities and evolutions.
- Randomizer-aware starters, encounters, trainers and Move Relearner handling.
- Module-safe defaults that restore normal game data when the Randomizer module is disabled.

### Battle systems

- **Modern Battle UI:** compact command and move-selection presentation with reusable scripted-battle support.
- **Bench Attacker:** reserve party Pokémon can contribute attacks, including double-battle support and optional half damage.
- **Active Battle:** R-button critical-hit and dodge timing that runs independently from battle-speed settings.
- Floating damage numbers and green healing numbers.
- Modern battle cursors and improved party/move selector behaviour.
- Trainer-battle escape support with safe rematches.

### Menus and controls

- Extended scrolling Options Menu with modular pages and L/R indicators.
- Modular Cheat Menu with EXP, money, catch-rate, hatch-speed, EV, Mart-price and Infinite Repel controls.
- Icon-based Start Menu.
- Key Item Dock with scrolling, selection enlargement, item names and direct activation.
- R-button shortcut from the party selector to the PC.
- L-button quick pick-up, drop and party replacement in the PC.

### Progression and quality of life

- Configurable EXP and money multipliers.
- Fast EXP, Fast HP, Faster Intro and text-speed options.
- Modular Difficulty API with Easy, Normal and Hard settings.
- Independent Level Cap and Pokémon Rules modules.
- Modular overworld and quality-of-life settings with vanilla-safe defaults.

### Game Corner and shops

- Expanded Mauville Game Corner with Gacha and additional minigames.
- Modern Shop UI with seller graphics and improved item presentation.
- Normal, variable-price, coin, Battle Point and decoration shop support.
- Cheat-price and Free Shop compatibility.

### Modular architecture

- Major custom systems are registered through the project module manager.
- Modules can hide their menus and disable their runtime hooks independently.
- Disabled modules fall back to vanilla or upstream Expansion behaviour.
- Current modules cover game modes, randomization, Nuzlocke, cheats, QoL, battle pacing, progression, Pokémon rules, difficulty, overworld features, Game Corner, Shop UI, Key Item Dock, Shared Transfer Box and Bench Attacker damage.

For milestone-by-milestone changes, see the [project changelog](CHANGELOG.md).

---

## Upstream pokeemerald-expansion features

## Configuration files
A lot of features listed below can be turned off as desired. Check which ones in these files
- [AI config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/ai.h)
- [Battle config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/battle.h)
- [Caps config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/caps.h)
- [Debug config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/debug.h)
- [DexNav config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/dexnav.h)
- [General config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/general.h)
- [HGSS Pokédex config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/pokedex_plus_hgss.h)
- [Item config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/item.h)
- [NPC Follower config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/follower_npc.h)
- [Overworld config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/overworld.h)
- [Pokémon config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/pokemon.h)
- [Save config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/save.h)
- [Species enabled](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/species_enabled.h)
- [Summary screen config](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/config/summary_screen.h)

## Upgraded Battle Engine
- ***Battle gimmicks:*** Mega Evolution, Primal Reversion, Ultra Burst, Z-Moves, Dynamax, Gigantamax and Terastallization.
- ***Newer game battle types:*** Double Wild Battles, custom Multi Battles, Inverse Battles, 1v2/2v1 battles, Sky Battles.
- ***Updated battle mechanics:*** Critical capture, Frostbite support, Poké Ball quick menu, Move description menu, no badge boosts, Gen 4 Fog, obedience, Affection, Party swap upon catch, move effectiveness in battle, FRLG/Gen4+ whiteout money calculation, Gen 4-style shadows.
- ***Updated move data***: Fairy/Stellar types, Physical/Special split, flags.
- ***Updated calculations:*** Damage, experience, mid-turn speed, end-battle stats and EVs, Level 100 EVs.
- ***Every item, ability and move effect up to Gen IX:*** Includes contest data up to SwSh ([source](https://pokemonurpg.com/info/contests/rse-move-list/)).
- ***Initial battle conditions:*** Stat stages, battle terrain, Wild AI flags.
- ***Faster battles:*** Simultaneous HP reduction, shortcut to "Run" option, faster battle intro, faster HP drain, faster AI calculations.
- ***Easier customization:*** Cleaner codebase to implement custom moves and effects.
- ***Improved AI:*** Faster and considers new effects added by Expansion.
- ***Popular features:*** Level/EV Caps, Sleep Clause, Type Indicators.

## Full Trainer customization
- ***Compatible with Pokémon Showdown's team syntax:*** Create your trainer teams in the [teambuilder](https://play.pokemonshowdown.com/teambuilder) and paste the results!
- ***Custom Pokémon data:*** Nicknames, EVs, IVs, Moves, Abilities, Poké Balls, Friendship, Nature, Gender, Shininess, Dynamax level, Gigantamax Factor and Tera Type.
  - ***"Ace Pokémon":*** Will save a specific Pokémon for last.
  - ***Trainer Pools:*** A trainer may get a pool of randomized Pokémon instead of set teams.
- ***Custom sliding trainer messages:*** First Turn, landing a super-effective hit, before Mega Evolution, etc.
- ***New AI Flag options:*** Customize the intelligence of your trainers.
- ***Trainer class Poké Balls:*** Divers use Dive Balls, Breeders use Nest Balls, etc.
## Pokémon data
- ***Improved Pokémon Data structure:*** Optimized space to allow fitting more information, such as Tera type, 12-character names, Hyper-trained stats, evolution conditions, saved HP/status effect.
- ***Updated breeding mechanics:*** Poké Ball/Egg Move/Ability/Nature inheritance, Level 1 eggs automatic incense babies.
- ***Updated species data:*** Stats, Types, Abilities, Hidden Abilities, Egg Groups, EV Yields, movesets, Battle Facility bans, guaranteed perfect IV counts, ORAS Dex numbers.
- ***Simpler species data manipulation:***: Only requires to edit ~5 files instead of vanilla pokeemerald's 20+ to add a new Pokémon.
- ***Updated sprites:*** DS-style sprites with support for Emerald's 2-frame animations and gender difference.
- ***Species toggles:*** You can disable specific groups of Pokémon to save space, including families, cross-gen evolutions, Mega Evolutions, Regional forms, etc.
- ***Revamped Evolution System***: Multiple Evolution conditions can be stacked in order to create complex methods without additional coding. Every condition except Affection and console gyroscope is supported.
- ***Form Change System.*** Most form changes can be added without additional coding. This includes support for: Holding/using an item, HP thresholds being met, weather change in and/or out of battle, Fusions, and more.

## Interface improvements
- ***Pokémon Summary:*** Move relearner, EV/IV checks, Nature colors ([feature branch](https://github.com/DizzyEggg/pokeemerald/tree/nature_color) by @DizzyEggg).
- ***Party Menu:*** "Move Item" option.
- ***Pokémon Storage System:*** Move option as default, access from Box Link item.
- ***HGSS-style Pokédex*** ([original feature branch](https://github.com/TheXaman/pokeemerald/tree/tx_pokedexPlus_hgss) by @TheXaman): Detailed in-game information accessible to players.

## Engine improvements
- ***All base pokeemerald bugfixes implemented by default:*** Anything under the `BUGFIX` define.
- ***Improved sprite and palette compression:*** Assets use less space than vanilla compression.
- ***Modern compiler support:*** Detect potential errors in your code more easily.
- ***Dynamic Multichoice*** ([original branch](https://github.com/SBird1337/pokeemerald/tree/feature/dynmulti) by @SBird1337): Easier way to add multiple-choice menus for scripting.
- ***High-Quality RNG:*** No more broken vanilla RNG.

## Overworld improvements
- ***Modern Mechanics***: Defog field move, B2W2+ Repel system, Running indoors, Removed field poison, Chain fishing, VS. Seeker, FRLG+ whiteout message.
- ***Overworld and Follower Pokémon*** ([feature branch](https://github.com/aarant/pokeemerald/tree/followers-expanded-id) by @aarant)
    - *Includes Dynamic overworld palettes (DOWP) and Overworld Expansion for event IDs beyond 255.*
    - *Includes Pokémon sprites up to Generation IX.*
- ***Day/Night System:*** ([feature branch](https://github.com/aarant/pokeemerald/tree/lighting-expanded-id) by @aarant)
    - *Includes support for non-real time clock*.
- ***NPC Followers***: ([feature branch](https://github.com/ghoulslash/pokeemerald/tree/follow_me) by @ghoulslash)
- ***BW Map Pop-ups*** ([feature branch](https://github.com/ravepossum/pokeemerald/tree/bsbob_map_popups) by @BSBob)
- ***XY Berry Mechanics:*** Mutations, moisture, weeds, pests.
- ***Obtained Item descriptions*** (feature branch by @ghoulslash).

## Developer tools
- ***Integrated Testing:*** Pinpoint if your custom mechanics have broken something else in the game or not.
- ***Pokémon Sprite Visualizer:*** Test every Pokémon sprite and animation.
- ***Overworld debug menu** ([original feature branch](https://github.com/TheXaman/pokeemerald/tree/tx_debug_system) by @TheXaman)*: Support menu with an assortment of features to facilitate debugging, including warping, flag and var toggling, Pokémon and item generation and more.
- ***Battle Debug Menu:*** Modify data on the fly in the middle of a battle.
- ***Learnset Helper:*** Autogenerate movesets from your custom TM and Tutor data based on official compatibility data.
- ***Configurable script flags:*** Disabling Wild encounters, Disabling Trainer battles, Forcing/Disabling Shinies.
- ***Saveblock Cleansing*** ([feature branch](https://github.com/ghoulslash/pokeemerald/tree/saveblock) by @ghoulslash)
