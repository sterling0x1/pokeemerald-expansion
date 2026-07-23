# Milestone 2.5 test checklist

Use normal single wild and trainer battles only for this checklist.

## Core flow

- [ ] Open **Battle** and confirm the party-attacker picker appears.
- [ ] Select each healthy party slot and confirm its own four moves and PP appear.
- [ ] Press **B** in the move list: return to the attacker picker.
- [ ] Press **B** again: return to the command menu without selecting a move.
- [ ] Choose Bag and Run where normally available; confirm they still behave normally.
- [ ] Confirm fainted party members cannot be selected.

## Damage and PP

- [ ] Use a damaging move with the lead Pokémon.
- [ ] Use a damaging move with a reserve attacker; the lead sprite remains visible.
- [ ] Confirm the selected attacker's PP decreases, not the lead's matching move.
- [ ] Use a status move from a reserve attacker.
- [ ] Use a move with a long name and a long description; confirm text stays readable.

## HP and fainting

- [ ] Let the opponent damage the lead after a reserve attack; the lead HP bar must update normally.
- [ ] Use a recoil move from a reserve attacker that survives; confirm the lead HP display is not overwritten.
- [ ] Faint a reserve attacker through recoil or self-damage; confirm it becomes unavailable and the battle continues.
- [ ] Finish the battle after a reserve attacker faints; confirm there is no freeze and the lead remains/restores correctly.
- [ ] After the battle, inspect the party: PP and HP changes for the acting Pokémon should persist as expected.

## UI

- [ ] Command list is four vertical lines with the cursor aligned.
- [ ] Attacker picker names fit their two-column layout.
- [ ] Selected-attacker panel shows level, HP, type, and EXP without clipping.
- [ ] Opponent card shows HP, type, and weaknesses without clipping.
- [ ] Move list and move-info panels keep the Milestone 2.0 font, alignment, and effectiveness marker.

## Out of scope checks

Do not treat doubles, link, Safari, raid, tutorial, Frontier, or other special battles as supported by this milestone. If one routes into normal battle UI, leave the bench-attacker behaviour disabled there.
