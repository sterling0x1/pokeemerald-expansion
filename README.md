# Pokémon Emerald Expansion Fork — Milestone 2.7

This repository is a customised `pokeemerald-expansion` fork containing the project’s accumulated gameplay, battle, interface and quality-of-life patches up to **Milestone 2.7**.

Milestone 2.7 is a stable development checkpoint, **not a finished 3.0 release**. The current build contains substantial working functionality, but the full options system and battle engine still require broad regression testing and further bug fixing before the core can be considered complete.

## Included Systems and Patches

The current project state includes:

- Previous project patches and battle-controller fixes
- Extended Options Menu work
- Configurable and corrected shiny-rate settings
- Instant Text
- Fast HP and Fast EXP bar runtime work
- Gameplay randomisers
- Modern and compact Battle UI work
- Bench Attacker System
- Held-A Quick Ball input fix
- Reserve/bench target-selection fix
- Stale battle-cursor cleanup
- Attacker-picker cleanup
- Additional quality-of-life and engine fixes accumulated across earlier milestones

Some included systems remain under audit. Inclusion in this branch does not mean every setting, mode or edge case has completed release-level testing.

# Milestone 2.7 — Bench Attacker Runtime Fixes

Milestone 2.7 rebuilds the Bench Attacker runtime fixes cleanly from `master` and resolves three major state-ownership problems:

1. PP ownership and zero-PP validation
2. Obedience metadata
3. Persistent state for multi-turn and recharge moves

The milestone checkpoint is:

- Branch: `milestone2.7`
- Tag: `v2.7-bench-attacker`
- Commit: `2946587cf1`
- Tag message: `Milestone 2.7: Bench Attacker System runtime fixes`

## Bench Attacker PP Fix

### Previous behaviour

The move menu could display a bench Pokémon’s moves and PP correctly while move-selection validation still read PP from the active battler.

This caused incorrect behaviour such as:

- False **No PP** messages
- The wrong move slot being rejected
- PP validation changing after the active Pokémon fainted or switched
- Risk of the active battler’s PP being treated as the attacking Pokémon’s PP

### Current fix

Bench-attacker selection is now handled as a distinct ownership context.

During bench selection, move IDs and current PP are read from the selected party Pokémon rather than from the active `gBattleMons` entry. Normal active-battler selection continues to use the standard battle state.

The result is:

- Bench attackers use their own PP.
- Zero-PP validation checks the correct move slot.
- Other moves remain selectable when only one slot has zero PP.
- The active Pokémon’s PP remains unchanged.
- PP is deducted from the bench attacker when its move executes.

## Obedience Fix

### Previous behaviour

A bench attacker could incorrectly disobey because the temporary `BattlePokemon` created by `PokemonToBattleMon()` contained stale or incomplete met-level data.

That made the configured Gen 7 obedience logic evaluate the wrong metadata.

### Current fix

The bench attacker’s current party met level is explicitly restored:

```c
freshBattleMon.metLevel =
    GetMonData(reserveMon, MON_DATA_MET_LEVEL);
```

This prevents false obedience failures caused by stale runtime data.

Focused testing confirms the original false-disobedience behaviour is fixed. Traded-Pokémon badge and level boundaries still require a complete regression matrix.

## Multi-turn and Recharge Move Fixes

### Previous behaviour

Bench attackers were rebuilt as temporary battlers for each action. Their battle-only runtime state was lost when the active battler was restored.

This broke mechanics that depend on state continuing across turns, including:

- Rollout
- Ice Ball
- Recharge turns
- Locked-move continuation

### Current fix

Persistent runtime storage is maintained per party slot.

The stored state includes:

- `BattlePokemon` runtime data
- Moves and PP
- HP and maximum HP
- Primary status
- Level and met level
- Locked move
- Runtime validity

When the same bench Pokémon acts again, its runtime snapshot and locked move are restored before action selection. The state is saved again before returning control to the active battler.

This allows:

- Rollout to continue across turns.
- Ice Ball to use the same persistent runtime path.
- Recharge moves to continue with the correct user.
- Locked moves to be selected automatically where required.
- Different bench Pokémon to have separate runtime storage by party slot.

Core Rollout and recharge continuation are working. The complete interruption and lifecycle matrix is not yet finished.

Only Rollout has been tested so far.

## Files Changed for the Milestone 2.7 Runtime Fix

```text
include/battle.h
src/battle_main.c
src/battle_util.c
```

The clean rebuild intentionally avoided importing unrelated changes from the earlier working prototype.

# Options and Quality-of-Life State

## Confirmed Working

The following options or runtime paths have confirmed fixes or focused runtime evidence:

- Instant Text
- Configurable shiny rates
- Fast EXP bar runtime behaviour
- Bench Attacker PP ownership
- Bench Attacker obedience metadata
- Bench Attacker Rollout and recharge continuation

These items must remain in regression coverage even though their original reported bugs are fixed.

## Implemented but Not Fully Verified

The following areas are present or partially wired but still require complete validation:

- Extended Options Menu pages and sections
- Fast HP bars
- Fast EXP storage, defaults and persistence
- Battle Text option
- Quick/Fast Intro option
- Packed extended-option fields
- Save and reload behaviour for every option
- New-game and clear-save defaults
- Existing-save fallback and migration behaviour
- Menu scrolling, cursor bounds, wrapping and callbacks
- Link, Battle Frontier, scripted and other special battle contexts

## Incomplete Runtime Work

Battle Speed does not yet have a proven, complete canonical runtime path across all relevant battle timing and controller consumers.

It must not be considered complete merely because a setting can be displayed or stored.

# Required Stable-Core Validation

The codebase is not ready to be treated as a completed 3.0 core until the following work is closed.

## Complete Options Menu Audit

Every current option needs:

- Valid, non-overlapping storage
- A documented default
- Correct new-game and clear-save initialisation
- One canonical getter
- Verified runtime consumers
- Save and reload persistence
- Safe behaviour when loading older or invalid values
- Correct enabled/disabled polarity
- Correct behaviour in special modes
- Verified menu labels and displayed values
- Safe navigation, scrolling, wrapping and cursor bounds
- Reliable cancel, save, cleanup and return-callback behaviour
- Confirmation that changing one packed option does not modify another

Vanilla options must also be retested after the extended-menu changes:

- Text Speed
- Battle Scene
- Battle Style
- Sound
- Button Mode
- Frame Type

## Complete Multi-turn Move Testing

Rollout, Ice Ball and recharge mechanics need exhaustive testing for:

- Successful continuation  - tested 
- Misses and failed moves
- Sleep
- Paralysis
- Confusion
- Flinch
- Disable
- Encore
- Taunt
- Torment
- Choice locking
- Struggle
- Fainting
- Manual switching
- Forced switching
- Target becoming unavailable
- Cancelling the attacker picker
- Cancelling the move menu
- Different bench attackers acting in sequence
- The active Pokémon fainting before selection
- Turn transitions
- Battle teardown
- Starting a new battle after a previous bench runtime existed

Each scenario must confirm both the expected move behaviour and correct cleanup of runtime state.

## Complete Battle-System Regression Testing

The Bench Attacker System and previous battle patches must be tested across:

- Wild battles - tested 
- Trainer battles - tested
- Singles - tested
- Doubles - tested
- Target selection - tested
- Partner and opponent selection - tested
- Fainting - tested
- Switching - tested
- Cancelling - tested
- Forced actions
- Special move selectors
- AI-controlled battlers
- Scripted battles
- Battle facilities
- Held-A Quick Ball behaviour - tested
- Stale cursor cleanup
- Attacker-picker cleanup
- Normal battles where the Bench Attacker System is not used

Special attention is required for ownership of:

- Battler indexes
- Party indexes
- Move and PP data
- Locked moves
- Temporary selection flags
- Controller buffers
- Runtime validity flags

No selection flag, index, locked move or runtime snapshot may leak into the next battler, turn or battle.

## Save and State Validation

Testing must confirm:

- Extended settings persist after saving and restarting.
- All settings receive valid defaults.
- Packed fields do not overlap or truncate.
- Invalid values fall back safely.
- Bench-attacker runtime data is battle-local and never saved accidentally.
- Runtime state is cleared at battle teardown.
- A new battle begins with no stale bench-attacker state.
- Existing saves remain usable after current changes.

## Build and Release Hygiene

Before the stable core is considered complete:

- Perform a clean build from a fresh checkout.
- Record the supported toolchain and build steps.
- Review compiler warnings.
- Verify the exact release commit.
- Test a clean save and representative existing saves.
- Remove debug-only code and temporary investigation changes.
- Keep local audit dumps and backup copies out of release commits.
- Review the final diff for unrelated changes.
- Confirm documentation matches the actual source and tested behaviour.

# Current Status

Milestone 2.7 fixes the known Bench Attacker PP ownership defect, restores correct obedience metadata and introduces persistent per-party-slot runtime state for multi-turn and recharge moves.

It is an important stable checkpoint, but substantial validation remains:

- The Options Menu is not yet fully verified.
- Battle Speed remains incomplete.
- Battle Text and Quick/Fast Intro require current re-verification.
- Fast HP and Fast EXP need complete storage, persistence and edge-case coverage.
- Multi-turn moves need exhaustive interruption and cleanup testing.
- The complete battle system needs a broad regression pass.
- Further bugs are expected to be found during those tests.

The stable-core goal is reached only when the existing systems work consistently together, survive save and battle lifecycle transitions, and pass repeatable release-level testing.
