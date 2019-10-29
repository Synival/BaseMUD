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

#include "olc_aedit.h"
#include "olc_hedit.h"
#include "olc_medit.h"
#include "olc_mpedit.h"
#include "olc_oedit.h"
#include "olc_redit.h"
#include "olc.h"

#include "act_olc.h"

void do_resets_display (CHAR_DATA * ch) {
    ROOM_INDEX_DATA *pRoom;
    RESET_DATA *pReset;
    RESET_VALUE_TYPE *v;
    MOB_INDEX_DATA *pMob = NULL;
    char buf[MAX_STRING_LENGTH];
    char final[MAX_STRING_LENGTH * 2];
    int iReset = 0;

    EDIT_ROOM (ch, pRoom);
    final[0] = '\0';

    send_to_char
        (" No.  Loads    Description       Location         Vnum   Mx Mn Description"
         "\n\r"
         "==== ======== ============= =================== ======== ===== ==========="
         "\n\r", ch);

    for (pReset = pRoom->reset_first; pReset; pReset = pReset->next) {
        OBJ_INDEX_DATA *pObj;
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_INDEX_DATA *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;

        final[0] = '\0';
        sprintf (final, "[%2d] ", ++iReset);

        v = &(pReset->v);
        switch (pReset->command) {
            case 'M': {
                ROOM_INDEX_DATA *pRoomIndexPrev;

                if (!(pMobIndex = get_mob_index (v->mob.mob_vnum))) {
                    sprintf (buf, "Load Mobile - Bad Mob %d\n\r", v->mob.mob_vnum);
                    strcat (final, buf);
                    continue;
                }

                if (!(pRoomIndex = get_room_index (v->mob.room_vnum))) {
                    sprintf (buf, "Load Mobile - Bad Room %d\n\r", v->mob.room_vnum);
                    strcat (final, buf);
                    continue;
                }

                pMob = pMobIndex;
                sprintf (buf,
                    "M[%5d] %-13.13s in room             "
                    "R[%5d] %2d-%2d %-15.15s\n\r",
                    v->mob.mob_vnum, pMob->short_descr, v->mob.room_vnum,
                    v->mob.global_limit, v->mob.room_limit, pRoomIndex->name);
                strcat (final, buf);

                /* Check for pet shop. */
                pRoomIndexPrev = get_room_index (pRoomIndex->vnum - 1);
                if (pRoomIndexPrev
                    && IS_SET (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
                    final[5] = 'P';
                break;
            }

            case 'O':
                if (!(pObjIndex = get_obj_index (v->obj.obj_vnum))) {
                    sprintf (buf, "Load Object - Bad Object %d\n\r", v->obj.obj_vnum);
                    strcat (final, buf);
                    continue;
                }

                pObj = pObjIndex;
                if (!(pRoomIndex = get_room_index (v->obj.room_vnum))) {
                    sprintf (buf, "Load Object - Bad Room %d\n\r", v->obj.room_vnum);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf,
                    "O[%5d] %-13.13s in room             "
                    "R[%5d] %2d-%2d %-15.15s\n\r",
                    v->obj.obj_vnum, pObj->short_descr, v->obj.room_vnum,
                    v->obj.global_limit, v->obj.room_limit, pRoomIndex->name);
                strcat (final, buf);
                break;

            case 'P':
                if (!(pObjIndex = get_obj_index (v->put.obj_vnum))) {
                    sprintf (buf, "Put Object - Bad Object %d\n\r", v->put.obj_vnum);
                    strcat (final, buf);
                    continue;
                }

                pObj = pObjIndex;
                if (!(pObjToIndex = get_obj_index (v->put.into_vnum))) {
                    sprintf (buf, "Put Object - Bad To Object %d\n\r", v->put.into_vnum);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf,
                    "O[%5d] %-13.13s inside              "
                    "O[%5d] %2d-%2d %-15.15s\n\r",
                     v->put.obj_vnum, pObj->short_descr, v->put.into_vnum,
                     v->put.global_limit, v->put.put_count,
                     pObjToIndex->short_descr);
                strcat (final, buf);
                break;

            case 'G':
            case 'E': {
                char cmd = pReset->command;
                const char *cmdName = (cmd == 'G') ? "Give" : "Equip";
                int obj_vnum = (cmd == 'G') ? v->give.obj_vnum : v->equip.obj_vnum;

                if (!(pObjIndex = get_obj_index (obj_vnum))) {
                    sprintf (buf, "%s Object - Bad Object %d\n\r",
                        cmdName, obj_vnum);
                    strcat (final, buf);
                    continue;
                }

                pObj = pObjIndex;
                if (!pMob) {
                    sprintf (buf, "%s Object - No Previous Mobile\n\r", cmdName);
                    strcat (final, buf);
                    break;
                }
                if (pMob->pShop) {
                    sprintf (buf,
                        "O[%5d] %-13.13s in the inventory of S[%5d]       "
                        "%-15.15s\n\r",
                        obj_vnum, pObj->short_descr, pMob->vnum,
                        pMob->short_descr);
                }
                else {
                    sprintf (buf,
                        "O[%5d] %-13.13s %-19.19s M[%5d] (%c)   "
                        "%-15.15s\n\r",
                        obj_vnum, pObj->short_descr,
                        (pReset->command == 'G')
                            ? flag_string (wear_loc_phrases, WEAR_NONE)
                            : flag_string (wear_loc_phrases, v->equip.wear_loc),
                        pMob->vnum, pReset->command, pMob->short_descr);
                }
                strcat (final, buf);
                break;
            }

            /* Doors are set in rs_flags don't need to be displayed.
             * If you want to display them then uncomment the new_reset
             * line in the case 'D' in load_resets in db.c and here. */
            /* ^^^ new_reset() is now room_take_reset(). -- Synival */
            case 'D':
                pRoomIndex = get_room_index (v->door.room_vnum);
                sprintf (buf, "R[%5d] %s door of %-19.19s reset to %s\n\r",
                    v->door.room_vnum,
                    capitalize (door_table[v->door.dir].name),
                    pRoomIndex->name,
                    flag_string (door_resets, v->door.locks));
                strcat (final, buf);

                break;

            case 'R':
                if (!(pRoomIndex = get_room_index (v->randomize.room_vnum))) {
                    sprintf (buf, "Randomize Exits - Bad Room %d\n\r", v->randomize.room_vnum);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf, "R[%5d] Exits are randomized in %s\n\r",
                    v->randomize.room_vnum, pRoomIndex->name);
                strcat (final, buf);
                break;

            default:
                sprintf (buf, "Bad reset command: %c.", pReset->command);
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
    AREA_DATA *pArea;
    int value;
    char arg1[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
        return;

    pArea = ch->in_room->area;
    argument = one_argument (argument, arg1);

    if (is_number (arg1)) {
        value = atoi (arg1);
        BAIL_IF (!(pArea = area_get_by_vnum (value)),
            "AEdit: That area vnum does not exist.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, pArea),
            "AEdit: Insufficient security to edit areas.\n\r", ch);
        ch->desc->pEdit = (void *) pArea;
        ch->desc->editor = ED_AREA;
        return;
    }
    else if (!str_cmp (arg1, "create")) {
        BAIL_IF (ch->pcdata->security < 9,
            "AEdit: Insufficient security to create area.\n\r", ch);
        aedit_create (ch, "");
        ch->desc->editor = ED_AREA;
        return;
    }
}

DEFINE_DO_FUN (do_hedit) {
    HELP_DATA *pHelp;
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
        for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next) {
            if (is_name (argall, pHelp->keyword)) {
                ch->desc->pEdit = (void *) pHelp;
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
    send_to_char ("HEdit: There is no default help to edit.\n\r", ch);
}

/* Entry point for editing mob_index_data. */
DEFINE_DO_FUN (do_medit) {
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int value;
    char arg1[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);

    if (is_number (arg1)) {
        value = atoi (arg1);
        BAIL_IF (!(pMob = get_mob_index (value)),
            "MEdit: That vnum does not exist.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, pMob->area),
            "MEdit: Insufficient security to modify mobs.\n\r", ch);
        ch->desc->pEdit = (void *) pMob;
        ch->desc->editor = ED_MOBILE;
        return;
    }
    else if (!str_cmp (arg1, "create")) {
        value = atoi (argument);
        BAIL_IF (argument[0] == '\0' || value == 0,
            "Syntax: edit mobile create [vnum]\n\r", ch);

        pArea = area_get_by_inner_vnum (value);
        BAIL_IF (!pArea,
            "MEdit: That vnum is not assigned an area.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, pArea),
            "MEdit: Insufficient security to create mobs.\n\r", ch);
        if (medit_create (ch, argument)) {
            SET_BIT (pArea->area_flags, AREA_CHANGED);
            ch->desc->editor = ED_MOBILE;
        }
        return;
    }

    send_to_char ("MEdit: There is no default mobile to edit.\n\r", ch);
}

DEFINE_DO_FUN (do_mpedit) {
    MPROG_CODE *pMcode;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument (argument, command);
    if (is_number (command)) {
        int vnum = atoi (command);
        AREA_DATA *ad;

        BAIL_IF ((pMcode = get_mprog_index (vnum)) == NULL,
            "MPEdit: That vnum does not exist.\n\r", ch);

        ad = area_get_by_inner_vnum (vnum);
        BAIL_IF (ad == NULL,
            "MPEdit: That vnum is not assigned an area.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, ad),
            "MPEdit: Insufficient security to modify mob programs.\n\r", ch);

        ch->desc->pEdit = (void *) pMcode;
        ch->desc->editor = ED_MPCODE;
        return;
    }
    else if (!str_cmp (command, "create")) {
        BAIL_IF (argument[0] == '\0',
            "Syntax: mpedit create [vnum]\n\r", ch);
        mpedit_create (ch, argument);
        return;
    }

    send_to_char ("Syntax: mpedit [vnum]\n\r", ch);
    send_to_char ("        mpedit create [vnum]\n\r", ch);
}

/* Entry point for editing obj_index_data. */
DEFINE_DO_FUN (do_oedit) {
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    char arg1[MAX_STRING_LENGTH];
    int value;

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);

    if (is_number (arg1)) {
        value = atoi (arg1);
        BAIL_IF (!(pObj = get_obj_index (value)),
            "OEdit: That vnum does not exist.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, pObj->area),
            "OEdit: Insufficient security to modify objects.\n\r", ch);
        ch->desc->pEdit = (void *) pObj;
        ch->desc->editor = ED_OBJECT;
        return;
    }
    else if (!str_cmp (arg1, "create")) {
        value = atoi (argument);
        BAIL_IF (argument[0] == '\0' || value == 0,
            "Syntax: edit object create [vnum]\n\r", ch);

        pArea = area_get_by_inner_vnum (value);
        BAIL_IF (!pArea,
            "OEdit: That vnum is not assigned an area.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, pArea),
            "OEdit: Insufficient security to create objects.\n\r", ch);
        if (oedit_create (ch, argument)) {
            SET_BIT (pArea->area_flags, AREA_CHANGED);
            ch->desc->editor = ED_OBJECT;
        }
        return;
    }

    send_to_char ("OEdit: There is no default object to edit.\n\r", ch);
}

/* Entry point for editing room_index_data. */
DEFINE_DO_FUN (do_redit) {
    ROOM_INDEX_DATA *pRoom;
    AREA_DATA *pArea;
    char arg1[MAX_STRING_LENGTH];
    int value;

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);
    pRoom = ch->in_room;

    /* redit <vnum> */
    if (is_number (arg1)) {
        pRoom = get_room_index (atoi (arg1));
        BAIL_IF (!pRoom,
            "REdit: That vnum does not exist.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, pRoom->area),
            "REdit: Insufficient security to modify room.\n\r", ch);
        char_from_room (ch);
        char_to_room (ch, pRoom);

        ch->desc->pEdit = (void *) pRoom;
        ch->desc->editor = ED_ROOM;
        return;
    }
    /* redit reset */
    if (!str_cmp (arg1, "reset")) {
        BAIL_IF (!IS_BUILDER (ch, pRoom->area),
            "REdit: Insufficient security to modify room.\n\r", ch);

        reset_room (pRoom);
        send_to_char ("Room reset.\n\r", ch);
        return;
    }
    /* redit create <vnum> */
    else if (!str_cmp (arg1, "create")) {
        value = atoi (argument);
        BAIL_IF (argument[0] == '\0' || value == 0,
            "Syntax: edit room create [vnum]\n\r", ch);

        pArea = area_get_by_inner_vnum (value);
        BAIL_IF (!pArea,
            "REdit: That vnum is not assigned an area.\n\r", ch);
        BAIL_IF (!IS_BUILDER (ch, pArea),
            "REdit: Insufficient security to create room.\n\r", ch);

        if (redit_create (ch, argument)) {
            ch->desc->editor = ED_ROOM;
            char_from_room (ch);
            char_to_room (ch, ch->desc->pEdit);
            SET_BIT (((ROOM_INDEX_DATA *) ch->desc->pEdit)->area->area_flags,
                     AREA_CHANGED);
        }
        return;
    }

    send_to_char ("REdit: There is no default room to edit.\n\r", ch);
}

DEFINE_DO_FUN (do_resets) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    char arg6[MAX_INPUT_LENGTH];
    char arg7[MAX_INPUT_LENGTH];
    RESET_DATA *pReset = NULL;
    RESET_VALUE_TYPE *v;

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
        if (ch->in_room->reset_first) {
            send_to_char ("Resets: M = mobile, R = room, O = object, "
                          "P = pet, S = shopkeeper\n\r", ch);
            do_resets_display (ch);
        }
        else
            send_to_char ("No resets in this room.\n\r", ch);
    }

    /* Take index number and search for commands. */
    if (is_number (arg1)) {
        ROOM_INDEX_DATA *pRoom = ch->in_room;

        /* Delete a reset. */
        if (!str_cmp (arg2, "delete")) {
            int insert_loc = atoi (arg1);
            BAIL_IF (!ch->in_room->reset_first,
                "No resets in this area.\n\r", ch);

            if (insert_loc - 1 <= 0) {
                pReset = pRoom->reset_first;
                pRoom->reset_first = pRoom->reset_first->next;
                if (!pRoom->reset_first)
                    pRoom->reset_last = NULL;
            }
            else {
                int iReset = 0;
                RESET_DATA *prev;

                LIST_FIND_WITH_PREV (++iReset == insert_loc, next,
                    pRoom->reset_first, pReset, prev);
                BAIL_IF (!pReset,
                    "Reset not found.\n\r", ch);
                LISTB_REMOVE_WITH_PREV (pReset, prev, next,
                    pRoom->reset_first, pRoom->reset_last);
            }

            reset_data_free (pReset);
            send_to_char ("Reset deleted.\n\r", ch);
        }
        /* Add a reset. */
        else if ((!str_cmp (arg2, "mob") && is_number (arg3))
              || (!str_cmp (arg2, "obj") && is_number (arg3)))
        {
            /* Check for Mobile reset. */
            if (!str_cmp (arg2, "mob")) {
                BAIL_IF (get_mob_index (is_number (arg3) ? atoi (arg3) : 1) == NULL,
                    "Mob doesn't exist.\n\r", ch);
                pReset = reset_data_new ();
                pReset->command = 'M';

                v = &(pReset->v);
                v->mob.mob_vnum     = atoi (arg3);
                v->mob.global_limit = is_number (arg4) ? atoi (arg4) : 1;
                v->mob.room_vnum    = ch->in_room->vnum;
                v->mob.room_limit   = is_number (arg5) ? atoi (arg5) : 1;
            }
            /* Check for Object reset. */
            else if (!str_cmp (arg2, "obj")) {
                pReset = reset_data_new ();
                v = &(pReset->v);

                /* Inside another object. */
                if (!str_prefix (arg4, "inside")) {
                    OBJ_INDEX_DATA *temp;

                    temp = get_obj_index (is_number (arg5) ? atoi (arg5) : 1);
                    BAIL_IF ((temp->item_type != ITEM_CONTAINER) &&
                             (temp->item_type != ITEM_CORPSE_NPC),
                        "Object 2 is not a container.\n\r", ch);
                    pReset->command = 'P';
                    v->put.obj_vnum     = atoi (arg3);
                    v->put.global_limit = is_number (arg6) ? atoi (arg6) : 1;
                    v->put.into_vnum    = is_number (arg5) ? atoi (arg5) : 1;
                    v->put.put_count    = is_number (arg7) ? atoi (arg7) : 1;
                }
                /* Inside the room. */
                else if (!str_cmp (arg4, "room")) {
                    BAIL_IF (get_obj_index (atoi (arg3)) == NULL,
                        "Vnum doesn't exist.\n\r", ch);
                    pReset->command = 'O';
                    v->obj.obj_vnum     = atoi (arg3);
                    v->obj.global_limit = 0;
                    v->obj.room_vnum    = ch->in_room->vnum;
                }
                /* Into a Mobile's inventory. */
                else {
                    int wear_loc;

                    BAIL_IF ((wear_loc = flag_value (wear_loc_types, arg4)) == NO_FLAG,
                        "Resets: '? wear-loc'\n\r", ch);
                    BAIL_IF (get_obj_index (atoi (arg3)) == NULL,
                        "Vnum doesn't exist.\n\r", ch);

                    if (wear_loc == WEAR_NONE) {
                        pReset->command = 'G';
                        v->give.obj_vnum     = atoi (arg3);
                        v->give.global_limit = 0;
                    }
                    else {
                        pReset->command = 'E';
                        v->equip.obj_vnum     = atoi (arg3);
                        v->equip.global_limit = 0;
                        v->equip.wear_loc     = wear_loc;
                    }
                }
            }
            redit_add_reset (ch->in_room, pReset, atoi (arg1));
            SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
            send_to_char ("Reset added.\n\r", ch);
        }
        else if (!str_cmp (arg2, "random") && is_number (arg3)) {
            BAIL_IF (atoi (arg3) < 1 || atoi (arg3) > 6,
                "Invalid argument.\n\r", ch);
            pReset = reset_data_new ();
            pReset->command = 'R';
            v->randomize.room_vnum = ch->in_room->vnum;
            v->randomize.dir_count = atoi (arg3);
            redit_add_reset (ch->in_room, pReset, atoi (arg1));
            SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
            send_to_char ("Random exits reset added.\n\r", ch);
        }
        else {
            send_to_char ("Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch);
            send_to_char ("        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r", ch);
            send_to_char ("        RESET <number> OBJ <vnum> room\n\r", ch);
            send_to_char ("        RESET <number> MOB <vnum> [max #x area] [max #x room]\n\r", ch);
            send_to_char ("        RESET <number> DELETE\n\r", ch);
            send_to_char ("        RESET <number> RANDOM [#x exits]\n\r", ch);
        }
    }
}

/*****************************************************************************
 Name:      do_alist
 Purpose:   Normal command to list areas and display area information.
 Called by: interpreter(interp.c)
 ****************************************************************************/
DEFINE_DO_FUN (do_alist) {
    char buf[MAX_STRING_LENGTH];
    char result[MAX_STRING_LENGTH * 2];    /* May need tweaking. */
    AREA_DATA *pArea;

    if (IS_NPC (ch))
        return;

    sprintf (result, "[%3s] [%-27s] (%-5s-%5s) [%-10s] %3s [%-10s]\n\r",
             "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec",
             "Builders");

    for (pArea = area_first; pArea; pArea = pArea->next) {
        sprintf (buf,
                 "[%3d] %-29.29s (%-5d-%5d) %-12.12s [%d] [%-10.10s]\n\r",
                 pArea->vnum, pArea->title, pArea->min_vnum, pArea->max_vnum,
                 pArea->filename, pArea->security, pArea->builders);
        strcat (result, buf);
    }

    send_to_char (result, ch);
}

/*****************************************************************************
 Name:      do_asave
 Purpose:   Entry point for saving area data.
 Called by: interpreter(interp.c)
 ****************************************************************************/
DEFINE_DO_FUN (do_asave) {
    char arg1[MAX_INPUT_LENGTH];
    AREA_DATA *pArea;
    int value;

    smash_tilde (argument);
    strcpy (arg1, argument);

    if (arg1[0] == '\0') {
        if (ch) {
            send_to_char ("Syntax:\n\r", ch);
            send_to_char ("  asave <vnum>   - saves a particular area\n\r", ch);
            send_to_char ("  asave list     - saves the area.lst file\n\r", ch);
            send_to_char ("  asave area     - saves the area being edited\n\r", ch);
            send_to_char ("  asave changed  - saves all changed zones\n\r", ch);
            send_to_char ("  asave world    - saves the world! (db dump)\n\r", ch);
            send_to_char ("\n\r", ch);
        }
        return;
    }

    /* Snarf the value (which need not be numeric). */
    value = atoi (arg1);
    BAIL_IF (!(pArea = area_get_by_vnum (value)) && is_number (arg1),
        "ASave: That area does not exist.\n\r", ch);

    /* Save area of given vnum. */
    /* ------------------------ */
    if (is_number (arg1)) {
        BAIL_IF (ch && !IS_BUILDER (ch, pArea),
            "ASave: You are not a builder for this area.\n\r", ch);
        save_area_list ();
        save_area (pArea);
        return;
    }

    /* Save the world, only authorized areas. */
    /* -------------------------------------- */
    if (!str_cmp ("world", arg1)) {
        save_area_list ();
        for (pArea = area_first; pArea; pArea = pArea->next) {
            /* Builder must be assigned this area. */
            if (ch && !IS_BUILDER (ch, pArea))
                continue;

            save_area (pArea);
            REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
        }
        if (ch)
            send_to_char ("You saved the world.\n\r", ch);
        save_other_helps (NULL);
        return;
    }

    /* Save changed areas, only authorized areas. */
    /* ------------------------------------------ */
    if (!str_cmp ("changed", arg1)) {
        char buf[MAX_INPUT_LENGTH];
        save_area_list ();

        if (ch)
            send_to_char ("Saved zones:\n\r", ch);
        else
            log_string ("Saved zones:");
        sprintf (buf, "None.\n\r");

        for (pArea = area_first; pArea; pArea = pArea->next) {
            /* Builder must be assigned this area. */
            if (ch && !IS_BUILDER (ch, pArea))
                continue;

            /* Save changed areas. */
            if (IS_SET (pArea->area_flags, AREA_CHANGED)) {
                save_area (pArea);
                sprintf (buf, "%24s - '%s'", pArea->title, pArea->filename);
                if (ch)
                    printf_to_char (ch, "%s\n\r", buf);
                else
                    log_string (buf);
                REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
            }
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
                pArea = (AREA_DATA *) ch->desc->pEdit;
                break;
            case ED_ROOM:
                pArea = ch->in_room->area;
                break;
            case ED_OBJECT:
                pArea = ((OBJ_INDEX_DATA *) ch->desc->pEdit)->area;
                break;
            case ED_MOBILE:
                pArea = ((MOB_INDEX_DATA *) ch->desc->pEdit)->area;
                break;
            case ED_HELP: {
                int saved;

                send_to_char ("Saving all changed helps:\n\r", ch);
                saved = save_other_helps (ch);
                if (saved == 0)
                    send_to_char ("No helps to save.\n\r", ch);
                else
                    printf_to_char (ch, "%d helps to saved.\n\r", saved);

                return;
            }
            default:
                pArea = ch->in_room->area;
                break;
        }

        BAIL_IF (!IS_BUILDER (ch, pArea),
            "ASave: You are not a builder for this area.\n\r", ch);

        save_area_list ();
        save_area (pArea);
        REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
        send_to_char ("Area saved.\n\r", ch);
        return;
    }

    /* Show correct syntax. */
    /* -------------------- */
    if (ch)
        do_asave (ch, "");
}
