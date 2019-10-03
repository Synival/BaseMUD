BaseMUD
==============
**A thorough code review/overhaul of QuickMUD / ROM 2.4b6 for starting your MUD**

BaseMUD is a work-in-progress fork of QuickMUD that has been heavily
reorganized, refactored, and cleaned-up for clarity and easy extendability. This
project has two main goals:

1. Provide an easy-to-modify codebase for those who want to play around with
  their own stock ROM mud, and
2. Provide a transitional project for porting the ROM codebase to C++ or another
  high-level language.

## Instructions

To compile and run:

```
cd src
make
cd ..
./run.sh
```

To play:

```
telnet localhost 4000
```

BaseMUD comes packaged with a level 60 "Implementor" account.
The name is `Admin` and the password is `Admin`. *Change the password* and,
if you'd like, create your own god character and raise them to level 60.

## Legal Stuff

BaseMUD is based on [QuickMUD](https://github.com/avinson/rom24-quickmud),
which is based on the ROM 2.4 beta version of Merc 2.1 base code, which is in
turn based on DikuMUD. Credits in licenses:

```
ROM 2.4 copyright (c) 1993-1995 Russ Taylor.
MERC 2.1 code by Hatchet, Furey, and Kahn.
DikuMUD by Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert, and Sebastian Hammer
```

If you wish to use BaseMUD, you must comply with the following licenses:

* `doc/license.doc`: DikuMUD license
* `doc/license.txt`: Merc Release 2.1 license
* `doc/rom.license`: ROM 2.4 license

As with QuickMUD, BaseMUD is packaged with the following code (which has been
integrated into the source code more thoroughly):

* OLC 1.81
* Lope's Color 2.0
* Erwin's Copyover
* Erwin's Noteboard
* Color Login

## About this project

This project is still very rough. I've been testing things as I go, but
I've been haphazardly moving large swaths of code around under the premise that
everything will be thoroughly tested later. Hopefully the resulting code will
be modular enough that some sort of unit testing environment will be possible.

Despite the heavy refactoring,
BaseMUD strives to be a pure version of ROM 2.4. However, in the process of sifting
through all this code, sometimes I just can't help myself.
Some small changes and additions have been made, which can be enabled by
commenting out the line `#define VANILLA` in `src/basemud.h`. Changes that
I've considered bugfixes, typo / spelling error fixes, or patches to small
oversights (i.e, fixing spells that have no casting message) will be left in.

Most of the code has been manually linted for better readability and brevity.
Some changes are subject to taste, but much care has been spent to make the
code consistent and easy to read. Here are some general style changes:

* Vertical space is prioritized highly - two-line if() statements are bountiful.
* Horizontal space is also cherished. Some lines exceed the old standard 80
  character limit _just barely_ to avoid some ugliness.
* Several giant tables, such as spells, races, and classes, go WAY over the line so
  they can be read and modified as tabular data.
* Redundant comments have been removed for the most part in favor of renaming
  functions, parameters, and variables to be more self-evident.
  The author comments have been preserved out of respect and to comply with licenses.
* _All_ tabs have been removed and replaced with spaces. Everywhere.

Some changes may also be _temporarilly regressive_ as a staging step for some
larger refactoring.  In other words, some things are a mess because I'm still cleaning it :)
The affect code, for example, had all of its easy-to-read assignments replaced with
incomprehensible one-liner functions. The long-term plan, in this case, is to move all of the
affects to a table, at which point this will be fixed.

Note: **OLC and Copyover code is also largely untested, and may even crash! D:**

## Internal Changes

**Deployment:**

* The `rom` executable is now in the `/bin` directory and run from the _root_
  directory. For convenience, you can use `run.sh` in the root directory.
* The `Dockerfile` has been temporarilly removed until some of the structural
  changes are accounted for.

**Code Reorganization:**

* Files have been split and rearranged so there is (hopefully) less time spent hunting
  for functions or data tables.
* All `do_*()` functions are in appropriate `act_*.c` or `wiz_*.c` files.
* Introduced `do_filter*()` functions whose role is to break out of code flow in `do_*()`
  functions with appropriate messages.
* Several functions have been renamed to follow the same naming scheme for consistency's sake.
* More data has been moved from functions to tables to facilitate easy modification,
  addition, or removal of features.
* Integrated board and OLC features into the main code.
* `handler.c` has been seperated out into `chars.c`, `objs.c`, `rooms.c`, and `find.c`.
* Mob / object materials are now stored in a table rather than as a simple string.
* Unused flags in flag / type have been added to their respective tables so they're
  accounted for.
* Moved "day of the week", "months of the year", "weather", and "sun position" values
  to a tables in `tables.c`.
* Weather change calculations have been made easier to understand (but could still be better).
* Merged `db2.c` into `db.c` and separated old database functions into `db_old.c`.
* The `wear` command now uses a table for looking up wear slots, as does `char_wear_obj()`.
  This is a big improvement from copy+pasted "You wear ___ on ___" code for each possible slot.
* Separated descriptor functions in `comm.c` into new file `descs.c`.
* Moved some game loop functions from `comm.c` to `main.c`.

**Code Reduction:**

* **_LOTS_** of redundant code has been swapped up with small helper functions or...
  (gulp) macros. Macros are indeed ugly and bad - they'll be replaced with
  something better in a later iteration of refactoring, potentially using C++.
* Common shared code between `do_*()` functions have been separated out and moved to
  `do_sub.c`.
* Extremely common code patterns were replaced with macros for brevity.
  Most significant is `BAIL_IF(cond, msg, ch)` to replace
  `if (cond) { send_to_char(msg, ch); return; }` found in 99% of `do_*()` functions.
* Common linked-list code has been replaced with helper macros. This is a stepping-stone
  to a more reliable set of helper structs and functions to manage all lists.
* `get`, `put`, `drop`, and `give` have been refactored to use a lot of
  shared code.
* NPC breath attacks now use a shared `perform_breath_attack()` function.
* Replaced `is_safe()` and `is_safe_spell()` with consolidated
    `do_filter_can_attack_real()`. Added shorter functions:
    `do_filter_can_attack()`, `do_filter_can_attack_spell()`,
    `can_attack()`, and `can_attack_spell()`.
* Merged code for displaying exits in `look`, `exits`, and exits in the prompt.

**Code Changes:**

* Several systems have been overhauled completely, namely: the color system,
  recycleable objects, and the "lookup" functions.
* Separated mob- and player-specific flags (`ACT_*`) into separate `MOB_*` and `PLR_*`
  flags, with separate variables. This should prevent a _lot_ of bugs.
* `act_new()` now uses flags instead of an `int type` variable to determine recipients.
  Target `#define`s have been appropriately renamed to be more accurate. Flags
  could potentially be extended to have flags regarding visibility for blindness,
  deafness, or other options.
* Introduced functions `act2()` (messages to char + room)  and `act3()` (messages to char + target +
  room).
* Replaced some groups of `act()` to CHAR/NOTCHAR and CHAR/VICT/OTHERS to `act2()`
  and `act3()` respectively.
* Some spells (like 'blindness') now can be invoked with or without messages
  so other spells that use cause the affect won't give "spell failed" messages.
  **(A better solution would be a way to 'scope' functions like this to have their
     messages suppressed. C doesn't have anything like that, so perhaps a global
     stack-based system?)**
* Exits between areas are tracked internally using a "portal" system that link exits/rooms together
  with keywords. This doesn't change anything for gameplay or `.are` files, but will
  (eventually) make building easier by defining links between areas without writing in
  specific vnums (e.g, link `midgaard_west` to `midennir_east`).
* Introduced `dam_type` classes `DAM_PHYSICAL` and `DAM_MAGICAL` for determining
  immunity.

There are some higher-level abstractions that have likely caused a slight performance hit
or a little more memory usage, but this is hardly an issue in 2019 ;) Some profiling will
likely happen later on down the line.

## Game Changes:

Some small changes have been made, most of which are small quality-of-life
improvements, some of which are personal preferences that will eventually be
phased out or at least optional. A handful of changes were just for fun.

**Bugfixes:**
* The `scan` command now checks for mob visibility in rooms based on room light level.
* `scan` doesn't peek through doors anymore.
* Doors are now properly closed and locked and both sides when loading zones. This
  fixes some resets as well.
* Getting hit by fire or ice breath no longer makes you _less_ hungry or
  thirsty. Whoops!
* 'Word of recall' now writes "Spell failed." to the _caster_ rather than
  the victim.
* 'Heat metal' would drop an item _more_ if the opponent had more dexterity - this
  is fixed, but still needs balancing.
* In most cases, finding characters and objects by '1.thing', '2.thing', etc.
  will now count correctly when searching multiple locations (e.g, check the room,
  then the rest of the world). This is done by a `find_continue_count()` call
  preceding the next check.
* Checks for immune/resistant/vulnerable flags completely ignored default
  weapon/magic imm/res/vuln flags, despite existing code with implied behavior.
  Resistance no longer overrides default immunity, and vulnerability now
  properly _downgrades_ the default status (imm`->`res, res`->`normal,
  normal`->`vuln).
* When wearing an item, _all_ potential slots are now checked, not just related
  slots (e.g, a ring can now be held as well as worn on a finger).

**Patched Oversights:**
* `scan <dir>` only checks two rooms ahead rather than four. To see
  that far, you'll need to use the "farsight" spell :)
* Tweaked some stat tables very slightly to allow for steady progression in
  areas that looked like oversights. (Maximum weight capacity did not increase
  steadily where it easily could have.)
* You can no longer trip targets without feet or legs.
* Looking for doors by name now works the same way as looking for anything else.
  You can now find a door by number - for example, if you're carrying "a portable door",
  and there are two "door"s in the room, you can now say `open 3.door` to open the
  room's second door.
* Clean-ups and bugfixes to OLC to ensure stock zones are saved exactly as they are loaded.
  (Confirmed using `diff` comparisons on `.are` files and one gargantuan `everything.json`
  file containing the _entire_ world in JSON format)
* Certain messages from shopkeepers, trainers, etc. would _try_ to look and act like the
  `tell` command, but it wasn't consistent and generally unecessary. Now they're just
  simple messages. Hopefully this doesn't break any mob scripting or anything like that.

**Quality-of-Life Features:**
* Current position can be shown in prompt.
* Exits in the prompt now show open/closed status (!VANILLA).
* Stunned / incapacitated / mortally wounded / dead state checks are now based on
  percentage of max hit points.
* The 'lore' command has been implemented. It works like the 'identify' spell,
  but hand-picks information based on your skill percentage. What the player
  knows about is random, seeded by the player's name and the object's vnum.
  It currently cannot be practiced beyond 75%.
* Some, but not all, simple "Ok." messages have been replaced with useful things.
* Lots and lots of warning messages for mismatched doors or keys when loading zones.
  This produces some warning messages from the stock zones (which are warranted).
* New messages for passing through doors (for both sides).
* Drunkeness, thirst, hunger, and fullness now gives you more useful
  messages when the state changes via drink/eat.
* If standing on/at/in something, The `stand` command without any argument will
  now step off/out.

**Gameplay Changes:**
* Door open / closed(!VANILLA) status is visible in the "exits" command.
* Beds, tents, stools, etc. have automatic flags for `STAND_IN`, `STAND_AT`, etc.
  in addition to some standard hit and move regeneration bonuses.
* Bashing / tripping was effectively useless because combatants were
  automatically moved back to standing position in `violence_update()`. This
  is now a bit smarter, with cooldown rates.
* You cannot change position while in a daze / cooldown state. You can still
  issue several commands while knocked down, but you can't get back up!
* Standing up in combat is automatic once the cooldown period has been reached.
  This is essentially the same as before where you would _always_ stand up during
  combat when attacked, but now it's no longer instant.
* Form / parts for races have been reviewed and tweaked slightly (consistency between
  canids). It makes no gameplay difference, but may in the future.
* Tiny balance changes have been made to certain skills and stat regen amounts.
* Fast healing is only active when sitting, resting, or sleeping.
* Meditation is only active when sitting or resting.
* As a bonus feature, booting the MUD currently outputs several files to JSON
  format in the `json/` directory. In time, the MUD will be able to read these files
  instead or in addition to the standard ".are" files. This is to help develop
  editing software other than notepad or OLC.

**Optional Features (disabled by default - enable by uncommenting `#define VANILLA` in `basemud.h`):**
* More status noficiations in addition to 'excellent', 'nasty wounds', etc.
* Room titles are now colored based on their terrain type.
* The "pixie" race has been added just for fun. They can fly, and they're small,
  but boy are they frail! (this will undoubtedly get rolled back at some point...)
* The `score` command now shows your current hit/mana/move rates. This was a
  debug feature that I felt was nice enough to have.
* Hits and misses have additional adjectives to show their power in addition
  to the verb that shows percentage of max hit points. Example: "You wound a scary
  beholder with your  **excellent** slash."
* Attacks from successful enhanced damage rolls are now displayed as "heavy <type>".
  Example: "You scratch a lich with your epic **heavy** pierce."
* Open and closed doors are now visible in auto exits. This reveals some otherwise
  hidden exits but makes exploring _sooo_ much nicer.
* Hit / mana / move generation now happens on every pulse rather than in
  singular giant ticks. This is undoubtedly a CPU hog, and will need to be
  improved in some way! In the meantime, it's very luxurious :)
* When a character arrives in the room via standard movement, you will now see in what
  direction they came from.
* The (Glowing), (White Aura), (Red Aura) etc. tags have appropriate colors.
* Added (Golden Aura) and (Black Aura) for extreme good/evil characters.
* Added the `disengage` command to let spellcasters stop fighting and recover mana.
  Doesn't work if they're being targetted by anyone in the room.
* You can no longer sacrifice worthless objects.
* Added `@` command as a shortcut for `order all`.
* Added `abilities` command, which shows both skills _and_ spells.
* NPCs can no longer bash, trip, kick, etc. while they're on the ground.
* When affects are joined, the duration will stack as before, but the modifier will
  cap out at the min/max between the two affects. For example: Suppose the 'giant strength'
  spell could be stacked. Casting 'giant strength' with a +3 str bonus onto a target who already has a
  'giant strength' +2 str bonus will result in a +3 str bonus (the maximum of the two)
  rather the a +5 str bonus (the sum of the two). The same applies for negative bonuses like
  for 'chill touch' and 'plague'.
* Object / character materials can now be shown with the 'materials' command.
* Mobs can now be in stunned, incapacitated, and mortally wounded states.
* Caster mobs now say their spells when casting just like players.

Enjoy!

-- Synival
