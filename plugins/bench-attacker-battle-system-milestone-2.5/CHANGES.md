# Milestone 2.5 change summary

## Battle system prototype

1. Added a per-battler selected-party-slot record and saved lead-battler state.
2. Added a restricted battle-state path that asks the player to choose an attacker before choosing a move.
3. Read the selected party member's moves and PP in the move picker.
4. Temporarily apply that party member's battle data for its action, with abilities disabled for this prototype.
5. Restore the normal lead data once the action is complete.
6. Added a dedicated off-field faint path. If the reserve attacker reaches zero HP, it is written as fainted in the party, removed from future selection, and does not run the normal active-Pokémon faint sequence.
7. Suppressed reserve-attacker health-bar animations so their recoil/self-damage cannot visually replace the lead Pokémon's HP.

## UI work added since Milestone 2.0

1. Reworked the battle-command screen into a vertical four-command list.
2. Added a compact party-attacker selector and selected-attacker details panel.
3. Added an opponent-information card: `Enemy Info`, HP, type, and weaknesses.
4. Updated navigation so move list → B → attacker picker → B → command menu.
5. Kept the Milestone 2.0 compact font and move-info panels as the visual base.

## Recent fixes verified during the prototype

- Selecting a different attacker no longer silently executes a move.
- Returning from the picker no longer produces a blank move panel.
- A reserve attacker that survives recoil does not replace the lead HP display.
- A reserve attacker that faints is no longer treated as the on-field Pokémon, preventing the previous battle-end freeze.

## Known limits

- The off-field attacker's damage is intentionally not animated in the lead health box.
- Special battle formats are not enabled.
- The interface is designed for the current compact Milestone 2.0 font/layout and may need a merge pass in a differently customised UI.
