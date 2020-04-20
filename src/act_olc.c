/***************************************************************************
 *  File: act_olc.c                                                        *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "recycle.h"
#include "lookup.h"
#include "interp.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "olc_save.h"
#include "act_info.h"
#include "chars.h"
#include "globals.h"
#include "items.h"
#include "mobiles.h"
#include "rooms.h"
#include "objs.h"
#include "mob_prog.h"
#include "descs.h"
#include "json_export.h"
#include "resets.h"
#include "memory.h"
#include "portals.h"

#include "olc_aedit.h"
#include "olc_hedit.h"
#include "olc_medit.h"
#include "olc_mpedit.h"
#include "olc_oedit.h"
#include "olc_redit.h"
#include "olc.h"

#include "act_olc.h"

void do_resets_display (CHAR_T *ch) {
    ROOM_INDEX_T *room;
    RESET_T *reset;
    RESET_VALUE_T *v;
    MOB_INDEX_T *mob = NULL;
    const WEAR_LOC_T *wear_loc;
    char buf[MAX_STRING_LENGTH];
    char final[MAX_STRING_LENGTH * 2];
    int reset_n = 0;

    EDIT_ROOM (ch, room);
    final[0] = '\0';

    send_to_char
        (" No.  Loads    Description       Location         Vnum   Mx Mn Description"
         "\n\r"
         "==== ======== ============= =================== ======== ===== ==========="
         "\n\r", ch);

    for (reset = room->reset_first; reset; reset = reset->room_next) {
        OBJ_INDEX_T *obj;
        MOB_INDEX_T *mob_index;
        OBJ_INDEX_T *obj_index;
        OBJ_INDEX_T *obj_to_index;
        ROOM_INDEX_T *room_index;

        final[0] = '\0';
        sprintf (final, "[%2d] ", ++reset_n);

        v = &(reset->v);
        switch (reset->command) {
            case 'M': {
                ROOM_INDEX_T *room_index_prev;

                if (!(mob_index = mobile_get_index (v->mob.mob_vnum))) {
                    sprintf (buf, "Load Mobile - Bad Mob %d\n\r", v->mob.mob_vnum);
                    strcat (final, buf);
                    continue;
                }

                if (!(room_index = room_get_index (v->mob.room_vnum))) {
                    sprintf (buf, "Load Mobile - Bad Room %d\n\r", v->mob.room_vnum);
                    strcat (final, buf);
                    continue;
                }

                mob = mob_index;
                sprintf (buf,
                    "M[%5d] %-13.13s in room             "
                    "R[%5d] %2d-%2d %-15.15s\n\r",
                    v->mob.mob_vnum, mob->short_descr, v->mob.room_vnum,
                    v->mob.global_limit, v->mob.room_limit, room_index->name);
                strcat (final, buf);

                /* Check for pet shop. */
                room_index_prev = room_get_index (room_index->vnum - 1);
                if (room_index_prev
                    && IS_SET (room_index_prev->room_flags, ROOM_PET_SHOP))
                    final[5] = 'P';
                break;
            }

            case 'O':
                if (!(obj_index = obj_get_index (v->obj.obj_vnum))) {
                    sprintf (buf, "Load Object - Bad Object %d\n\r", v->obj.obj_vnum);
                    strcat (final, buf);
                    continue;
                }

                obj = obj_index;
                if (!(room_index = room_get_index (v->obj.room_vnum))) {
                    sprintf (buf, "Load Object - Bad Room %d\n\r", v->obj.room_vnum);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf,
                    "O[%5d] %-13.13s in room             "
                    "R[%5d] %2d-%2d %-15.15s\n\r",
                    v->obj.obj_vnum, obj->short_descr, v->obj.room_vnum,
                    v->obj.global_limit, v->obj.room_limit, room_index->name);
                strcat (final, buf);
                break;

            case 'P':
                if (!(obj_index = obj_get_index (v->put.obj_vnum))) {
                    sprintf (buf, "Put Object - Bad Object %d\n\r", v->put.obj_vnum);
                    strcat (final, buf);
                    continue;
                }

                obj = obj_index;
                if (!(obj_to_index = obj_get_index (v->put.into_vnum))) {
                    sprintf (buf, "Put Object - Bad To Object %d\n\r", v->put.into_vnum);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf,
                    "O[%5d] %-13.13s inside              "
                    "O[%5d] %2d-%2d %-15.15s\n\r",
                     v->put.obj_vnum, obj->short_descr, v->put.into_vnum,
                     v->put.global_limit, v->put.put_count,
                     obj_to_index->short_descr);
                strcat (final, buf);
                break;

            case 'G':
            case 'E': {
                char cmd = reset->command;
                const char *cmdName = (cmd == 'G') ? "Give" : "Equip";
                int obj_vnum = (cmd == 'G') ? v->give.obj_vnum : v->equip.obj_vnum;

                if (!(obj_index = obj_get_index (obj_vnum))) {
                    sprintf (buf, "%s Object - Bad Object %d\n\r",
                        cmdName, obj_vnum);
                    strcat (final, buf);
                    continue;
                }

                obj = obj_index;
                if (!mob) {
                    sprintf (buf, "%s Object - No Previous Mobile\n\r", cmdName);
                    strcat (final, buf);
                    break;
                }
                if (mob->shop) {
                    sprintf (buf,
                        "O[%5d] %-13.13s in the inventory of S[%5d]       "
                        "%-15.15s\n\r",
                        obj_vnum, obj->short_descr, mob->vnum,
                        mob->short_descr);
                }
                else {
                    wear_loc = wear_loc_get ((reset->command == 'G')
                        ? WEAR_LOC_NONE : v->equip.wear_loc);
                    sprintf (buf,
                        "O[%5d] %-13.13s %-19.19s M[%5d] (%c)   "
                        "%-15.15s\n\r", obj_vnum, obj->short_descr,
                        (wear_loc ? wear_loc->phrase : "none"),
                        mob->vnum, reset->command, mob->short_descr);
                }
                strcat (final, buf);
                break;
            }

            /* Doors are set in rs_flags don't need to be displayed.
             * If you want to display them then uncomment the new_reset
             * line in the case 'D' in load_resets in db.c and here. */
            case 'D':
                room_index = room_get_index (v->door.room_vnum);
                sprintf (buf, "R[%5d] %s door of %-19.19s reset to %s\n\r",
                    v->door.room_vnum,
                    str_capitalized (door_table[v->door.dir].name),
                    room_index->name,
                    type_get_name (door_reset_types, v->door.locks));
                strcat (final, buf);
                break;

            case 'R':
                if (!(room_index = room_get_index (v->randomize.room_vnum))) {
                    sprintf (buf, "Randomize Exits - Bad Room %d\n\r", v->randomize.room_vnum);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf, "R[%5d] Exits are randomized in %s\n\r",
                    v->randomize.room_vnum, room_index->name);
                strcat (final, buf);
                break;

            default:
                sprintf (buf, "Bad reset command: %c.", reset->command);
                strcat (final, buf);
                break;
        }
        send_to_char (final, ch);
    }
}

/* Entry point for all editors. */
DEFINE_DO_FUN (do_olc) {
    char command[MAX_INPUT_LENGTH];
    int cmd;

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, command);
    if (command[0] == '\0') {
        do_help (ch, "olc");
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; editor_table[cmd].name != NULL; cmd++) {
        if (!str_prefix (command, editor_table[cmd].name)) {
            (*editor_table[cmd].do_fun) (ch, argument);
            return;
        }
    }

    /* Invalid command, send help. */
    do_help (ch, "olc");
}

/* Entry point for editing area_data. */
DEFINE_DO_FUN (do_aedit) {
    AREA_T *area;
    int value;
    char arg1[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);
    if (!str_cmp (arg1, "create")) {
        BAIL_IF (ch->pcdata->security < 9,
            "AEdit: Insufficient security to create area.\n\r", ch);
        aedit_create (ch, "");
        ch->desc->editor = ED_AREA;
        return;
    }

    if (is_number (arg1)) {
        value = atoi (arg1);
        BAIL_IF (!(area = area_get_by_vnum (value)),
            "AEdit: That area vnum does not exist.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, area),
            "AEdit: Insufficient security to edit areas.\n\r", ch);
    }
    else
        area = ch->in_room->area;

    BAIL_IF (area == NULL,
        "REdit: There is no default room to edit.\n\r", ch);

    ch->desc->olc_edit = (void *) area;
    ch->desc->editor = ED_AREA;
}

DEFINE_DO_FUN (do_hedit) {
    HELP_T *help;
    char arg1[MIL];
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];
    bool found = FALSE;

    strcpy (arg1, argument);
    if (argument[0] != '\0') {
        /* Taken from do_help */
        argall[0] = '\0';
        while (argument[0] != '\0') {
            argument = one_argument (argument, argone);
            if (argall[0] != '\0')
                strcat (argall, " ");
            strcat (argall, argone);
        }
        for (help = help_first; help != NULL; help = help->global_next) {
            if (str_in_namelist (argall, help->keyword)) {
                ch->desc->olc_edit = (void *) help;
                ch->desc->editor = ED_HELP;
                found = TRUE;
                return;
            }
        }
    }
    if (!found) {
        char arg2[MAX_STRING_LENGTH];
        argument = one_argument (arg1, arg2);
        if (!str_cmp (arg2, "new")) {
            BAIL_IF (argument[0] == '\0',
                "Syntax: edit help new [topic]\n\r", ch);
            if (hedit_new (ch, argument))
                ch->desc->editor = ED_HELP;
            return;
        }
    }
    printf_to_char (ch, "HEdit: There is no default help to edit.\n\r");
}

/* Entry point for editing mob_index_data. */
DEFINE_DO_FUN (do_medit) {
    MOB_INDEX_T *mob;
    AREA_T *area;
    int value;
    char arg1[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);

    if (is_number (arg1)) {
        value = atoi (arg1);
        BAIL_IF (!(mob = mobile_get_index (value)),
            "MEdit: That vnum does not exist.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, mob->area),
            "MEdit: Insufficient security to modify mobs.\n\r", ch);
        ch->desc->olc_edit = (void *) mob;
        ch->desc->editor = ED_MOBILE;
        return;
    }
    else if (!str_cmp (arg1, "create")) {
        value = atoi (argument);
        BAIL_IF (argument[0] == '\0' || value == 0,
            "Syntax: edit mobile create [vnum]\n\r", ch);

        area = area_get_by_inner_vnum (value);
        BAIL_IF (!area,
            "MEdit: That vnum is not assigned an area.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, area),
            "MEdit: Insufficient security to create mobs.\n\r", ch);
        if (medit_create (ch, argument)) {
            SET_BIT (area->area_flags, AREA_CHANGED);
            ch->desc->editor = ED_MOBILE;
        }
        return;
    }

    printf_to_char (ch, "MEdit: There is no default mobile to edit.\n\r");
}

DEFINE_DO_FUN (do_mpedit) {
    MPROG_CODE_T *mcode;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument (argument, command);
    if (is_number (command)) {
        int vnum = atoi (command);
        AREA_T *ad;

        BAIL_IF ((mcode = mpcode_get_index (vnum)) == NULL,
            "MPEdit: That vnum does not exist.\n\r", ch);

        ad = area_get_by_inner_vnum (vnum);
        BAIL_IF (ad == NULL,
            "MPEdit: That vnum is not assigned an area.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, ad),
            "MPEdit: Insufficient security to modify mob programs.\n\r", ch);

        ch->desc->olc_edit = (void *) mcode;
        ch->desc->editor = ED_MPCODE;
        return;
    }
    else if (!str_cmp (command, "create")) {
        BAIL_IF (argument[0] == '\0',
            "Syntax: mpedit create [vnum]\n\r", ch);
        mpedit_create (ch, argument);
        return;
    }

    printf_to_char (ch, "Syntax: mpedit [vnum]\n\r");
    printf_to_char (ch, "        mpedit create [vnum]\n\r");
}

/* Entry point for editing obj_index_data. */
DEFINE_DO_FUN (do_oedit) {
    OBJ_INDEX_T *obj;
    AREA_T *area;
    char arg1[MAX_STRING_LENGTH];
    int value;

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);

    if (is_number (arg1)) {
        value = atoi (arg1);
        BAIL_IF (!(obj = obj_get_index (value)),
            "OEdit: That vnum does not exist.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, obj->area),
            "OEdit: Insufficient security to modify objects.\n\r", ch);
        ch->desc->olc_edit = (void *) obj;
        ch->desc->editor = ED_OBJECT;
        return;
    }
    else if (!str_cmp (arg1, "create")) {
        value = atoi (argument);
        BAIL_IF (argument[0] == '\0' || value == 0,
            "Syntax: edit object create [vnum]\n\r", ch);

        area = area_get_by_inner_vnum (value);
        BAIL_IF (!area,
            "OEdit: That vnum is not assigned an area.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, area),
            "OEdit: Insufficient security to create objects.\n\r", ch);
        if (oedit_create (ch, argument)) {
            SET_BIT (area->area_flags, AREA_CHANGED);
            ch->desc->editor = ED_OBJECT;
        }
        return;
    }

    printf_to_char (ch, "OEdit: There is no default object to edit.\n\r");
}

/* Entry point for editing room_index_data. */
DEFINE_DO_FUN (do_redit) {
    ROOM_INDEX_T *room;
    AREA_T *area;
    char arg1[MAX_STRING_LENGTH];
    int value;

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);

    /* redit reset */
    if (!str_cmp (arg1, "reset")) {
        room = ch->in_room;
        BAIL_IF (!IS_BUILDER (ch, room->area),
            "REdit: Insufficient security to modify room.\n\r", ch);

        room_reset (room);
        printf_to_char (ch, "Room reset.\n\r");
        return;
    }
    /* redit create <vnum> */
    else if (!str_cmp (arg1, "create")) {
        value = atoi (argument);
        BAIL_IF (argument[0] == '\0' || value == 0,
            "Syntax: edit room create [vnum]\n\r", ch);

        area = area_get_by_inner_vnum (value);
        BAIL_IF (!area,
            "REdit: That vnum is not assigned an area.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, area),
            "REdit: Insufficient security to create room.\n\r", ch);

        if (redit_create (ch, argument)) {
            ch->desc->editor = ED_ROOM;
            char_to_room (ch, ch->desc->olc_edit);
            SET_BIT (((ROOM_INDEX_T *) ch->desc->olc_edit)->area->area_flags,
                     AREA_CHANGED);
        }
        return;
    }

    /* redit <vnum> */
    if (is_number (arg1)) {
        room = room_get_index (atoi (arg1));
        BAIL_IF (!room,
            "REdit: That vnum does not exist.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, room->area),
            "REdit: Insufficient security to modify room.\n\r", ch);
        char_to_room (ch, room);
        return;
    }
    else
        room = ch->in_room;

    BAIL_IF (room == NULL,
        "REdit: There is no default room to edit.\n\r", ch);

    ch->desc->olc_edit = (void *) room;
    ch->desc->editor = ED_ROOM;
}

#define DO_RESETS_SYNTAX \
    "Syntax: RESETS\n\r" \
    "        RESETS <number> OBJ <vnum> <wear_loc>\n\r" \
    "        RESETS <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r" \
    "        RESETS <number> OBJ <vnum> room\n\r" \
    "        RESETS <number> MOB <vnum> [max #x area] [max #x room]\n\r" \
    "        RESETS <number> DELETE\n\r" \
    "        RESETS <number> RANDOM [#x exits]\n\r"

DEFINE_DO_FUN (do_resets) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    char arg6[MAX_INPUT_LENGTH];
    char arg7[MAX_INPUT_LENGTH];
    RESET_T *reset = NULL;
    RESET_VALUE_T *v;
    ROOM_INDEX_T *room;
    int index;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    argument = one_argument (argument, arg3);
    argument = one_argument (argument, arg4);
    argument = one_argument (argument, arg5);
    argument = one_argument (argument, arg6);
    argument = one_argument (argument, arg7);

    BAIL_IF (!IS_BUILDER (ch, ch->in_room->area),
        "Resets: Invalid security for editing this area.\n\r", ch);

    /* Display resets in current room. */
    if (arg1[0] == '\0') {
        BAIL_IF (!ch->in_room->reset_first,
            "No resets in this room.\n\r", ch);

        printf_to_char (ch, "Resets: M = mobile, R = room, O = object, "
                      "P = pet, S = shopkeeper\n\r");
        do_resets_display (ch);
        return;
    }

    /* Argument 1 must always be a number. */
    BAIL_IF (!is_number (arg1),
        DO_RESETS_SYNTAX, ch);

    /* Take index number and search for commands. */
    index = atoi (arg1) - 1;
    room = ch->in_room;

    /* Delete a reset. */
    if (!str_cmp (arg2, "delete")) {
        BAIL_IF (!ch->in_room->reset_first,
            "No resets in this area.\n\r", ch);

        if (index == 0)
            reset = room->reset_first;
        else if (index < 0)
            reset = room->reset_last;
        else {
            int reset_n = 0;
            LIST_FIND (reset_n++ == index, room_next,
                room->reset_first, reset);
            BAIL_IF (!reset,
                "Reset not found.\n\r", ch);
            LIST2_REMOVE (reset, room_prev, room_next,
                room->reset_first, room->reset_last);
        }

        reset_data_free (reset);
        printf_to_char (ch, "Reset deleted.\n\r");
        return;
    }

    /* Add a reset. */
    if ((!str_cmp (arg2, "mob") && is_number (arg3)) ||
        (!str_cmp (arg2, "obj") && is_number (arg3)))
    {
        /* Check for Mobile reset. */
        if (!str_cmp (arg2, "mob")) {
            BAIL_IF (mobile_get_index (is_number (arg3) ? atoi (arg3) : 1) == NULL,
                "Mob doesn't exist.\n\r", ch);
            reset = reset_data_new ();
            reset->command = 'M';

            v = &(reset->v);
            v->mob.mob_vnum     = atoi (arg3);
            v->mob.global_limit = is_number (arg4) ? atoi (arg4) : 1;
            v->mob.room_vnum    = ch->in_room->vnum;
            v->mob.room_limit   = is_number (arg5) ? atoi (arg5) : 1;
        }
        /* Check for Object reset. */
        else if (!str_cmp (arg2, "obj")) {
            reset = reset_data_new ();
            v = &(reset->v);

            /* Inside another object. */
            if (!str_prefix (arg4, "inside")) {
                OBJ_INDEX_T *temp;

                temp = obj_get_index (is_number (arg5) ? atoi (arg5) : 1);
                BAIL_IF (!item_index_is_container (temp),
                    "Object 2 is not a container.\n\r", ch);
                reset->command = 'P';
                v->put.obj_vnum     = atoi (arg3);
                v->put.global_limit = is_number (arg6) ? atoi (arg6) : 1;
                v->put.into_vnum    = is_number (arg5) ? atoi (arg5) : 1;
                v->put.put_count    = is_number (arg7) ? atoi (arg7) : 1;
            }
            /* Inside the room. */
            else if (!str_cmp (arg4, "room")) {
                BAIL_IF (obj_get_index (atoi (arg3)) == NULL,
                    "Vnum doesn't exist.\n\r", ch);
                reset->command = 'O';
                v->obj.obj_vnum     = atoi (arg3);
                v->obj.global_limit = 0;
                v->obj.room_vnum    = ch->in_room->vnum;
            }
            /* Into a Mobile's inventory. */
            else {
                const WEAR_LOC_T *wear_loc;

                BAIL_IF ((wear_loc = wear_loc_get_by_name (arg4)) == NULL,
                    "Resets: '? wear-loc'\n\r", ch);
                BAIL_IF (obj_get_index (atoi (arg3)) == NULL,
                    "Vnum doesn't exist.\n\r", ch);

                if (wear_loc->type WEAR_LOC_NONE) {
                    reset->command = 'G';
                    v->give.obj_vnum     = atoi (arg3);
                    v->give.global_limit = 0;
                }
                else {
                    reset->command = 'E';
                    v->equip.obj_vnum     = atoi (arg3);
                    v->equip.global_limit = 0;
                    v->equip.wear_loc     = wear_loc->type;
                }
            }
        }
        reset_to_room_before (reset, ch->in_room, index);
        SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
        printf_to_char (ch, "Reset added.\n\r");
        return;
    }

    if (!str_cmp (arg2, "random") && is_number (arg3)) {
        BAIL_IF (atoi (arg3) < 1 || atoi (arg3) > 6,
            "Invalid argument.\n\r", ch);
        reset = reset_data_new ();
        reset->command = 'R';
        v = &(reset->v);

        v->randomize.room_vnum = ch->in_room->vnum;
        v->randomize.dir_count = atoi (arg3);
        reset_to_room_before (reset, ch->in_room, index);
        SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
        printf_to_char (ch, "Random exits reset added.\n\r");
        return;
    }

    /* No valid reset found. */
    send_to_char (DO_RESETS_SYNTAX, ch);
}

/*****************************************************************************
 Name:      do_alist
 Purpose:   Normal command to list areas and display area information.
 Called by: interpreter(interp.c)
 ****************************************************************************/
DEFINE_DO_FUN (do_alist) {
    char buf[MAX_STRING_LENGTH];
    char result[MAX_STRING_LENGTH * 2];    /* May need tweaking. */
    AREA_T *area;

    if (IS_NPC (ch))
        return;

    sprintf (result, "[%3s] [%-27s] (%-5s-%5s) [%-10s] %3s [%-10s]\n\r",
             "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec",
             "Builders");

    for (area = area_first; area; area = area->global_next) {
        sprintf (buf,
                 "[%3d] %-29.29s (%-5d-%5d) %-12.12s [%d] [%-10.10s]\n\r",
                 area->vnum, area->title, area->min_vnum, area->max_vnum,
                 area->filename, area->security, area->builders);
        strcat (result, buf);
    }

    send_to_char (result, ch);
}

/*****************************************************************************
 Name:      do_asave
 Purpose:   Entry point for saving area data.
 Called by: interpreter(interp.c)
 ****************************************************************************/
#define DO_ASAVE_SYNTAX \
    "Syntax:\n\r" \
    "  asave <vnum>   - saves a particular area\n\r" \
    "  asave list     - saves the area.lst file\n\r" \
    "  asave portals  - saves config/portals.json\n\r" \
    "  asave area     - saves the area being edited\n\r" \
    "  asave changed  - saves all changed zones\n\r" \
    "  asave world    - saves the world! (db dump)\n\r"

DEFINE_DO_FUN (do_asave) {
    char arg1[MAX_INPUT_LENGTH];
    AREA_T *area;
    int value;

    str_smash_tilde (argument);
    strcpy (arg1, argument);

    BAIL_IF (arg1[0] == '\0',
        DO_ASAVE_SYNTAX, ch);

    /* Snarf the value (which need not be numeric). */
    value = atoi (arg1);
    BAIL_IF (!(area = area_get_by_vnum (value)) && is_number (arg1),
        "ASave: That area does not exist.\n\r", ch);

    /* Save area of given vnum. */
    /* ------------------------ */
    if (is_number (arg1)) {
        BAIL_IF (ch && !IS_BUILDER (ch, area),
            "ASave: You are not a builder for this area.\n\r", ch);
        save_area_list ();
        json_export_portals (JSON_EXPORT_MODE_SAVE);
        save_area (area);
        printf_to_char (ch, "Area saved.\n\r");
        return;
    }

    /* Save the world, only authorized areas. */
    /* -------------------------------------- */
    if (!str_cmp ("world", arg1)) {
        printf_to_char (ch, "Saving world...\n\r");
        desc_flush_output (ch->desc);
        json_export_portals (JSON_EXPORT_MODE_SAVE);
        save_area_list ();

        for (area = area_first; area; area = area->global_next) {
            /* Builder must be assigned this area. */
            if (ch && !IS_BUILDER (ch, area))
                continue;

            printf_to_char (ch, "- %s\n\r", area->filename);
            desc_flush_output (ch->desc);

            save_area (area);
            REMOVE_BIT (area->area_flags, AREA_CHANGED);
        }
        if (ch)
            printf_to_char (ch, "You saved the world.\n\r");
        save_other_helps (NULL);
        return;
    }

    /* Save changed areas, only authorized areas. */
    /* ------------------------------------------ */
    if (!str_cmp ("changed", arg1)) {
        char buf[MAX_INPUT_LENGTH];
        save_area_list ();
        json_export_portals (JSON_EXPORT_MODE_SAVE);

        if (ch)
            printf_to_char (ch, "Saved zones:\n\r");
        else
            log_string ("Saved zones:");
        sprintf (buf, "None.\n\r");

        for (area = area_first; area; area = area->global_next) {
            /* Builder must be assigned this area. */
            if (ch && !IS_BUILDER (ch, area))
                continue;

            /* Save changed areas. */
            if (!IS_SET (area->area_flags, AREA_CHANGED))
                continue;

            save_area (area);
            sprintf (buf, "%24s - '%s'", area->title, area->filename);
            if (ch)
                printf_to_char (ch, "%s\n\r", buf);
            else
                log_string (buf);
            REMOVE_BIT (area->area_flags, AREA_CHANGED);
        }

        save_other_helps (ch);
        if (!str_cmp (buf, "None.\n\r")) {
            if (ch)
                send_to_char (buf, ch);
            else
                log_string ("None.");
        }
        return;
    }

    /* Save the area.lst file. */
    /* ----------------------- */
    if (!str_cmp (arg1, "list")) {
        save_area_list ();
        printf_to_char (ch, "Area list saved.\n\r");
        return;
    }

    /* Save the area.lst file. */
    /* ----------------------- */
    if (!str_cmp (arg1, "portals")) {
        json_export_portals (JSON_EXPORT_MODE_SAVE);
        printf_to_char (ch, "Portals saved.\n\r");
        return;
    }

    /* Save area being edited, if authorized. */
    /* -------------------------------------- */
    if (!str_cmp (arg1, "area")) {
        if (!ch || !ch->desc)
            return;

        /* Is character currently editing. */
        BAIL_IF (ch->desc->editor == ED_NONE,
            "ASave: You are not editing an area, "
                   "therefore an area vnum is required.\n\r", ch);

        /* Find the area to save. */
        switch (ch->desc->editor) {
            case ED_AREA:
                area = (AREA_T *) ch->desc->olc_edit;
                break;
            case ED_ROOM:
                area = ch->in_room->area;
                break;
            case ED_OBJECT:
                area = ((OBJ_INDEX_T *) ch->desc->olc_edit)->area;
                break;
            case ED_MOBILE:
                area = ((MOB_INDEX_T *) ch->desc->olc_edit)->area;
                break;
            case ED_HELP: {
                int saved;

                printf_to_char (ch, "Saving all changed helps:\n\r");
                saved = save_other_helps (ch);
                if (saved == 0)
                    printf_to_char (ch, "No helps to save.\n\r");
                else
                    printf_to_char (ch, "%d helps to saved.\n\r", saved);

                return;
            }
            default:
                area = ch->in_room->area;
                break;
        }

        BAIL_IF (!IS_BUILDER (ch, area),
            "ASave: You are not a builder for this area.\n\r", ch);

        save_area_list ();
        json_export_portals (JSON_EXPORT_MODE_SAVE);
        save_area (area);
        REMOVE_BIT (area->area_flags, AREA_CHANGED);
        printf_to_char (ch, "Area saved.\n\r");
        return;
    }

    /* Show correct syntax. */
    /* -------------------- */
    if (ch)
        do_asave (ch, "");
}

#define DO_PORTALS_SYNTAX \
    "Syntax: portals exits <area | world>\n\r" \
    "        portals links <area | world>\n\r"

DEFINE_DO_FUN (do_portals) {
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    BUFFER_T *pagebuf;
    PORTAL_T *portal;
    PORTAL_EXIT_T *pex;
    AREA_T *area;
    const char *arrow;
    int index;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    /* arg2 must be 'area' or 'world'. */
    if (arg2[0] != '\0' && !str_prefix (arg2, "area")) {
        BAIL_IF ((area = ch->in_room->area) == NULL,
            "You aren't in an area!\n\r", ch);
    }
    else if (arg2[0] != '\0' && !str_prefix (arg2, "world"))
        area = NULL;
    else {
        send_to_char (DO_PORTALS_SYNTAX, ch);
        return;
    }

    /* Check for "portals exits" or "portals links" */
    if (arg1[0] != '\0' && !str_prefix (arg1, "exits")) {
        pagebuf = buf_new ();
        index = 1;

        for (pex = portal_exit_get_first(); pex; pex = portal_exit_get_next (pex)) {
            /* make sure this exit is somehow connected to this area. */
            if (area && !portal_exit_is_in_area (pex, area))
                continue;

            printf_to_buf (pagebuf, "%3d. %20.20s | ", index, pex->name);
            index++;

            if (pex->room) {
                snprintf (buf, sizeof (buf), "on room #%d (%s)",
                    pex->room->vnum, pex->room->name);
                printf_to_buf (pagebuf, "%-51.51s\n\r", buf);
            }
            else if (pex->exit) {
                snprintf (buf, sizeof (buf), "%s from #%d (%s)",
                    door_get (pex->exit->orig_door)->name,
                    pex->exit->from_room->vnum,
                    pex->exit->from_room->name);
                printf_to_buf (pagebuf, "%-51.51s\n\r", buf);

                if (pex->exit->to_room) {
                    snprintf (buf, sizeof (buf), "   to #%d (%s)",
                        pex->exit->to_room->vnum, pex->exit->to_room->name);
                    printf_to_buf (pagebuf,
                        "                          | %-51.51s\n\r", buf);
                }
                else {
                    printf_to_buf (pagebuf,
                        "                          |    to *NOT CONNECTED*\n\r");
                }
            }
            else
                printf_to_buf (pagebuf, "*NO LINK*\n\r");
        }

        page_to_char (buf_string (pagebuf), ch);
        buf_free (pagebuf);
        return;
    }

    if (arg1[0] != '\0' && !str_prefix (arg1, "links")) {
        pagebuf = buf_new ();
        index = 1;

        for (portal = portal_get_first(); portal; portal = portal_get_next (portal)) {
            const char *name_left, *name_right;
            const PORTAL_EXIT_T *pex_left, *pex_right;
            bool reverse;

            /* make sure this link is somehow connected to this area. */
            if (area != NULL) {
                int in_area_result;
                in_area_result = portal_is_in_area (portal, area);

                if (in_area_result & PORTAL_IN_AREA_FROM)
                    reverse = FALSE;
                else if (in_area_result & PORTAL_IN_AREA_TO)
                    reverse = TRUE;
                else
                    continue;
            }
            else
                reverse = FALSE;


            name_left  = reverse ? portal->name_to   : portal->name_from;
            name_right = reverse ? portal->name_from : portal->name_to;
            pex_left   = reverse ? portal->to        : portal->from;
            pex_right  = reverse ? portal->from      : portal->to;
            arrow = (portal->two_way) ? "<==>" : (reverse ? "<---" : "--->");

            printf_to_buf (pagebuf, "%3d. %20.20s [%s] %s [%s] %-20.20s\n\r",
                index,
                name_left, pex_left ? "Connected" : "*MISSING*",
                arrow,
                pex_right ? "Connected" : "*MISSING*", name_right
            );
            index++;
        }

        page_to_char (buf_string (pagebuf), ch);
        buf_free (pagebuf);
        return;
    }

    /* no matches - show syntax. */
    send_to_char (DO_PORTALS_SYNTAX, ch);
}
