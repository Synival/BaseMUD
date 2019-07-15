/***************************************************************************
 *  File: olc_redit.c                                                      *
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

#include "comm.h"
#include "lookup.h"
#include "db.h"
#include "recycle.h"
#include "utils.h"
#include "interp.h"
#include "string.h"
#include "act_info.h"
#include "olc.h"
#include "olc_medit.h"
#include "olc_oedit.h"
#include "chars.h"
#include "objs.h"
#include "find.h"

#include "olc_redit.h"

/*****************************************************************************
 Name:       add_reset
 Purpose:    Inserts a new reset in the given index slot.
 Called by:  do_resets(olc.c).
 ****************************************************************************/
void add_reset (ROOM_INDEX_DATA * room, RESET_DATA * pReset, int index) {
    RESET_DATA *prev;
    int iReset = 0;

    /* No resets or first slot (1) selected. */
    index--;
    if (!room->reset_first || index == 0) {
        LISTB_FRONT (pReset, next,
            room->reset_first, room->reset_last);
        return;
    }

    /* If negative slot( <= 0 selected) then this will find the last. */
    for (prev = room->reset_first; prev->next; prev = prev->next)
        if (++iReset == index)
            break;

    LISTB_INSERT_AFTER (pReset, prev, next,
        room->reset_first, room->reset_last);
}

/* Local function. */
bool change_exit (CHAR_DATA * ch, char *argument, int door) {
    ROOM_INDEX_DATA *pRoom;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int value;

    EDIT_ROOM (ch, pRoom);

    /* Set the exit flags, needs full argument.
     * ---------------------------------------- */
    if ((value = flag_value (exit_flags, argument)) != NO_FLAG) {
        ROOM_INDEX_DATA *pToRoom;
        sh_int rev;                /* ROM OLC */

        if (!pRoom->exit[door]) {
            send_to_char ("Exit doesn't exist.\n\r", ch);
            return FALSE;
        }

        /* This room. */
        TOGGLE_BIT (pRoom->exit[door]->rs_flags, value);
        /* Don't toggle exit_flags because it can be changed by players. */
        pRoom->exit[door]->exit_flags = pRoom->exit[door]->rs_flags;

        /* Connected room. */
        pToRoom = pRoom->exit[door]->to_room;    /* ROM OLC */
        rev = door_table[door].reverse;

        if (pToRoom->exit[rev] != NULL) {
            pToRoom->exit[rev]->exit_flags = pRoom->exit[door]->exit_flags;
            pToRoom->exit[rev]->rs_flags   = pRoom->exit[door]->rs_flags;
        }

        send_to_char ("Exit flag toggled.\n\r", ch);
        return TRUE;
    }

    /* Now parse the arguments. */
    argument = one_argument (argument, command);
    one_argument (argument, arg);

    if (command[0] == '\0' && argument[0] == '\0') {                            /* Move command. */
        char_move (ch, door, TRUE); /* ROM OLC */
        return FALSE;
    }
    if (command[0] == '?') {
        do_help (ch, "EXIT");
        return FALSE;
    }

    if (!str_cmp (command, "delete")) {
        ROOM_INDEX_DATA *pToRoom;
        sh_int rev; /* ROM OLC */

        if (!pRoom->exit[door]) {
            send_to_char ("REdit:  Cannot delete a null exit.\n\r", ch);
            return FALSE;
        }

        /* Remove ToRoom Exit. */
        rev = door_table[door].reverse;
        pToRoom = pRoom->exit[door]->to_room; /* ROM OLC */

        if (pToRoom->exit[rev]) {
            exit_free (pToRoom->exit[rev]);
            pToRoom->exit[rev] = NULL;
        }

        /* Remove this exit. */
        exit_free (pRoom->exit[door]);
        pRoom->exit[door] = NULL;

        send_to_char ("Exit unlinked.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "link")) {
        EXIT_DATA *pExit;
        ROOM_INDEX_DATA *toRoom;

        if (arg[0] == '\0' || !is_number (arg)) {
            send_to_char ("Syntax:  [direction] link [vnum]\n\r", ch);
            return FALSE;
        }

        value = atoi (arg);
        if (!(toRoom = get_room_index (value))) {
            send_to_char ("REdit:  Cannot link to non-existant room.\n\r", ch);
            return FALSE;
        }
        if (!IS_BUILDER (ch, toRoom->area)) {
            send_to_char ("REdit:  Cannot link to that area.\n\r", ch);
            return FALSE;
        }
        if (toRoom->exit[door_table[door].reverse]) {
            send_to_char ("REdit:  Remote side's exit already exists.\n\r", ch);
            return FALSE;
        }

        if (!pRoom->exit[door])
            pRoom->exit[door] = exit_new ();

        pRoom->exit[door]->to_room = toRoom;
        pRoom->exit[door]->orig_door = door;

        door = door_table[door].reverse;
        pExit = exit_new ();
        pExit->to_room = pRoom;
        pExit->orig_door = door;
        toRoom->exit[door] = pExit;

        send_to_char ("Two-way link established.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "dig")) {
        char buf[MAX_STRING_LENGTH];
        if (arg[0] == '\0' || !is_number (arg)) {
            send_to_char ("Syntax: [direction] dig <vnum>\n\r", ch);
            return FALSE;
        }

        redit_create (ch, arg);
        sprintf (buf, "link %s", arg);
        change_exit (ch, buf, door);
        return TRUE;
    }

    if (!str_cmp (command, "room")) {
        ROOM_INDEX_DATA *toRoom;

        if (arg[0] == '\0' || !is_number (arg)) {
            send_to_char ("Syntax:  [direction] room [vnum]\n\r", ch);
            return FALSE;
        }

        value = atoi (arg);
        if (!(toRoom = get_room_index (value))) {
            send_to_char ("REdit:  Cannot link to non-existant room.\n\r", ch);
            return FALSE;
        }

        if (!pRoom->exit[door])
            pRoom->exit[door] = exit_new ();

        pRoom->exit[door]->to_room = toRoom;    /* ROM OLC */
        pRoom->exit[door]->orig_door = door;

        send_to_char ("One-way link established.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "key")) {
        OBJ_INDEX_DATA *key;
        if (arg[0] == '\0' || !is_number (arg)) {
            send_to_char ("Syntax:  [direction] key [vnum]\n\r", ch);
            return FALSE;
        }
        if (!pRoom->exit[door]) {
            send_to_char ("Exit doesn't exist.\n\r", ch);
            return FALSE;
        }

        value = atoi (arg);
        if (!(key = get_obj_index (value))) {
            send_to_char ("REdit:  Key doesn't exist.\n\r", ch);
            return FALSE;
        }
        if (key->item_type != ITEM_KEY) {
            send_to_char ("REdit:  Object is not a key.\n\r", ch);
            return FALSE;
        }
        pRoom->exit[door]->key = value;

        send_to_char ("Exit key set.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "name")) {
        if (arg[0] == '\0') {
            send_to_char ("Syntax:  [direction] name [string]\n\r", ch);
            send_to_char ("         [direction] name none\n\r", ch);
            return FALSE;
        }
        if (!pRoom->exit[door]) {
            send_to_char ("Exit doesn't exist.\n\r", ch);
            return FALSE;
        }
        str_free (pRoom->exit[door]->keyword);

        if (str_cmp (arg, "none"))
            pRoom->exit[door]->keyword = str_dup (arg);
        else
            pRoom->exit[door]->keyword = str_dup ("");

        send_to_char ("Exit name set.\n\r", ch);
        return TRUE;
    }

    if (!str_prefix (command, "description")) {
        if (arg[0] == '\0') {
            if (!pRoom->exit[door]) {
                send_to_char ("Exit doesn't exist.\n\r", ch);
                return FALSE;
            }
            string_append (ch, &pRoom->exit[door]->description);
            return TRUE;
        }
        send_to_char ("Syntax:  [direction] desc\n\r", ch);
        return FALSE;
    }

    return FALSE;
}

REDIT (redit_rlist) {
    ROOM_INDEX_DATA *pRoomIndex;
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buf1;
    char arg[MAX_INPUT_LENGTH];
    bool found;
    int vnum;
    int col = 0;

    one_argument (argument, arg);

    pArea = ch->in_room->area;
    buf1 = buf_new ();
/*    buf1[0] = '\0'; */
    found = FALSE;

    for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
        if ((pRoomIndex = get_room_index (vnum))) {
            found = TRUE;
            sprintf (buf, "[%5d] %-17.16s",
                     vnum, capitalize (pRoomIndex->name));
            add_buf (buf1, buf);
            if (++col % 3 == 0)
                add_buf (buf1, "\n\r");
        }
    }

    if (!found) {
        send_to_char ("Room(s) not found in this area.\n\r", ch);
        return FALSE;
    }

    if (col % 3 != 0)
        add_buf (buf1, "\n\r");

    page_to_char (buf_string (buf1), ch);
    buf_free (buf1);
    return FALSE;
}

REDIT (redit_mlist) {
    MOB_INDEX_DATA *pMobIndex;
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buf1;
    char arg[MAX_INPUT_LENGTH];
    bool fAll, found;
    int vnum;
    int col = 0;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Syntax:  mlist <all/name>\n\r", ch);
        return FALSE;
    }

    buf1 = buf_new ();
    pArea = ch->in_room->area;
/*    buf1[0] = '\0'; */
    fAll = !str_cmp (arg, "all");
    found = FALSE;

    for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
        if ((pMobIndex = get_mob_index (vnum)) != NULL) {
            if (fAll || is_name (arg, pMobIndex->name)) {
                found = TRUE;
                sprintf (buf, "[%5d] %-17.16s",
                         pMobIndex->vnum,
                         capitalize (pMobIndex->short_descr));
                add_buf (buf1, buf);
                if (++col % 3 == 0)
                    add_buf (buf1, "\n\r");
            }
        }
    }

    if (!found) {
        send_to_char ("Mobile(s) not found in this area.\n\r", ch);
        return FALSE;
    }

    if (col % 3 != 0)
        add_buf (buf1, "\n\r");

    page_to_char (buf_string (buf1), ch);
    buf_free (buf1);
    return FALSE;
}

REDIT (redit_olist) {
    OBJ_INDEX_DATA *pObjIndex;
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buf1;
    char arg[MAX_INPUT_LENGTH];
    bool fAll, found;
    int vnum, item_type;
    int col = 0;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Syntax:  olist <all/name/item_type>\n\r", ch);
        return FALSE;
    }

    pArea = ch->in_room->area;
    buf1 = buf_new ();
/*  buf1[0] = '\0'; */
    fAll = !str_cmp (arg, "all");
    found = FALSE;

    item_type = item_lookup (arg);
    for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
        if ((pObjIndex = get_obj_index (vnum))) {
            if (fAll || is_name (arg, pObjIndex->name) || item_type == pObjIndex->item_type) {
                found = TRUE;
                sprintf (buf, "[%5d] %-17.16s",
                         pObjIndex->vnum,
                         capitalize (pObjIndex->short_descr));
                add_buf (buf1, buf);
                if (++col % 3 == 0)
                    add_buf (buf1, "\n\r");
            }
        }
    }
    if (!found) {
        send_to_char ("Object(s) not found in this area.\n\r", ch);
        return FALSE;
    }
    if (col % 3 != 0)
        add_buf (buf1, "\n\r");

    page_to_char (buf_string (buf1), ch);
    buf_free (buf1);
    return FALSE;
}

REDIT (redit_mshow) {
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  mshow <vnum>\n\r", ch);
        return FALSE;
    }
    if (!is_number (argument)) {
        send_to_char ("REdit: Must be a number.\n\r", ch);
        return FALSE;
    }

    if (is_number (argument)) {
        value = atoi (argument);
        if (!(pMob = get_mob_index (value))) {
            send_to_char ("REdit:  That mobile does not exist.\n\r", ch);
            return FALSE;
        }
        ch->desc->pEdit = (void *) pMob;
    }

    medit_show (ch, argument);
    ch->desc->pEdit = (void *) ch->in_room;
    return FALSE;
}

REDIT (redit_oshow) {
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  oshow <vnum>\n\r", ch);
        return FALSE;
    }
    if (!is_number (argument)) {
        send_to_char ("REdit: Must be a number.\n\r", ch);
        return FALSE;
    }
    if (is_number (argument)) {
        value = atoi (argument);
        if (!(pObj = get_obj_index (value))) {
            send_to_char ("REdit:  That object does not exist.\n\r", ch);
            return FALSE;
        }
        ch->desc->pEdit = (void *) pObj;
    }

    oedit_show (ch, argument);
    ch->desc->pEdit = (void *) ch->in_room;
    return FALSE;
}

/* Room Editor Functions. */
REDIT (redit_show) {
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH];
    char buf1[2 * MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;
    bool fcnt;

    EDIT_ROOM (ch, pRoom);

    buf1[0] = '\0';

    sprintf (buf, "Description:\n\r%s", pRoom->description);
    strcat (buf1, buf);

    sprintf (buf, "Name:       [%s]\n\rArea:       [%5d] %s\n\r",
             pRoom->name, pRoom->area->vnum, pRoom->area->title);
    strcat (buf1, buf);

    sprintf (buf, "Vnum:       [%5d]\n\rSector:     [%s]\n\r",
             pRoom->vnum, sector_get_name (pRoom->sector_type));
    strcat (buf1, buf);

    sprintf (buf, "Room flags: [%s]\n\r",
             flag_string (room_flags, pRoom->room_flags));
    strcat (buf1, buf);

    if (pRoom->heal_rate != 100 || pRoom->mana_rate != 100) {
        sprintf (buf, "Health rec: [%d]\n\rMana rec  : [%d]\n\r",
                 pRoom->heal_rate, pRoom->mana_rate);
        strcat (buf1, buf);
    }
    if (pRoom->clan > 0) {
        sprintf (buf, "Clan      : [%d] %s\n\r",
                 pRoom->clan, clan_table[pRoom->clan].name);
        strcat (buf1, buf);
    }
    if (!IS_NULLSTR (pRoom->owner)) {
        sprintf (buf, "Owner     : [%s]\n\r", pRoom->owner);
        strcat (buf1, buf);
    }
    if (pRoom->extra_descr) {
        EXTRA_DESCR_DATA *ed;

        strcat (buf1, "Desc Kwds:  [");
        for (ed = pRoom->extra_descr; ed; ed = ed->next) {
            strcat (buf1, ed->keyword);
            if (ed->next)
                strcat (buf1, " ");
        }
        strcat (buf1, "]\n\r");
    }

    strcat (buf1, "Characters: [");
    fcnt = FALSE;
    for (rch = pRoom->people; rch; rch = rch->next_in_room) {
        one_argument (rch->name, buf);
        strcat (buf1, buf);
        strcat (buf1, " ");
        fcnt = TRUE;
    }
    if (fcnt) {
        int end = strlen (buf1) - 1;
        buf1[end] = ']';
        strcat (buf1, "\n\r");
    }
    else
        strcat (buf1, "none]\n\r");

    strcat (buf1, "Objects:    [");
    fcnt = FALSE;
    for (obj = pRoom->contents; obj; obj = obj->next_content) {
        one_argument (obj->name, buf);
        strcat (buf1, buf);
        strcat (buf1, " ");
        fcnt = TRUE;
    }
    if (fcnt) {
        int end = strlen (buf1) - 1;
        buf1[end] = ']';
        strcat (buf1, "\n\r");
    }
    else
        strcat (buf1, "none]\n\r");

    for (door = 0; door < DIR_MAX; door++) {
        EXIT_DATA *pexit;
        if ((pexit = pRoom->exit[door])) {
            char word[MAX_INPUT_LENGTH];
            char reset_state[MAX_STRING_LENGTH];
            char *state;
            int i, length;

            sprintf (buf, "-%-5s to [%5d] Key: [%5d] ",
                     capitalize (door_table[door].name),
                     pexit->to_room ? pexit->to_room->vnum : 0, /* ROM OLC */
                     pexit->key);
            strcat (buf1, buf);

            /* Format up the exit info.
             * Capitalize all flags that are not part of the reset info. */
            strcpy (reset_state, flag_string (exit_flags, pexit->rs_flags));
            state = flag_string (exit_flags, pexit->exit_flags);
            strcat (buf1, " Exit flags: [");
            while (1) {
                state = one_argument (state, word);
                if (word[0] == '\0') {
                    int end = strlen (buf1) - 1;
                    buf1[end] = ']';
                    strcat (buf1, "\n\r");
                    break;
                }
                if (str_infix (word, reset_state)) {
                    length = strlen (word);
                    for (i = 0; i < length; i++)
                        word[i] = UPPER (word[i]);
                }
                strcat (buf1, word);
                strcat (buf1, " ");
            }

            if (pexit->keyword && pexit->keyword[0] != '\0') {
                sprintf (buf, "Kwds: [%s]\n\r", pexit->keyword);
                strcat (buf1, buf);
            }
            if (pexit->description && pexit->description[0] != '\0') {
                sprintf (buf, "%s", pexit->description);
                strcat (buf1, buf);
            }
        }
    }

    send_to_char (buf1, ch);
    return FALSE;
}

REDIT (redit_north)
    { return (change_exit (ch, argument, DIR_NORTH)) ? TRUE : FALSE; }
REDIT (redit_south)
    { return (change_exit (ch, argument, DIR_SOUTH)) ? TRUE : FALSE; }
REDIT (redit_east)
    { return (change_exit (ch, argument, DIR_EAST)) ? TRUE : FALSE; }
REDIT (redit_west)
    { return (change_exit (ch, argument, DIR_WEST)) ? TRUE : FALSE; }
REDIT (redit_up)
    { return (change_exit (ch, argument, DIR_UP)) ? TRUE : FALSE; }
REDIT (redit_down)
    { return (change_exit (ch, argument, DIR_DOWN)) ? TRUE : FALSE; }

REDIT (redit_ed) {
    ROOM_INDEX_DATA *pRoom;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_ROOM (ch, pRoom);

    argument = one_argument (argument, command);
    one_argument (argument, keyword);

    if (command[0] == '\0' || keyword[0] == '\0') {
        send_to_char ("Syntax:  ed add [keyword]\n\r", ch);
        send_to_char ("         ed edit [keyword]\n\r", ch);
        send_to_char ("         ed delete [keyword]\n\r", ch);
        send_to_char ("         ed format [keyword]\n\r", ch);
        return FALSE;
    }

    if (!str_cmp (command, "add")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax:  ed add [keyword]\n\r", ch);
            return FALSE;
        }
        ed = extra_descr_new ();
        ed->keyword = str_dup (keyword);
        ed->description = str_dup ("");
        LIST_FRONT (ed, next, pRoom->extra_descr);

        string_append (ch, &ed->description);
        return TRUE;
    }

    if (!str_cmp (command, "edit")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax:  ed edit [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND (is_name (keyword, ed->keyword), next,
            pRoom->extra_descr, ed);
        if (!ed) {
            send_to_char ("REdit:  Extra description keyword not found.\n\r", ch);
            return FALSE;
        }
        string_append (ch, &ed->description);
        return TRUE;
    }

    if (!str_cmp (command, "delete")) {
        EXTRA_DESCR_DATA *ped;
        if (keyword[0] == '\0') {
            send_to_char ("Syntax:  ed delete [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND_WITH_PREV (is_name (keyword, ed->keyword),
            next, pRoom->extra_descr, ed, ped);
        if (!ed) {
            send_to_char ("REdit:  Extra description keyword not found.\n\r", ch);
            return FALSE;
        }
        LIST_REMOVE_WITH_PREV (ed, ped, next, pRoom->extra_descr);

        extra_descr_free (ed);
        send_to_char ("Extra description deleted.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "format")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax:  ed format [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND (is_name (keyword, ed->keyword), next,
            pRoom->extra_descr, ed);
        if (!ed) {
            send_to_char ("REdit:  Extra description keyword not found.\n\r", ch);
            return FALSE;
        }
        ed->description = format_string (ed->description);
        send_to_char ("Extra description formatted.\n\r", ch);
        return TRUE;
    }

    redit_ed (ch, "");
    return FALSE;
}

REDIT (redit_create) {
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int value;
    int iHash;

    EDIT_ROOM (ch, pRoom);

    value = atoi (argument);
    if (argument[0] == '\0' || value <= 0) {
        send_to_char ("Syntax:  create [vnum > 0]\n\r", ch);
        return FALSE;
    }

    pArea = area_get_by_inner_vnum (value);
    if (!pArea) {
        send_to_char ("REdit:  That vnum is not assigned an area.\n\r", ch);
        return FALSE;
    }
    if (!IS_BUILDER (ch, pArea)) {
        send_to_char ("REdit:  Vnum in an area you cannot build in.\n\r", ch);
        return FALSE;
    }
    if (get_room_index (value)) {
        send_to_char ("REdit:  Room vnum already exists.\n\r", ch);
        return FALSE;
    }

    pRoom = room_index_new ();
    pRoom->area = pArea;
    pRoom->vnum = value;
    pRoom->anum = value - pArea->min_vnum;

    if (value > top_vnum_room)
        top_vnum_room = value;

    iHash = value % MAX_KEY_HASH;
    LIST_FRONT (pRoom, next, room_index_hash[iHash]);
    ch->desc->pEdit = (void *) pRoom;

    send_to_char ("Room created.\n\r", ch);
    return TRUE;
}

REDIT (redit_name) {
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM (ch, pRoom);

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  name [name]\n\r", ch);
        return FALSE;
    }

    str_free (pRoom->name);
    pRoom->name = str_dup (argument);

    send_to_char ("Name set.\n\r", ch);
    return TRUE;
}

REDIT (redit_desc) {
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM (ch, pRoom);

    if (argument[0] == '\0') {
        string_append (ch, &pRoom->description);
        return TRUE;
    }

    send_to_char ("Syntax:  desc\n\r", ch);
    return FALSE;
}

REDIT (redit_heal) {
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM (ch, pRoom);
    if (is_number (argument)) {
        pRoom->heal_rate = atoi (argument);
        send_to_char ("Heal rate set.\n\r", ch);
        return TRUE;
    }

    send_to_char ("Syntax: heal <#xnumber>\n\r", ch);
    return FALSE;
}

REDIT (redit_mana) {
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM (ch, pRoom);
    if (is_number (argument)) {
        pRoom->mana_rate = atoi (argument);
        send_to_char ("Mana rate set.\n\r", ch);
        return TRUE;
    }

    send_to_char ("Syntax: mana <#xnumber>\n\r", ch);
    return FALSE;
}

REDIT (redit_clan) {
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM (ch, pRoom);
    pRoom->clan = clan_lookup (argument);

    send_to_char ("Clan set.\n\r", ch);
    return TRUE;
}

REDIT (redit_format) {
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM (ch, pRoom);
    pRoom->description = format_string (pRoom->description);

    send_to_char ("String formatted.\n\r", ch);
    return TRUE;
}

REDIT (redit_mreset) {
    ROOM_INDEX_DATA *pRoom;
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *newmob;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    RESET_DATA *pReset;
    char output[MAX_STRING_LENGTH];

    EDIT_ROOM (ch, pRoom);

    argument = one_argument (argument, arg);
    argument = one_argument (argument, arg2);

    if (arg[0] == '\0' || !is_number (arg)) {
        send_to_char ("Syntax:  mreset <vnum> <max #x> <mix #x>\n\r", ch);
        return FALSE;
    }
    if (!(pMobIndex = get_mob_index (atoi (arg)))) {
        send_to_char ("REdit: No mobile has that vnum.\n\r", ch);
        return FALSE;
    }
    if (pMobIndex->area != pRoom->area) {
        send_to_char ("REdit: No such mobile in this area.\n\r", ch);
        return FALSE;
    }

    /* Create the mobile reset. */
    pReset = reset_data_new ();
    pReset->command = 'M';
    pReset->value[1] = pMobIndex->vnum;
    pReset->value[2] = is_number (arg2) ? atoi (arg2) : MAX_MOB;
    pReset->value[3] = pRoom->vnum;
    pReset->value[4] = is_number (argument) ? atoi (argument) : 1;
    add_reset (pRoom, pReset, 0 /* Last slot */ );

    /* Create the mobile. */
    newmob = create_mobile (pMobIndex);
    char_to_room (newmob, pRoom);

    sprintf (output, "%s (%d) has been loaded and added to resets.\n\r"
             "There will be a maximum of %d loaded to this room.\n\r",
             capitalize (pMobIndex->short_descr),
             pMobIndex->vnum, pReset->value[2]);
    send_to_char (output, ch);
    act ("$n has created $N!", ch, NULL, newmob, TO_NOTCHAR);
    return TRUE;
}

REDIT (redit_oreset) {
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *newobj;
    OBJ_DATA *to_obj;
    CHAR_DATA *to_mob;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int olevel = 0;

    RESET_DATA *pReset;
    char output[MAX_STRING_LENGTH];

    EDIT_ROOM (ch, pRoom);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || !is_number (arg1)) {
        send_to_char ("Syntax:  oreset <vnum> <args>\n\r", ch);
        send_to_char ("        -no_args               = into room\n\r", ch);
        send_to_char ("        -<obj_name>            = into obj\n\r", ch);
        send_to_char ("        -<mob_name> <wear_loc> = into mob\n\r", ch);
        return FALSE;
    }
    if (!(pObjIndex = get_obj_index (atoi (arg1)))) {
        send_to_char ("REdit: No object has that vnum.\n\r", ch);
        return FALSE;
    }
    if (pObjIndex->area != pRoom->area) {
        send_to_char ("REdit: No such object in this area.\n\r", ch);
        return FALSE;
    }

    /* Load into room. */
    if (arg2[0] == '\0') {
        pReset = reset_data_new ();
        pReset->command = 'O';
        pReset->value[1] = pObjIndex->vnum;
        pReset->value[2] = 0;
        pReset->value[3] = pRoom->vnum;
        pReset->value[4] = 0;
        add_reset (pRoom, pReset, 0 /* Last slot */ );

        newobj = create_object (pObjIndex, number_fuzzy (olevel));
        obj_to_room (newobj, pRoom);

        sprintf (output, "%s (%d) has been loaded and added to resets.\n\r",
                 capitalize (pObjIndex->short_descr), pObjIndex->vnum);
        send_to_char (output, ch);
    }
    /* Load into object's inventory. */
    else if (argument[0] == '\0' &&
        ((to_obj = find_obj_list (ch, pRoom->contents, arg2)) != NULL))
    {
        pReset = reset_data_new ();
        pReset->command = 'P';
        pReset->value[1] = pObjIndex->vnum;
        pReset->value[2] = 0;
        pReset->value[3] = to_obj->pIndexData->vnum;
        pReset->value[4] = 1;
        add_reset (pRoom, pReset, 0 /* Last slot */ );

        newobj = create_object (pObjIndex, number_fuzzy (olevel));
        newobj->cost = 0;
        obj_to_obj (newobj, to_obj);

        sprintf (output, "%s (%d) has been loaded into "
            "%s (%d) and added to resets.\n\r",
            capitalize (newobj->short_descr), newobj->pIndexData->vnum,
            to_obj->short_descr, to_obj->pIndexData->vnum);
        send_to_char (output, ch);
    }
    /* Load into mobile's inventory. */
    else if ((to_mob = find_char_room (ch, arg2)) != NULL) {
        int wear_loc;

        /* Make sure the location on mobile is valid.  */
        if ((wear_loc = flag_value (wear_loc_types, argument)) == NO_FLAG) {
            send_to_char ("REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch);
            return FALSE;
        }

        /* Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD. */
        if (!IS_SET (pObjIndex->wear_flags, wear_get_type_by_loc (wear_loc))) {
            sprintf (output,
                "%s (%d) has wear flags: [%s]\n\r",
                capitalize (pObjIndex->short_descr), pObjIndex->vnum,
                flag_string (wear_flags, pObjIndex->wear_flags));
            send_to_char (output, ch);
            return FALSE;
        }

        /* Can't load into same position.  */
        if (char_get_eq_by_wear (to_mob, wear_loc)) {
            send_to_char ("REdit:  Object already equipped.\n\r", ch);
            return FALSE;
        }

        pReset = reset_data_new ();
        pReset->value[1] = pObjIndex->vnum;
        pReset->value[2] = wear_loc;
        if (pReset->value[2] == WEAR_NONE)
            pReset->command = 'G';
        else
            pReset->command = 'E';
        pReset->value[3] = wear_loc;

        add_reset (pRoom, pReset, 0 /* Last slot */ );

        olevel = URANGE (0, to_mob->level - 2, LEVEL_HERO);
        newobj = create_object (pObjIndex, number_fuzzy (olevel));

        if (to_mob->pIndexData->pShop) { /* Shop-keeper? */
            switch (pObjIndex->item_type) {
                default:
                    olevel = 0;
                    break;
                case ITEM_PILL:
                    olevel = number_range (0, 10);
                    break;
                case ITEM_POTION:
                    olevel = number_range (0, 10);
                    break;
                case ITEM_SCROLL:
                    olevel = number_range (5, 15);
                    break;
                case ITEM_WAND:
                    olevel = number_range (10, 20);
                    break;
                case ITEM_STAFF:
                    olevel = number_range (15, 25);
                    break;
                case ITEM_ARMOR:
                    olevel = number_range (5, 15);
                    break;
                case ITEM_WEAPON:
                    if (pReset->command == 'G')
                        olevel = number_range (5, 15);
                    else
                        olevel = number_fuzzy (olevel);
                    break;
            }

            newobj = create_object (pObjIndex, olevel);
            if (pReset->value[2] == WEAR_NONE)
                SET_BIT (newobj->extra_flags, ITEM_INVENTORY);
        }
        else
            newobj = create_object (pObjIndex, number_fuzzy (olevel));

        obj_to_char (newobj, to_mob);
        if (pReset->command == 'E')
            char_equip (to_mob, newobj, pReset->value[3]);

        sprintf (output, "%s (%d) has been loaded "
                 "%s of %s (%d) and added to resets.\n\r",
                 capitalize (pObjIndex->short_descr),
                 pObjIndex->vnum,
                 flag_string (wear_loc_phrases, pReset->value[3]),
                 to_mob->short_descr, to_mob->pIndexData->vnum);
        send_to_char (output, ch);
    }
    else { /* Display Syntax */
        send_to_char ("REdit:  That mobile isn't here.\n\r", ch);
        return FALSE;
    }

    act ("$n has created $p!", ch, newobj, NULL, TO_NOTCHAR);
    return TRUE;
}

REDIT (redit_owner) {
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM (ch, pRoom);

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  owner [owner]\n\r", ch);
        send_to_char ("         owner none\n\r", ch);
        return FALSE;
    }

    str_free (pRoom->owner);
    if (!str_cmp (argument, "none"))
        pRoom->owner = str_dup ("");
    else
        pRoom->owner = str_dup (argument);

    send_to_char ("Owner set.\n\r", ch);
    return TRUE;
}

REDIT (redit_room) {
    ROOM_INDEX_DATA *room;
    int value;

    EDIT_ROOM (ch, room);
    if ((value = flag_value (room_flags, argument)) == NO_FLAG) {
        send_to_char ("Sintaxis: room [flags]\n\r", ch);
        return FALSE;
    }

    TOGGLE_BIT (room->room_flags, value);
    send_to_char ("Room flags toggled.\n\r", ch);
    return TRUE;
}

REDIT (redit_sector) {
    ROOM_INDEX_DATA *room;
    int value;

    EDIT_ROOM (ch, room);
    if ((value = sector_lookup (argument)) < 0) {
        send_to_char ("Sintaxis: sector [tipo]\n\r", ch);
        return FALSE;
    }

    room->sector_type = value;
    send_to_char ("Sector type set.\n\r", ch);
    return TRUE;
}
