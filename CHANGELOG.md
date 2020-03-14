# Changelog for BaseMUD

## Version 0.1.0 Beta 2 (2020-03-14)

**New Commands:**

- Added `jsave` to `wiz_ml.c`: This saves files in the JSON directory. Enter  
    `jsave` without any arguments for more details.

**New Major Features:**

- Added `json_std/` folder for reference only. This is _not_ used by the game,  
    but is rather for administrators that want to disable  
    `BASEMUD_WRITE_EXTENDED_JSON`. In this case, replace `json/` with this  
    directory.

**Bugfixes:**

- Areas from JSON now update at boot time. Whoops!!
- `had_get_by_name()` and `had_get_by_name_exact()` now reference `name`
    instead of `filename`.
- Refreshed `json/` directory, which added some missing flags.

## Version 0.1.0 Beta 1 (2020-03-13)

Special thanks to contributions from:
- [blakepell](https://github.com/blakepell)
- [rhoxie21](https://github.com/rhoxie21)

### SIGNIFICANT CHANGES:

**New Commands:**
* Added `confiscate` command to `wiz_l7.c`. Thanks, [blakepell](https://github.com/blakepell)!
* Added `portals` command. This displays all "portal exits" (labels attached
    to rooms or exits) or "portal links" (connectors between portal exits)
    for either the area or the world.
* Added `redit_portal()` to modify room portal exits.
* Added "portal" command to `redit_change_exit()` to modify exit portal exits.

**New Major Features:**
* Introduced "extended flags", inspired by the more-than-32-bit flag implementation from
    SMAUG. The following macros are available (most correspond to functions): 
    `EXT_ZERO` (Represents "no flags"), `EXT_IS_ZERO()`, `EXT_IS_NONZERO()`, `EXT_BITS()`, 
    `EXT_IS_SET()`, `EXT_EQUALS()`, `EXT_SET()`, `EXT_SET_MANY()`, `EXT_UNSET()`, `EXT_UNSET_MANY()`, 
    `EXT_TOGGLE()`, `EXT_TOGGLE_MANY()`, `EXT_TO_FLAG_T()`, `EXT_INIT_ZERO`, `EXIT_INIT_BITS()`,
    `EXT_FROM_FLAG_T()`, `EXT_FROM_INIT()`, `EXT_WITH()`, `EXT_WITHOUT()`, `EXT_WITH_MANY()`,
    `EXT_WITHOUT_MANY()`, `EXT_INVERTED()`
* Added `races.json` to `json/config`. This replaces the table of races.
* Added `pc_races.json` to `json/config`. This replaces the table of playable races.
* Added `songs.json` to `json/config`. This replaces `songs.txt`.
* Added JSON files for all other in-game tables that could potentially be loaded dynamically.
    They were written at run-time to the `json/unsupported` directory.
* Added "enhanced" JSON format that allows for human-readable line breaks. Because this is
    non-standard, it can be disabled in `basemud.h`.
* Removed all remaining hungarian notation (YAY)

**New Files:**
* Introduced `items.c`, which contains (nearly) all item-type-specific logic. There are many, many
    new helper functions here that dictate item behavior, most of it migrated from `do_*()` functions.
    This is a staging step for migrating objects to a more object-oriented model with polymorphism.
* Introduced `fread.c` and `fwrite.c` for reading/writing basic data to files. Migrated related
    routines from `olc*`, `db.c` and `save.c`.
* Moved `db_export_json()` to new file, `json_export.c` and did a lot of
    refactoring. Most of the nasty macros are gone - yay!
* Introduced `players.c` and `mobiles.c` to separate player-only and mobile-only methods
    that correspond to CHAR_Ts.
* Extracted many area functions to new file `areas.c`.
* Extracted many reset functions to new file `resets.c`.
* Extracted many extra_descr functions to new file `extra_descrs.c`.
* Extracted some help functions to new file `help.c`.
* Moved `qmconfig_read()` to new file `quickmud.c`.

**(Behavioral) Changes:**
* Modified JSON folder structure a bit - `config` is now loaded first, then `areas` then `help`.
    Flags, types, and (new) extended flags are stored in the `meta` directory. New tables that are
    not (yet) read in `config` are in the `unsupported` directory.
* `wizhelp` now shows the minimum level required for each wiz command.
* Replaced all "Ok." messages with more descriptive messages to the player.
* Pressing Control-C now gracefully triggers a shutdown. Hitting Control-C again will forcefully quit.
* Made several properties in JSON objects optional.
* Removed redundant newlines from JSON strings.
* `abilities` and `spells` commands now shows both mana _and_ percentage learned for spells.

**Major Internal Changes:**
* Completely overhauled the ROM skill group system. "Groups" have been renamed to "skill groups"
    for consistency.
* Replaced _all_ linked lists with double listed lists.
* Added links to parent entities to all child entity structures, including
    entities that can be linked to multiple parents like objects, extra_descrs,
    and affects. This is mostly so entities can detach themselves to their
    parents easilly upon disposal.
* Replaced most linked list modification to their own functions so lists are
    more reliably maintained between various methods of entity loading/unloading.
* Removed `gsn_*` for global skills. Please use `SN(SKILL)` instead (ex: `SN(FRENZY)`).
* Lots and lots and LOTS of rewrites to portals to make them work better with
    OLC. Many dumb things were replaced, many new features were added, and they
    are (at least comparitively) much easier to work with.

### SMALLER CHANGES:

**New Minor Features:**
* Modifying exits via OLC now automatically manages portal exits and portal
    links. Portal exits/links deleted or created are logged to the player.
* Added `min_pos` property to socials in `json/config/socials.json`. This is mainly for the
    `snore` social, which was hard-coded to be allowed during sleeping.
* Replaced player flags and mob flags with new extended flags.
* Added dotted lines in `practice` and `skills` commands between skills and their mana/percent values
    for better readability. This can be disabled via `basemud.h`.
* Bit flags can now be written and read using the new extended flag bracket
    notation (ex, `[flag1 flag2 flag3]`). This is on by default but can be
    disabled in `basemud.h` for backwards compatibility.
* All entities (objects, obj_indexes, rooms, areas, etc) can now be unloaded
    safely from memory without corrupting the world state (AFAIK). This is to help ensure
    that the MUD was running in a stable state when shutting down.
* JSON reading/writing now has better logging with proper file and location.
* Portal exits are now written to `.are` files using the `P` command for rooms
    and resets (following exits). This is on by default but can be
    disabled in `basemud.h` for backwards compatibility.
* Created `ban_flags[]` and `wiz_flags[]`. These are used for reading/writing
    flags in the ban list and player saves.
* Added single character `n/e/s/w/u/d` commands to the top of the OLC redit
    so we don't have to type all the exits out :)
* Added new macros `LIST2_REASSIGN_FRONT` and `LIST2_REASSIGN_BACK` to easily
    manage entities and their parents.
* Added `str_inject_args()` in `utils.c` to inject string arguments into a format string in any location.
* Added global variable `in_game_loop` so disposal methods can report bugs
    under more specific conditions.
* Doubled maximum page buffer size. In 2020, I think a 32k buffer is an
    acceptable memory hit ;)
* Completely overhauled how jukeboxes / global playlists work. This is probably the least-requested
    thing in the universe, but it was a hot mess :)

**Debugging Features:**
* Added `#define BASEMUD_LOG_EQUIP_WARNINGS` in `basemud.h` to show verbose warnings for badly-placed equipment
    during boot. Off by default.
* Added `#define BASEMUD_LOG_KEY_WARNINGS` in `basemud.h` to show verbose warnings for missing or
    bad keys during boot. Off by default.
* Added `#define BASEMUD_LOG_EXIT_WARNINGS` in `basemud.h` to show verbose warnings for incorrect or
    otherwise bad exits during boot. Off by default.
* Added `#define BASEMUD_LOG_FILES_LOADED` in `basemud.h` to show verbose messages for files loaded
    (or ignored) during boot. On by default.
* Added `#define BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT` in `basemud.h` to completely circumvent
    the memory caching. This is so memory usage can be properly tracked and memory
    leaks / errors can be fixed. Off by default.
* Added `#define BASEMUD_DEBUG_DISABLE_RECYCLE` in `basemud.h` to disable all
    entity memory recycling and free them on demand. This is so memory usage
    can be properly tracked and memory leaks / errors can be fixed. Off by
    default.
* Updated `db_dump_world()` to dump all of the latest changes.
* Updated `db_dump_world()` to use non-randomized exits when dumping rooms.
    This should make things much easier when comparing `world.dump` files
    with another to check for inconsistencies when mucking with the db code.

**Bugfixes:**
* JSON objects/tables are no longer imported if property requirements are not met.
* Fixed newline consistency in stock world.
* Replaced tab characters in help files with spaces.
* Messages for socials performed while sleeping (like `snore`) are now always visible to
    the player.
* Introduced `ctime_fixed()` to remove trailing newline in `ctime()`.
* It was possible to permanently remove skills by adding and dropping certain skill groups.
    This has been fixed.
* Fixed several memory leaks that were hidden by ROM's memory caching system.
* Fixed inconsistencies in loading behavior between traditional `#AREA`
    sections and OLC `#AREADATA` sections in `.are` files.
* Added missing "materials" flag to `comm_flags[]`. Whoops!

**Structural / Relational Changes:**
* Clean-ups to room exit management code. There should be much less
    mucking-around with `EXIT_T` values internally.
* Refactored `resets` command to use updated reset creation/linking code.
* Updated all `*_dispose()` methods to unlink child entities and unlink from
    parents.
* `char_from_room()` and `obj_take_from_*()` methods are now called
    automatically when performing `char_to_room()` and `obj_to_*()`. There
    is barely any performance increase to calling these methods manually, but
    there is _significantly_ less room for error this way. Also it's very
    convenient :)
* Modified `affect` and `extra_descr` structures to specify what type of
    parent entities they're attached to.
* Reviewed and cleaned-up object/mobile count management.
* Attached all entities associated with "areas" to linked lists directly
    in the area structure.
* Updated and refactored string hashing to use more specific hashes rather than 
    string length. It is now _much_ faster at the cost of a little more memory.

**Minor Internal Changes:**
* Classes no longer have any hard-coded behaviors. All class behavior can be determined by properties
    in the class table, and could theoretically be loaded during run-time via JSON.
* Removed hard-coded races because they're not loaded by `config/races.json`.
* Checks for the donation pit are now with `obj_is_donation_pit()` instead of
    checking the vnum manually.
* Replaced many `fprintf()` calls in `json_write()` to use `fputc()`, `fputs()`,
    or `fwrite()`. This should speed up exporting significantly.
* Overhauled the "pose" command and the "pose" table.
* There is now a separate table for skill mapping called
    `skill_map_table`, with hard-coded skill indexes and names of skills. These indexes are
    mapped to the real skill table during runtime.
* Consolidated a lot of routines involving skill groups.
* Nanny functions, init functions, and disposal functions now use `DEFINE_XXX_FUN()` for their
    definitions.
* Moved many position messages to a new table in `tables.c` called `position_table[].
* Replaced `NO_FLAG` with `FLAG_NONE`, `TYPE_NONE`, and `EXT_FLAG_NONE`. Values are now 0, -999, and
    -999 respectively.
* Merged some behavior for showing victim positions to players.
* Cleaned up some of `effects.c` as a result of migrating lots of behaviors to `items.c`.
* `make_corpse()` now returns the corpse it made so `autoloot` is guaranteed to find the correct
    corpse.
* Migrated HP condition messages (excellent, few scratches, etc) to a table called `condition_table[]` in
    `tables.c`.
* Removed `DAM_PHYSICAL` - this is now the default type, and `DAM_MAGICAL` is a flag rather than a type.
* Removed `wear_loc_types[]` and `wear_loc_phrases[]` - they're now pear of the `wear_loc_table[]`.
* Added internal tables for door reset types (`door_reset_types[]`), stats (`stat_types[]`), condition
    types (`cond_types[]`), and skill target types (`skill_target_types[]`). They are used when exporting
    (and eventually importing) other tables to JSON.
* Moved timed conditions (hunger, thirst, fullness, drunkness) to a table with
    good/bad check functions and messages. Message checks were moved to
    `player_change_condition()`.
* Completely rewrote `area_update()` to clean up the logic for when areas
    should update. The MUD school has some pretty stupid reset rules...
* `load_rooms()` uses a switch() statement for resets now instead of a big
    chain of if/else statements.
* Lots and lots of rewrites to `redit_change_exit()` to make it work with newer
    code.
* Modified "room", "exit", "portal exit", and "portal link" disposal to properly
    detatch themselves from one another.

**Refactoring / Rearranging:**
* Clarified usage between `*_extract()` and `*_free()` functions. "Extract"
    functions are used for removing entities in the game world (extra removal
    logic is required), and "free" functions are for unloading from memory
    (these should be used for entities _not yet_ in-game).
* Defined `*_get()` and `*_get_exact()` function behaviors more consistently.
* Moved several functions (but not enough!) from `db.c` to more appropriate
    places and renamed them accordingly.
* Replaced `raw_kill()` (`fight.c`) with `char_die()` (`chars.c`),
    which then calls either `player_die()` (`players.c`) or
    `mobile_die()` (`mobiles.c`).
* Moved several functions in `update.c` to their appropriate entities
    (i.e, `char_update()` is now in `chars.c`).
* Split several `*_update()` functions into `*_update_all()`, which updates
    all, and `*_update()` which updates a single entity.
* Extracted mob wandering into `mobile_wander()`.
* Moved buffer management functions from `recycle.c` to `memory.c`.
* `TYPE_UNDEFINED` has been replaced with `ATTACK_DEFAULT` for clarity.
* Replaced `redit_add_reset()` with `reset_to_room_before()`.
* Some code formatting clean-ups in OLC commands.
* Separated room exit saving code in OLC from `save_room()` into smaller
    `save_exit()`.
* Rearranged `json*.c` for consistency. `json_import()` is now just for general functions. Object/table
    reading/writing functions are now in `json_objr.c`, `json_objw.c`, `json_tlbr.c`, and `json_tblw.c`.

**Renamed Functions / Variables:**
* Renamed `obj_get_by_index()` to `obj_get_last_by_index()` to better express
    what the method does.
* Renamed `affect_to_char()` to `affect_copy_to_char()` to better describe
    its usage.
* Renamed several affect functions to better describe their behavior with
    other entities (i.e, `affect_strip()` -> `affect_strip_char()`).
* Renamed global/structure link names for consistency.
* Renamed board functions to match more consistent naming schemes.
* Renamed some global variables for naming consistency.
* Renamed `capitalize()` to `str_capitalized()` to better indicate its behavior.
* Renamed functions on `ban.c` to match naming schemes.
* Renamed `is_full_name()` to `str_in_namelist_exact()` to better indicate what this function does.
* Renamed other string utility functions to better indicate their behavior.
* Renamed `door_*()` in `act_move.c` to `do_door_*()` for sub-routine naming consistency.
* Renamed `WEAR_*` to `WEAR_LOC_*` to avoid confusion.
* Renamed skill targets from `TAR_*` to `SKILL_TARGET_*` for clarity's sake.
* Renamed `pcdata.conditions` to `pcdata.cond_hours` to better express its
    function.
* Renamed `json_write_to_file()` to `json_fwrite()` for consistency (even
    though it's not quite the same thing... HMM)
* Renamed `exit_data` fields to better describe what they do.
* Changed `COLOUR_MAX` to `COLOUR_SETTING_MAX` for clarity. Introduced new `COLOUR_MAX` for
    colour codes.
* Replaced `A, B, C...` #defines with `BIT_01`, `BIT_02`, etc.
* Replaced hard-coded magic attack numbers for slash/pound/pierce/punch with ATTACK_SLASH, ATTACK_POUND,
    etc. They're still hard-coded (which is bad), but at least they're no longer magical.

**Other changes:**
* Cleaned-up inconsistent spacing in notices at the top of `.c` and `.h` files. The content
    remains exactly the same - who knows why the spacing got off. Some messages that should have
    been present have been added (like in comm.h).

## Version 0.0.5 (2019-12-06)

**Project Organization:**
* All "TO-DO" notes have been transferred to issues in the GitHub repository
    and given appropriate labels.

**New Features:**
* Area files can now be loaded in any order.
* JSON files can now be imported.
* JSON importing is an option defined by `BASEMUD_IMPORT_JSON` in basemud.h
* Modified the `dump` command to allow `dump stats` or
    `dump world <raw | json>`. Files are dumped to the `dump/` directory.
* Areas already loaded by JSON importing are now ignored.
* Warnings are now produced if non-doors have keys.
* Warnings are now produced for keys that don't exist.
* Added db_dump_world() to dump _every_ field of every loaded structure for change comparisons.
* During the JSON export process, several properties with their default values are now ignored.
* The "poisoned" flag from fountains is now applied to containers when filling.

**Bug Fixes:**
* Fixed "You see no door <dir> here" broken message.
* The `AREA_LOADING` flag is no removed for the last area loaded.
* Duplicate socials are now removed.
* Added `db_finalize_mob()` to produce final flags based on race.
* Fixed several missing, improperly handled, or unnecessary fields during JSON export process.
* Replaced tabs in help files with spaces.

**Other Changes:**
* JSON is no longer automatically exported.
* Non-doors now always have their key set to KEY_NOKEYHOLE.
* Added nicer error messages in fix_exits().
* Changed `#ifndef VANILLA` checks to `#ifdef BASEMUD_(FEATURE)`.
* The default material is now `MATERIAL_GENERIC` instead of `0`.
* The "poisoned" flag for food and drink is now set to `TRUE` instead of 1.

**Internal Changes:**
* Moved all shared global variables to new `globals.c` and `globals.h`. All
    other global variables used in a single file are now marked `static`.
* Moved all string or memory allocation functions from `utils.c` to new
    `memory.c`. Functions in `memory.c` have been renamed more appropriately,
    reviewed, and wiped clean of hungarian notation (yay).
* Moved all typedefs to new `typedefs.h`. This way, structures can be used in
    all headers before they're defined.
* Removed some unused global variables.
* Removed `target_name` and `damage_adj` global variables - they are now
    explicitly provided to the functions that require them.
* All structure typedefs (except for 'flags' and 'types') are now named with
    `_T` for consistency. For example:
    `CHAR_DATA` is not `CHAR_T` and `DOOR_TYPE` is now `DOOR_T`.
* Replaced key 0 and -1 with KEY_NO_KEY and KEY_NOKEYHOLE wherever it was seen.
* Replaced flags for mobiles that are overridden by flags - now uses "_plus", "_minus", and "_final" flags when building final flags.
* Removed all "_str" fields except for "area_str".
* Separated room/mob/obj hash registration process into separate functions.
* Removed "opposite" pointer in portals; added "generated" flag to account for portals that were created while generating missing portals.
* Renamed basemud version from VERSION to BASEMUD_VERSION.
* assign_area_vnum() now takes an explicit area instead of using area_last.
* Renamed fix_bogus_obj() to db_finalize_obj().
* Added ANUM_TYPE for linking area+anum combinations to objects during JSON import process.
* Fixed json_get() to allow for const inputs.
* Added json_expand_newlines() for replacing "\n" with "\n\r" during JSON import process.
* Added json_value_as_*() functions to quickly get JSON values as C types.
* Added json_import_obj_*() functions to convert JSON objects to world objects.
* flag_value() now uses flag_value_real(), which can specify a "no flag" return value.
* Added various lookup functions required by JSON import process.
* Grouped bit macros together in "macros.h".
* Removed DIF() macro from OLC and replaced it with MISSING_BITS() in "macros.h".
* Modified OLC to account for internal changes (needs testing!!)
* Added several default object values to *_init() functions in "recycle.c".
* Added default values for object_index value mapping used during JSON object export/import processes.
* Added unused item types to item_table[] (which can probably be removed anyway).

## Version 0.0.4 (2019-10-29)

**Project Organization:**

* This project now has a rough roadmap for version 0.1.0 up to 1.0.0 in `BENCHMARKS.md`.
* All TODO messages have been moved to `src/TODO.txt` for staging into
    Github issues.

**Bug Fixes:**

* Reset `'E'` and `'P'` "local limit" flag is unused or was misinterpreted. It's
    no longer saved in JSON files.
* For some reason, skills were always evaluated with a chance of 100%. This has
    been fixed.

**Internal Changes:**

* Object values now use a union with all property types. Nearly all references
    to `obj->value[x]` have been replaced by `obj->v.<type>.<property>`. 
    Example: `obj->value[0]` for weapons is now `obj->v.weapon.weapon_type`.
* Reset values now use a union with all property types. Nearly all references
    to `reset->value[x]` has been replaced by `reset->v.<type>.<property>`. 
    Example: `reset->value[3]` for the `'E'` (equip) command is now `reset->v.equip.slot`.
* Replaced most character and object macros with small functions. Macros have
    been left in place as shorthand functions.
* Moved all top-level booting functions to `boot.c`.
* Replaced "sprintf(), send_to_char()" with "printf_to_char()" wherever possible
* Replaced "sprintf(), log_string()" with "log_f()" wherever possible
* Replaced "sprintf(), bug()" with "bugf()" wherever possible
* Replaced "sprintf(), wiznet()" with "wiznetf()" wherever possible
* Added RETURN_IF_BUG() family of macros to reduce code for common bug early
    exit patterns
* The "TO_XXX" affect flags have been renamed to "AFF_TO_XXX" to avoid conflict
    with act() flags (TO_CHAR, TO_NOTCHAR, etc)
* Clarified wear _flags_ (head, finger, wrist) vs. wear _locations_ (head, lfinger,
    rfinger, lwrist, rwrist). Most code now uses `wear_flag` and `wear_loc` to
    distinguish the two. A few random bugs (likely by me) from this confusion
    have been patched.
* Some error messages in OLC were in Spanish - all messages are now in English.
* All `do_*()` functions are now defined using `DEFINE_DO_FUNC(do_*)`. This way,
    if the arguments to a top-level command are changed, every function doesn't
    need to be modified.
* Moved mob/obj instantiation and clone functions to `chars.c` and `objs.c`. They
    are now named `char_create_mobile()`, `char_clone_mobile()`, `obj_create()`
    and `obj_clone()`.
* Replaced <attr>_app[] lookups with `<attr>_app_get()`, `char_get_curr_<attr>()`
    functions.  Added lookup functions for all stat bonuses.

**New Features:**

* Clarified some error messages caused by `fix_exits()` and `reset_room_reset()`.
* Added warnings for `'E'` resets with unequippable wear locations for the item
  or wear locations that are already taken on the mob. Many of these errors are
  acceptable with the stock world, but many indicate real bugs.

**New Features (if `VANILLA` is disabled):**

* Added OLC editor and vnum to the front of the prompt by default.

## Version 0.0.3 (2019-10-03)

**Project Organization:**

* Reorganized 'Internal Changes' section in README.md to:
    - Deployment
    - Code Reorganization
    - Code Reduction
    - Code Changes
* Reorganized 'Gameplay Changes' section (now 'Game Changes') in README.md to:
    - Bugfixes
    - Patched oversights
    - Quality-of-Life features
    - Gameplay Changes
    - Optional Features
* Many of the "TO-DO's" have been moved to a Trello board for better tracking.

**Bugfixes:**

* When wearing an item, _all_ potential slots are now checked, not just related
  slots (e.g, a ring can now be held as well as worn on a finger).
* In most cases, finding characters and objects by '1.thing', '2.thing', etc.
  will now count correctly when searching multiple locations (e.g, check the room,
  then the rest of the world). This is done by a `find_continue_count()` call
  preceding the next check.
* Checks for immune/resistant/vulnerable flags completely ignored default
  weapon/magic imm/res/vuln flags, despite existing code with implied behavior.
  Resistance no longer overrides default immunity, and vulnerability now
  properly _downgrades_ the default status (imm`->`res, res`->`normal,
  normal`->`vuln).

**Internal Changes:**

* Separated mob- and player-specific flags (`ACT_*`) into separate `MOB_*` and `PLR_*`
  flags, with separate variables. This should prevent a _lot_ of bugs.
* The `wear` command now uses a table for looking up wear slots, as does `char_wear_obj()`.
  This is a big improvement from copy+pasted "You wear ___ on ___" code for each possible slot.
* Weather change calculations have been made easier to understand (but could still be better).
* Replaced `is_safe()` and `is_safe_spell()` with consolidated
    `do_filter_can_attack_real()`. Added shorter functions:
    `do_filter_can_attack()`, `do_filter_can_attack_spell()`,
    `can_attack()`, and `can_attack_spell()`.
* Merged `db2.c` into `db.c` and separated old database functions into `db_old.c`.
* Merged code for displaying exits in `look`, `exits`, and exits in the prompt.
* Introduced `dam_type` classes `DAM_PHYSICAL` and `DAM_MAGICAL` for determining
  immunity.
* Separated descriptor functions in `comm.c` into new file `descs.c`.
* Moved some game loop functions from `comm.c` to `main.c`.
* Several miscellaneous clean-ups for redundant or deprecated code.

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
