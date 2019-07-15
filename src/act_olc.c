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

/* TODO: there's an outrageous amount of redundancy in here.
         reduce, and test thoroughly! */
/* TODO: BAIL_IF() everywhere */
/* TODO: this file is HUGE. split into different olc files (medit, redit, etc) */
/* TODO: move any universally-helpful functions to a different files. */
/* TODO: move sub-routines and filters to the top. */
/* TODO: rename sub-routines appropriately. */

void display_resets (CHAR_DATA * ch) {
    ROOM_INDEX_DATA *pRoom;
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMob = NULL;
    char buf[MAX_STRING_LENGTH];
    char final[MAX_STRING_LENGTH];
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

        switch (pReset->command) {
            default:
                sprintf (buf, "Bad reset command: %c.", pReset->command);
                strcat (final, buf);
                break;

            case 'M':
                if (!(pMobIndex = get_mob_index (pReset->value[1]))) {
                    sprintf (buf, "Load Mobile - Bad Mob %d\n\r", pReset->value[1]);
                    strcat (final, buf);
                    continue;
                }

                if (!(pRoomIndex = get_room_index (pReset->value[3]))) {
                    sprintf (buf, "Load Mobile - Bad Room %d\n\r", pReset->value[3]);
                    strcat (final, buf);
                    continue;
                }

                pMob = pMobIndex;
                sprintf (buf,
                         "M[%5d] %-13.13s in room             R[%5d] %2d-%2d %-15.15s\n\r",
                         pReset->value[1], pMob->short_descr, pReset->value[3],
                         pReset->value[2], pReset->value[4], pRoomIndex->name);
                strcat (final, buf);

                /* Check for pet shop. */
                {
                    ROOM_INDEX_DATA *pRoomIndexPrev;

                    pRoomIndexPrev = get_room_index (pRoomIndex->vnum - 1);
                    if (pRoomIndexPrev
                        && IS_SET (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
                        final[5] = 'P';
                }
                break;

            case 'O':
                if (!(pObjIndex = get_obj_index (pReset->value[1]))) {
                    sprintf (buf, "Load Object - Bad Object %d\n\r", pReset->value[1]);
                    strcat (final, buf);
                    continue;
                }

                pObj = pObjIndex;

                if (!(pRoomIndex = get_room_index (pReset->value[3]))) {
                    sprintf (buf, "Load Object - Bad Room %d\n\r", pReset->value[3]);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf, "O[%5d] %-13.13s in room             "
                         "R[%5d]       %-15.15s\n\r",
                         pReset->value[1], pObj->short_descr,
                         pReset->value[3], pRoomIndex->name);
                strcat (final, buf);
                break;

            case 'P':
                if (!(pObjIndex = get_obj_index (pReset->value[1]))) {
                    sprintf (buf, "Put Object - Bad Object %d\n\r", pReset->value[1]);
                    strcat (final, buf);
                    continue;
                }

                pObj = pObjIndex;
                if (!(pObjToIndex = get_obj_index (pReset->value[3]))) {
                    sprintf (buf, "Put Object - Bad To Object %d\n\r", pReset->value[3]);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf,
                         "O[%5d] %-13.13s inside              O[%5d] %2d-%2d %-15.15s\n\r",
                         pReset->value[1],
                         pObj->short_descr,
                         pReset->value[3],
                         pReset->value[2], pReset->value[4],
                         pObjToIndex->short_descr);
                strcat (final, buf);
                break;

            case 'G':
            case 'E':
                if (!(pObjIndex = get_obj_index (pReset->value[1]))) {
                    sprintf (buf, "Give/Equip Object - Bad Object %d\n\r", pReset->value[1]);
                    strcat (final, buf);
                    continue;
                }

                pObj = pObjIndex;
                if (!pMob) {
                    sprintf (buf, "Give/Equip Object - No Previous Mobile\n\r");
                    strcat (final, buf);
                    break;
                }
                if (pMob->pShop) {
                    sprintf (buf,
                             "O[%5d] %-13.13s in the inventory of S[%5d]       %-15.15s\n\r",
                             pReset->value[1],
                             pObj->short_descr, pMob->vnum,
                             pMob->short_descr);
                }
                else {
                    sprintf (buf,
                             "O[%5d] %-13.13s %-19.19s M[%5d]       %-15.15s\n\r",
                             pReset->value[1],
                             pObj->short_descr,
                             (pReset->command == 'G') ?
                             flag_string (wear_loc_phrases, WEAR_NONE)
                             : flag_string (wear_loc_phrases, pReset->value[3]),
                             pMob->vnum, pMob->short_descr);
                }
                strcat (final, buf);

                break;

                /* Doors are set in rs_flags don't need to be displayed.
                 * If you want to display them then uncomment the new_reset
                 * line in the case 'D' in load_resets in db.c and here. */
                /* ^^^ new_reset() is now room_take_reset(), but the commented
                 *     line doesn't exist anymore anyway. huh. -- Synival */
            case 'D':
                pRoomIndex = get_room_index (pReset->value[1]);
                sprintf (buf, "R[%5d] %s door of %-19.19s reset to %s\n\r",
                         pReset->value[1],
                         capitalize (door_table[pReset->value[2]].name),
                         pRoomIndex->name,
                         flag_string (door_resets, pReset->value[3]));
                strcat (final, buf);

                break;
                /* End Doors Comment. */
            case 'R':
                if (!(pRoomIndex = get_room_index (pReset->value[1]))) {
                    sprintf (buf, "Randomize Exits - Bad Room %d\n\r", pReset->value[1]);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf, "R[%5d] Exits are randomized in %s\n\r",
                         pReset->value[1], pRoomIndex->name);
                strcat (final, buf);
                break;
        }
        send_to_char (final, ch);
    }
}

/* Entry point for all editors. */
void do_olc (CHAR_DATA * ch, char *argument) {
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
    return;
}

/* Entry point for editing area_data. */
void do_aedit (CHAR_DATA * ch, char *argument) {
    AREA_DATA *pArea;
    int value;
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
        return;

    pArea = ch->in_room->area;
    argument = one_argument (argument, arg);

    if (is_number (arg)) {
        value = atoi (arg);
        if (!(pArea = area_get_by_vnum (value))) {
            send_to_char ("That area vnum does not exist.\n\r", ch);
            return;
        }
    }
    else if (!str_cmp (arg, "create")) {
        if (ch->pcdata->security < 9) {
            send_to_char ("AEdit : Insufficient security to create area.\n\r", ch);
            return;
        }
        aedit_create (ch, "");
        ch->desc->editor = ED_AREA;
        return;
    }
    if (!IS_BUILDER (ch, pArea)) {
        send_to_char ("Insufficient security to edit areas.\n\r", ch);
        return;
    }

    ch->desc->pEdit = (void *) pArea;
    ch->desc->editor = ED_AREA;
}

void do_hedit (CHAR_DATA *ch, char *argument) {
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
        /* TODO: vvv outputting to the same buffer we're reading from??? */
        argument = one_argument (arg1, arg1);
        if (!str_cmp (arg1,"new")) {
            if (argument[0] == '\0') {
                send_to_char ("Syntax: edit help new [topic]\n\r", ch);
                return;
            }
            if (hedit_new (ch, argument))
                ch->desc->editor = ED_HELP;
            return;
        }
    }
    send_to_char ("HEdit: There is no default help to edit.\n\r", ch);
}

/* Entry point for editing mob_index_data. */
void do_medit (CHAR_DATA * ch, char *argument) {
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int value;
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument (argument, arg1);

    if (IS_NPC (ch))
        return;

    if (is_number (arg1)) {
        value = atoi (arg1);
        if (!(pMob = get_mob_index (value))) {
            send_to_char ("MEdit:  That vnum does not exist.\n\r", ch);
            return;
        }
        if (!IS_BUILDER (ch, pMob->area)) {
            send_to_char ("Insufficient security to modify mobs.\n\r", ch);
            return;
        }
        ch->desc->pEdit = (void *) pMob;
        ch->desc->editor = ED_MOBILE;
        return;
    }
    else {
        if (!str_cmp (arg1, "create")) {
            value = atoi (argument);
            if (arg1[0] == '\0' || value == 0) {
                send_to_char ("Syntax:  edit mobile create [vnum]\n\r", ch);
                return;
            }

            pArea = area_get_by_inner_vnum (value);
            if (!pArea) {
                send_to_char ("OEdit:  That vnum is not assigned an area.\n\r", ch);
                return;
            }
            if (!IS_BUILDER (ch, pArea)) {
                send_to_char ("Insufficient security to modify mobs.\n\r", ch);
                return;
            }
            if (medit_create (ch, argument)) {
                SET_BIT (pArea->area_flags, AREA_CHANGED);
                ch->desc->editor = ED_MOBILE;
            }
            return;
        }
    }

    send_to_char ("MEdit:  There is no default mobile to edit.\n\r", ch);
}

void do_mpedit (CHAR_DATA * ch, char *argument) {
    MPROG_CODE *pMcode;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument (argument, command);
    if (is_number (command)) {
        int vnum = atoi (command);
        AREA_DATA *ad;

        if ((pMcode = get_mprog_index (vnum)) == NULL) {
            send_to_char ("MPEdit : That vnum does not exist.\n\r", ch);
            return;
        }

        ad = area_get_by_inner_vnum (vnum);
        if (ad == NULL) {
            send_to_char ("MPEdit : VNUM no asignado a ningun area.\n\r", ch);
            return;
        }
        if (!IS_BUILDER (ch, ad)) {
            send_to_char
                ("MPEdit : Insuficiente seguridad para editar area.\n\r", ch);
            return;
        }

        ch->desc->pEdit = (void *) pMcode;
        ch->desc->editor = ED_MPCODE;
        return;
    }

    if (!str_cmp (command, "create")) {
        if (argument[0] == '\0') {
            send_to_char ("Sintaxis : mpedit create [vnum]\n\r", ch);
            return;
        }
        mpedit_create (ch, argument);
        return;
    }

    send_to_char ("Sintaxis : mpedit [vnum]\n\r", ch);
    send_to_char ("           mpedit create [vnum]\n\r", ch);
}

/* Entry point for editing obj_index_data. */
void do_oedit (CHAR_DATA * ch, char *argument) {
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    char arg1[MAX_STRING_LENGTH];
    int value;

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);

    if (is_number (arg1)) {
        value = atoi (arg1);
        if (!(pObj = get_obj_index (value))) {
            send_to_char ("OEdit:  That vnum does not exist.\n\r", ch);
            return;
        }
        if (!IS_BUILDER (ch, pObj->area)) {
            send_to_char ("Insufficient security to modify objects.\n\r", ch);
            return;
        }
        ch->desc->pEdit = (void *) pObj;
        ch->desc->editor = ED_OBJECT;
        return;
    }
    else {
        if (!str_cmp (arg1, "create")) {
            value = atoi (argument);
            if (argument[0] == '\0' || value == 0) {
                send_to_char ("Syntax:  edit object create [vnum]\n\r", ch);
                return;
            }

            pArea = area_get_by_inner_vnum (value);
            if (!pArea) {
                send_to_char ("OEdit:  That vnum is not assigned an area.\n\r", ch);
                return;
            }
            if (!IS_BUILDER (ch, pArea)) {
                send_to_char ("Insufficient security to modify objects.\n\r", ch);
                return;
            }
            if (oedit_create (ch, argument)) {
                SET_BIT (pArea->area_flags, AREA_CHANGED);
                ch->desc->editor = ED_OBJECT;
            }
            return;
        }
    }

    send_to_char ("OEdit:  There is no default object to edit.\n\r", ch);
}

/* Entry point for editing room_index_data. */
void do_redit (CHAR_DATA * ch, char *argument) {
    ROOM_INDEX_DATA *pRoom;
    char arg1[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);
    pRoom = ch->in_room;

    if (!str_cmp (arg1, "reset")) { /* redit reset */
        if (!IS_BUILDER (ch, pRoom->area)) {
            send_to_char ("Insufficient security to modify room.\n\r", ch);
            return;
        }

        reset_room (pRoom);
        send_to_char ("Room reset.\n\r", ch);
        return;
    }
    else if (!str_cmp (arg1, "create")) { /* redit create <vnum> */
        if (argument[0] == '\0' || atoi (argument) == 0) {
            send_to_char ("Syntax:  edit room create [vnum]\n\r", ch);
            return;
        }
        if (redit_create (ch, argument)) { /* pEdit == nuevo cuarto */
            ch->desc->editor = ED_ROOM;
            char_from_room (ch);
            char_to_room (ch, ch->desc->pEdit);
            SET_BIT (((ROOM_INDEX_DATA *) ch->desc->pEdit)->area->area_flags,
                     AREA_CHANGED);
        }
        return;
    }
    else if (!IS_NULLSTR (arg1)) { /* redit <vnum> */
        pRoom = get_room_index (atoi (arg1));
        if (!pRoom) {
            send_to_char ("REdit : Nonexistant room.\n\r", ch);
            return;
        }
        if (!IS_BUILDER (ch, pRoom->area)) {
            send_to_char ("REdit : Insufficient security to modify room.\n\r", ch);
            return;
        }
        char_from_room (ch);
        char_to_room (ch, pRoom);
    }
    if (!IS_BUILDER (ch, pRoom->area)) {
        send_to_char ("REdit : Insufficient security to modify room.\n\r", ch);
        return;
    }

    ch->desc->pEdit = (void *) pRoom;
    ch->desc->editor = ED_ROOM;
}

void do_resets (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    char arg6[MAX_INPUT_LENGTH];
    char arg7[MAX_INPUT_LENGTH];
    RESET_DATA *pReset = NULL;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    argument = one_argument (argument, arg3);
    argument = one_argument (argument, arg4);
    argument = one_argument (argument, arg5);
    argument = one_argument (argument, arg6);
    argument = one_argument (argument, arg7);

    if (!IS_BUILDER (ch, ch->in_room->area)) {
        send_to_char ("Resets: Invalid security for editing this area.\n\r", ch);
        return;
    }

    /* Display resets in current room. */
    if (arg1[0] == '\0') {
        if (ch->in_room->reset_first) {
            send_to_char ("Resets: M = mobile, R = room, O = object, "
                          "P = pet, S = shopkeeper\n\r", ch);
            display_resets (ch);
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
            if (!ch->in_room->reset_first) {
                send_to_char ("No resets in this area.\n\r", ch);
                return;
            }

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
                if (!pReset) {
                    send_to_char ("Reset not found.\n\r", ch);
                    return;
                }
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
                if (get_mob_index (is_number (arg3) ? atoi (arg3) : 1) == NULL) {
                    send_to_char ("Mob doesn't exist.\n\r", ch);
                    return;
                }
                pReset = reset_data_new ();
                pReset->command = 'M';
                pReset->value[1] = atoi (arg3);
                pReset->value[2] = is_number (arg4) ? atoi (arg4) : 1;    /* Max # */
                pReset->value[3] = ch->in_room->vnum;
                pReset->value[4] = is_number (arg5) ? atoi (arg5) : 1;    /* Min # */
            }
            /* Check for Object reset. */
            else if (!str_cmp (arg2, "obj")) {
                pReset = reset_data_new ();
                pReset->value[1] = atoi (arg3);

                /* Inside another object. */
                if (!str_prefix (arg4, "inside")) {
                    OBJ_INDEX_DATA *temp;

                    temp = get_obj_index (is_number (arg5) ? atoi (arg5) : 1);
                    if ((temp->item_type != ITEM_CONTAINER) &&
                        (temp->item_type != ITEM_CORPSE_NPC))
                    {
                        send_to_char ("Object 2 is not a container.\n\r", ch);
                        return;
                    }
                    pReset->command = 'P';
                    pReset->value[2] = is_number (arg6) ? atoi (arg6) : 1;
                    pReset->value[3] = is_number (arg5) ? atoi (arg5) : 1;
                    pReset->value[4] = is_number (arg7) ? atoi (arg7) : 1;
                }
                /* Inside the room. */
                else if (!str_cmp (arg4, "room")) {
                    if (get_obj_index (atoi (arg3)) == NULL) {
                        send_to_char ("Vnum doesn't exist.\n\r", ch);
                        return;
                    }
                    pReset->command = 'O';
                    pReset->value[2] = 0;
                    pReset->value[3] = ch->in_room->vnum;
                    pReset->value[4] = 0;
                }
                /* Into a Mobile's inventory. */
                else {
                    if (flag_value (wear_loc_types, arg4) == NO_FLAG) {
                        send_to_char ("Resets: '? wear-loc'\n\r", ch);
                        return;
                    }
                    if (get_obj_index (atoi (arg3)) == NULL) {
                        send_to_char ("Vnum doesn't exist.\n\r", ch);
                        return;
                    }
                    pReset->value[1] = atoi (arg3);
                    pReset->value[3] = flag_value (wear_loc_types, arg4);
                    if (pReset->value[3] == WEAR_NONE)
                        pReset->command = 'G';
                    else
                        pReset->command = 'E';
                }
            }
            add_reset (ch->in_room, pReset, atoi (arg1));
            SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
            send_to_char ("Reset added.\n\r", ch);
        }
        else if (!str_cmp (arg2, "random") && is_number (arg3)) {
            if (atoi (arg3) < 1 || atoi (arg3) > 6) {
                send_to_char ("Invalid argument.\n\r", ch);
                return;
            }
            pReset = reset_data_new ();
            pReset->command = 'R';
            pReset->value[1] = ch->in_room->vnum;
            pReset->value[2] = atoi (arg3);
            add_reset (ch->in_room, pReset, atoi (arg1));
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
void do_alist (CHAR_DATA * ch, char *argument) {
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
void do_asave (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH];
    AREA_DATA *pArea;
    int value;

/*    {
    save_area_list();
    for( pArea = area_first; pArea; pArea = pArea->next )
    {
        save_area( pArea );
        REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
    }
    return;
    } */

    smash_tilde (argument);
    strcpy (arg1, argument);

    if (arg1[0] == '\0') {
        if (ch) {
            send_to_char ("Syntax:\n\r", ch);
            send_to_char ("  asave <vnum>   - saves a particular area\n\r",
                          ch);
            send_to_char ("  asave list     - saves the area.lst file\n\r",
                          ch);
            send_to_char
                ("  asave area     - saves the area being edited\n\r", ch);
            send_to_char ("  asave changed  - saves all changed zones\n\r",
                          ch);
            send_to_char ("  asave world    - saves the world! (db dump)\n\r",
                          ch);
            send_to_char ("\n\r", ch);
        }
        return;
    }

    /* Snarf the value (which need not be numeric). */
    value = atoi (arg1);
    if (!(pArea = area_get_by_vnum (value)) && is_number (arg1)) {
        if (ch)
            send_to_char ("That area does not exist.\n\r", ch);
        return;
    }

    /* Save area of given vnum. */
    /* ------------------------ */
    if (is_number (arg1)) {
        if (ch && !IS_BUILDER (ch, pArea)) {
            send_to_char ("You are not a builder for this area.\n\r", ch);
            return;
        }
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
                if (ch) {
                    send_to_char (buf, ch);
                    send_to_char ("\n\r", ch);
                }
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
        if (ch->desc->editor == ED_NONE) {
            send_to_char ("You are not editing an area, "
                          "therefore an area vnum is required.\n\r", ch);
            return;
        }

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
            case ED_HELP:
                send_to_char ("Grabando area : ", ch);
                save_other_helps (ch);
                return;
            default:
                pArea = ch->in_room->area;
                break;
        }

        if (!IS_BUILDER (ch, pArea)) {
            send_to_char ("You are not a builder for this area.\n\r", ch);
            return;
        }

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
