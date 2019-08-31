# Changelog for BaseMUD

## Version 0.0.2 (2019-08-31)

- Added the entire world as a sample JSON file (`json/everything.json`).
- Reviewed some immortal commands, re-added `addapply` oedit command, changed `vector` and `bitvector` to `bits` for consistency
- Implementors no longer have to suffer wait/daze lag. Fixed "heat metal" to abort if a save is SUCCESSFUL, not the opposite.
- Vanilla open exits no longer show "-" mark when doors are open. Without `#define VANILLA`, dash marks are still present.
- Removed `Dockerfile` until it's updated properly.
- Refactored most of the "wiz" commands.
- Moved "day of the week" and "months of the year" to a table in `tables.c`.
- Weather and sun position now use a table.
- Weather change calculations have been made easier to understand (but could still be better).
- Modified `stand` command to let you stand at/on/in different objects (or no object) when already standing. Likewise, typing `stand` without an argument while standing at/on/in something will step off/out of them.
- Lots of refactoring to 'find_()' methods. Maybe have been renamed for consistency and clarity.
- Added `find_door_same_room()` so you can type things like `open 2.door`.
