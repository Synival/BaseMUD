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
#include "chars.h"
#include "objs.h"
#include "find.h"
#include "globals.h"
#include "olc.h"
#include "olc_medit.h"
#include "olc_oedit.h"
#include "memory.h"

#include "olc_redit.h"

/*****************************************************************************
 Name:       redit_add_reset
 Purpose:    Inserts a new reset in the given index slot.
 Called by:  do_resets(olc.c).
 ****************************************************************************/
void redit_add_reset (ROOM_INDEX_T *room, RESET_T *reset, int index) {
    RESET_T *prev;
    int reset_n = 0;

    /* No resets or first slot (1) selected. */
    index--;
    if (!room->reset_first || index == 0) {
        LISTB_FRONT (reset, next,
            room->reset_first, room->reset_last);
        return;
    }

    /* If negative slot( <= 0 selected) then this will find the last. */
    for (prev = room->reset_first; prev->next; prev = prev->next)
        if (++reset_n == index)
            break;

    LISTB_INSERT_AFTER (reset, prev, next,
        room->reset_first, room->reset_last);
}

/* Local function. */
bool redit_change_exit (CHAR_T *ch, char *argument, int door) {
    ROOM_INDEX_T *room;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int value;

    EDIT_ROOM (ch, room);

    /* Set the exit flags, needs full argument.
     * ---------------------------------------- */
    if ((value = flags_from_string (exit_flags, argument)) != FLAG_NONE) {
        ROOM_INDEX_T *to_room;
        sh_int rev;                /* ROM OLC */

        if (!room->exit[door]) {
            send_to_char ("Exit doesn't exist.\n\r", ch);
            return FALSE;
        }

        /* This room. */
        TOGGLE_BIT (room->exit[door]->rs_flags, value);
        /* Don't toggle exit_flags because it can be changed by players. */
        room->exit[door]->exit_flags = room->exit[door]->rs_flags;

        /* Connected room. */
        to_room = room->exit[door]->to_room;    /* ROM OLC */
        rev = door_table[door].reverse;

        if (to_room->exit[rev] != NULL) {
            to_room->exit[rev]->exit_flags = room->exit[door]->exit_flags;
            to_room->exit[rev]->rs_flags   = room->exit[door]->rs_flags;
        }

        send_to_char ("Exit flag toggled.\n\r", ch);
        return TRUE;
    }

    /* Now parse the arguments. */
    argument = one_argument (argument, command);
    one_argument (argument, arg);

    /* Move command. */
    if (command[0] == '\0' && argument[0] == '\0') {
        char_move (ch, door, TRUE); /* ROM OLC */
        return FALSE;
    }
    if (command[0] == '?') {
        do_help (ch, "EXIT");
        return FALSE;
    }

    if (!str_cmp (command, "delete")) {
        ROOM_INDEX_T *to_room;
        sh_int rev; /* ROM OLC */

        if (!room->exit[door]) {
            send_to_char ("REdit: Cannot delete a null exit.\n\r", ch);
            return FALSE;
        }

        /* Remove ToRoom Exit. */
        rev = door_table[door].reverse;
        to_room = room->exit[door]->to_room; /* ROM OLC */

        if (to_room->exit[rev]) {
            exit_free (to_room->exit[rev]);
            to_room->exit[rev] = NULL;
        }

        /* Remove this exit. */
        exit_free (room->exit[door]);
        room->exit[door] = NULL;

        send_to_char ("Exit unlinked.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "link")) {
        EXIT_T *ex;
        ROOM_INDEX_T *toRoom;

        if (arg[0] == '\0' || !is_number (arg)) {
            send_to_char ("Syntax: [direction] link [vnum]\n\r", ch);
            return FALSE;
        }

        value = atoi (arg);
        if (!(toRoom = get_room_index (value))) {
            send_to_char ("REdit: Cannot link to non-existant room.\n\r", ch);
            return FALSE;
        }
        if (!IS_BUILDER (ch, toRoom->area)) {
            send_to_char ("REdit: Cannot link to that area.\n\r", ch);
            return FALSE;
        }
        if (toRoom->exit[door_table[door].reverse]) {
            send_to_char ("REdit: Remote side's exit already exists.\n\r", ch);
            return FALSE;
        }

        if (!room->exit[door])
            room->exit[door] = exit_new ();

        room->exit[door]->to_room = toRoom;
        room->exit[door]->orig_door = door;

        door = door_table[door].reverse;
        ex = exit_new ();
        ex->to_room = room;
        ex->orig_door = door;
        toRoom->exit[door] = ex;

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
        redit_change_exit (ch, buf, door);
        return TRUE;
    }

    if (!str_cmp (command, "room")) {
        ROOM_INDEX_T *toRoom;

        if (arg[0] == '\0' || !is_number (arg)) {
            send_to_char ("Syntax: [direction] room [vnum]\n\r", ch);
            return FALSE;
        }

        value = atoi (arg);
        if (!(toRoom = get_room_index (value))) {
            send_to_char ("REdit: Cannot link to non-existant room.\n\r", ch);
            return FALSE;
        }

        if (!room->exit[door])
            room->exit[door] = exit_new ();

        room->exit[door]->to_room = toRoom;    /* ROM OLC */
        room->exit[door]->orig_door = door;

        send_to_char ("One-way link established.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "key")) {
        OBJ_INDEX_T *key;
        if (arg[0] == '\0' || !is_number (arg)) {
            send_to_char ("Syntax: [direction] key [vnum]\n\r", ch);
            return FALSE;
        }
        if (!room->exit[door]) {
            send_to_char ("Exit doesn't exist.\n\r", ch);
            return FALSE;
        }

        value = atoi (arg);
        if (!(key = get_obj_index (value))) {
            send_to_char ("REdit: Key doesn't exist.\n\r", ch);
            return FALSE;
        }
        if (key->item_type != ITEM_KEY) {
            send_to_char ("REdit: Object is not a key.\n\r", ch);
            return FALSE;
        }
        room->exit[door]->key = value;

        send_to_char ("Exit key set.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "name")) {
        if (arg[0] == '\0') {
            send_to_char ("Syntax: [direction] name [string]\n\r", ch);
            send_to_char ("        [direction] name none\n\r", ch);
            return FALSE;
        }
        if (!room->exit[door]) {
            send_to_char ("Exit doesn't exist.\n\r", ch);
            return FALSE;
        }

        str_free (&(room->exit[door]->keyword));
        if (str_cmp (arg, "none"))
            room->exit[door]->keyword = str_dup (arg);
        else
            room->exit[door]->keyword = str_dup ("");

        send_to_char ("Exit name set.\n\r", ch);
        return TRUE;
    }

    if (!str_prefix (command, "description")) {
        if (arg[0] == '\0') {
            if (!room->exit[door]) {
                send_to_char ("Exit doesn't exist.\n\r", ch);
                return FALSE;
            }
            string_append (ch, &room->exit[door]->description);
            return TRUE;
        }
        send_to_char ("Syntax: [direction] desc\n\r", ch);
        return FALSE;
    }

    return FALSE;
}

REDIT (redit_rlist) {
    ROOM_INDEX_T *room_index;
    AREA_T *area;
    char buf[MAX_STRING_LENGTH];
    BUFFER_T *buf1;
    char arg[MAX_INPUT_LENGTH];
    bool found;
    int vnum;
    int col = 0;

    one_argument (argument, arg);

    area = ch->in_room->area;
    buf1 = buf_new ();
/*    buf1[0] = '\0'; */
    found = FALSE;

    for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++) {
        if ((room_index = get_room_index (vnum))) {
            found = TRUE;
            sprintf (buf, "[%5d] %-17.16s",
                     vnum, str_capitalized (room_index->name));
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
    MOB_INDEX_T *mob_index;
    AREA_T *area;
    char buf[MAX_STRING_LENGTH];
    BUFFER_T *buf1;
    char arg[MAX_INPUT_LENGTH];
    bool all, found;
    int vnum;
    int col = 0;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Syntax: mlist <all/name>\n\r", ch);
        return FALSE;
    }

    buf1 = buf_new ();
    area = ch->in_room->area;
/*    buf1[0] = '\0'; */
    all = !str_cmp (arg, "all");
    found = FALSE;

    for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++) {
        if ((mob_index = get_mob_index (vnum)) != NULL) {
            if (all || str_in_namelist (arg, mob_index->name)) {
                found = TRUE;
                sprintf (buf, "[%5d] %-17.16s",
                         mob_index->vnum,
                         str_capitalized (mob_index->short_descr));
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
    OBJ_INDEX_T *obj_index;
    AREA_T *area;
    char buf[MAX_STRING_LENGTH];
    BUFFER_T *buf1;
    char arg[MAX_INPUT_LENGTH];
    bool all, found;
    int vnum, item_type;
    int col = 0;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Syntax: olist <all/name/item_type>\n\r", ch);
        return FALSE;
    }

    area = ch->in_room->area;
    buf1 = buf_new ();
/*  buf1[0] = '\0'; */
    all = !str_cmp (arg, "all");
    found = FALSE;

    item_type = item_lookup (arg);
    for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++) {
        if ((obj_index = get_obj_index (vnum))) {
            if (all || str_in_namelist (arg, obj_index->name) ||
                item_type == obj_index->item_type)
            {
                found = TRUE;
                sprintf (buf, "[%5d] %-17.16s",
                         obj_index->vnum,
                         str_capitalized (obj_index->short_descr));
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
    MOB_INDEX_T *mob;
    int value;

    if (argument[0] == '\0') {
        send_to_char ("Syntax: mshow <vnum>\n\r", ch);
        return FALSE;
    }
    if (!is_number (argument)) {
        send_to_char ("REdit: Must be a number.\n\r", ch);
        return FALSE;
    }

    if (is_number (argument)) {
        value = atoi (argument);
        if (!(mob = get_mob_index (value))) {
            send_to_char ("REdit: That mobile does not exist.\n\r", ch);
            return FALSE;
        }
        ch->desc->olc_edit = (void *) mob;
    }

    medit_show (ch, argument);
    ch->desc->olc_edit = (void *) ch->in_room;
    return FALSE;
}

REDIT (redit_oshow) {
    OBJ_INDEX_T *obj;
    int value;

    if (argument[0] == '\0') {
        send_to_char ("Syntax: oshow <vnum>\n\r", ch);
        return FALSE;
    }
    if (!is_number (argument)) {
        send_to_char ("REdit: Must be a number.\n\r", ch);
        return FALSE;
    }
    if (is_number (argument)) {
        value = atoi (argument);
        if (!(obj = get_obj_index (value))) {
            send_to_char ("REdit: That object does not exist.\n\r", ch);
            return FALSE;
        }
        ch->desc->olc_edit = (void *) obj;
    }

    oedit_show (ch, argument);
    ch->desc->olc_edit = (void *) ch->in_room;
    return FALSE;
}

/* Room Editor Functions. */
REDIT (redit_show) {
    ROOM_INDEX_T *room;
    char buf[MAX_STRING_LENGTH];
    char buf1[2 * MAX_STRING_LENGTH];
    OBJ_T *obj;
    CHAR_T *rch;
    int door;
    bool fcnt;

    EDIT_ROOM (ch, room);

    buf1[0] = '\0';

    sprintf (buf, "Description:\n\r%s", room->description);
    strcat (buf1, buf);

    sprintf (buf, "Name:       [%s]\n\rArea:       [%5d] %s\n\r",
             room->name, room->area->vnum, room->area->title);
    strcat (buf1, buf);

    sprintf (buf, "Vnum:       [%5d]\n\rSector:     [%s]\n\r",
             room->vnum, sector_get_name (room->sector_type));
    strcat (buf1, buf);

    sprintf (buf, "Room flags: [%s]\n\r",
             flags_to_string (room_flags, room->room_flags));
    strcat (buf1, buf);

    if (room->heal_rate != 100 || room->mana_rate != 100) {
        sprintf (buf, "Health rec: [%d]\n\rMana rec  : [%d]\n\r",
                 room->heal_rate, room->mana_rate);
        strcat (buf1, buf);
    }
    if (room->clan > 0) {
        sprintf (buf, "Clan      : [%d] %s\n\r",
                 room->clan, clan_table[room->clan].name);
        strcat (buf1, buf);
    }
    if (!IS_NULLSTR (room->owner)) {
        sprintf (buf, "Owner     : [%s]\n\r", room->owner);
        strcat (buf1, buf);
    }
    if (room->extra_descr) {
        EXTRA_DESCR_T *ed;

        strcat (buf1, "Desc Kwds:  [");
        for (ed = room->extra_descr; ed; ed = ed->next) {
            strcat (buf1, ed->keyword);
            if (ed->next)
                strcat (buf1, " ");
        }
        strcat (buf1, "]\n\r");
    }

    strcat (buf1, "Characters: [");
    fcnt = FALSE;
    for (rch = room->people; rch; rch = rch->next_in_room) {
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
    for (obj = room->contents; obj; obj = obj->next_content) {
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
        EXIT_T *pexit;
        if ((pexit = room->exit[door])) {
            const char *state;
            char word[MAX_INPUT_LENGTH];
            char reset_state[MAX_STRING_LENGTH];
            int i, length;

            sprintf (buf, "-%-5s to [%5d] Key: [%5d] ",
                     str_capitalized (door_table[door].name),
                     pexit->to_room ? pexit->to_room->vnum : 0, /* ROM OLC */
                     pexit->key);
            strcat (buf1, buf);

            /* Format up the exit info.
             * Capitalize all flags that are not part of the reset info. */
            strcpy (reset_state, flags_to_string (exit_flags, pexit->rs_flags));
            state = flags_to_string (exit_flags, pexit->exit_flags);
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
    { return redit_change_exit (ch, argument, DIR_NORTH) ? TRUE : FALSE; }
REDIT (redit_south)
    { return redit_change_exit (ch, argument, DIR_SOUTH) ? TRUE : FALSE; }
REDIT (redit_east)
    { return redit_change_exit (ch, argument, DIR_EAST)  ? TRUE : FALSE; }
REDIT (redit_west)
    { return redit_change_exit (ch, argument, DIR_WEST)  ? TRUE : FALSE; }
REDIT (redit_up)
    { return redit_change_exit (ch, argument, DIR_UP)    ? TRUE : FALSE; }
REDIT (redit_down)
    { return redit_change_exit (ch, argument, DIR_DOWN)  ? TRUE : FALSE; }

REDIT (redit_ed) {
    ROOM_INDEX_T *room;
    EXTRA_DESCR_T *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_ROOM (ch, room);

    argument = one_argument (argument, command);
    one_argument (argument, keyword);

    if (command[0] == '\0' || keyword[0] == '\0') {
        send_to_char ("Syntax: ed add [keyword]\n\r", ch);
        send_to_char ("        ed edit [keyword]\n\r", ch);
        send_to_char ("        ed delete [keyword]\n\r", ch);
        send_to_char ("        ed format [keyword]\n\r", ch);
        return FALSE;
    }

    if (!str_cmp (command, "add")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax: ed add [keyword]\n\r", ch);
            return FALSE;
        }
        ed = extra_descr_new ();
        ed->keyword = str_dup (keyword);
        ed->description = str_dup ("");
        LIST_FRONT (ed, next, room->extra_descr);

        string_append (ch, &ed->description);
        return TRUE;
    }

    if (!str_cmp (command, "edit")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax: ed edit [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND (str_in_namelist (keyword, ed->keyword), next,
            room->extra_descr, ed);
        if (!ed) {
            send_to_char ("REdit: Extra description keyword not found.\n\r", ch);
            return FALSE;
        }
        string_append (ch, &ed->description);
        return TRUE;
    }

    if (!str_cmp (command, "delete")) {
        EXTRA_DESCR_T *ped;
        if (keyword[0] == '\0') {
            send_to_char ("Syntax: ed delete [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND_WITH_PREV (str_in_namelist (keyword, ed->keyword),
            next, room->extra_descr, ed, ped);
        if (!ed) {
            send_to_char ("REdit: Extra description keyword not found.\n\r", ch);
            return FALSE;
        }
        LIST_REMOVE_WITH_PREV (ed, ped, next, room->extra_descr);

        extra_descr_free (ed);
        send_to_char ("Extra description deleted.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "format")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax: ed format [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND (str_in_namelist (keyword, ed->keyword), next,
            room->extra_descr, ed);
        if (!ed) {
            send_to_char ("REdit: Extra description keyword not found.\n\r", ch);
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
    AREA_T *area;
    ROOM_INDEX_T *room;
    int value, hash;

    EDIT_ROOM (ch, room);

    value = atoi (argument);
    if (argument[0] == '\0' || value <= 0) {
        send_to_char ("Syntax: create [vnum > 0]\n\r", ch);
        return FALSE;
    }

    area = area_get_by_inner_vnum (value);
    if (!area) {
        send_to_char ("REdit: That vnum is not assigned an area.\n\r", ch);
        return FALSE;
    }
    if (!IS_BUILDER (ch, area)) {
        send_to_char ("REdit: Vnum in an area you cannot build in.\n\r", ch);
        return FALSE;
    }
    if (get_room_index (value)) {
        send_to_char ("REdit: Room vnum already exists.\n\r", ch);
        return FALSE;
    }

    room = room_index_new ();
    room->area = area;
    room->vnum = value;
    room->anum = value - area->min_vnum;

    if (value > top_vnum_room)
        top_vnum_room = value;

    hash = value % MAX_KEY_HASH;
    LIST_FRONT (room, next, room_index_hash[hash]);
    ch->desc->olc_edit = (void *) room;

    send_to_char ("Room created.\n\r", ch);
    return TRUE;
}

REDIT (redit_name) {
    ROOM_INDEX_T *room;
    EDIT_ROOM (ch, room);

    if (argument[0] == '\0') {
        send_to_char ("Syntax: name [name]\n\r", ch);
        return FALSE;
    }

    str_replace_dup (&(room->name), argument);

    send_to_char ("Name set.\n\r", ch);
    return TRUE;
}

REDIT (redit_desc) {
    ROOM_INDEX_T *room;
    EDIT_ROOM (ch, room);

    if (argument[0] == '\0') {
        string_append (ch, &room->description);
        return TRUE;
    }

    send_to_char ("Syntax: desc\n\r", ch);
    return FALSE;
}

REDIT (redit_heal) {
    ROOM_INDEX_T *room;

    EDIT_ROOM (ch, room);
    if (is_number (argument)) {
        room->heal_rate = atoi (argument);
        send_to_char ("Heal rate set.\n\r", ch);
        return TRUE;
    }

    send_to_char ("Syntax: heal <#xnumber>\n\r", ch);
    return FALSE;
}

REDIT (redit_mana) {
    ROOM_INDEX_T *room;

    EDIT_ROOM (ch, room);
    if (is_number (argument)) {
        room->mana_rate = atoi (argument);
        send_to_char ("Mana rate set.\n\r", ch);
        return TRUE;
    }

    send_to_char ("Syntax: mana <#xnumber>\n\r", ch);
    return FALSE;
}

REDIT (redit_clan) {
    ROOM_INDEX_T *room;

    EDIT_ROOM (ch, room);
    room->clan = clan_lookup (argument);

    send_to_char ("Clan set.\n\r", ch);
    return TRUE;
}

REDIT (redit_format) {
    ROOM_INDEX_T *room;

    EDIT_ROOM (ch, room);
    room->description = format_string (room->description);

    send_to_char ("String formatted.\n\r", ch);
    return TRUE;
}

REDIT (redit_mreset) {
    ROOM_INDEX_T *room;
    MOB_INDEX_T *mob_index;
    CHAR_T *newmob;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    RESET_T *reset;

    EDIT_ROOM (ch, room);

    argument = one_argument (argument, arg);
    argument = one_argument (argument, arg2);

    if (arg[0] == '\0' || !is_number (arg)) {
        send_to_char ("Syntax: mreset <vnum> <max #x> <mix #x>\n\r", ch);
        return FALSE;
    }
    if (!(mob_index = get_mob_index (atoi (arg)))) {
        send_to_char ("REdit: No mobile has that vnum.\n\r", ch);
        return FALSE;
    }
    if (mob_index->area != room->area) {
        send_to_char ("REdit: No such mobile in this area.\n\r", ch);
        return FALSE;
    }

    /* Create the mobile reset. */
    reset = reset_data_new ();
    reset->command = 'M';
    reset->v.mob.mob_vnum     = mob_index->vnum;
    reset->v.mob.global_limit = is_number (arg2) ? atoi (arg2) : MAX_MOB;
    reset->v.mob.room_vnum    = room->vnum;
    reset->v.mob.room_limit   = is_number (argument) ? atoi (argument) : 1;
    redit_add_reset (room, reset, 0 /* Last slot */ );

    /* Create the mobile. */
    newmob = char_create_mobile (mob_index);
    char_to_room (newmob, room);

    printf_to_char (ch,
        "%s (%d) has been loaded and added to resets.\n\r"
        "There will be a maximum of %d loaded to this room.\n\r",
        str_capitalized (mob_index->short_descr),
        mob_index->vnum, reset->v.mob.room_limit);
    act ("$n has created $N!", ch, NULL, newmob, TO_NOTCHAR);
    return TRUE;
}

REDIT (redit_oreset) {
    ROOM_INDEX_T *room;
    OBJ_INDEX_T *obj_index;
    OBJ_T *newobj;
    OBJ_T *to_obj;
    CHAR_T *to_mob;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int olevel = 0;
    RESET_T *reset;

    EDIT_ROOM (ch, room);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || !is_number (arg1)) {
        send_to_char ("Syntax: oreset <vnum> <args>\n\r", ch);
        send_to_char ("        -no_args               = into room\n\r", ch);
        send_to_char ("        -<obj_name>            = into obj\n\r", ch);
        send_to_char ("        -<mob_name> <wear_loc> = into mob\n\r", ch);
        return FALSE;
    }
    if (!(obj_index = get_obj_index (atoi (arg1)))) {
        send_to_char ("REdit: No object has that vnum.\n\r", ch);
        return FALSE;
    }
    if (obj_index->area != room->area) {
        send_to_char ("REdit: No such object in this area.\n\r", ch);
        return FALSE;
    }

    /* Load into room. */
    if (arg2[0] == '\0') {
        reset = reset_data_new ();
        reset->command = 'O';
        reset->v.obj.obj_vnum     = obj_index->vnum;
        reset->v.obj.global_limit = 0;
        reset->v.obj.room_vnum    = room->vnum;
        redit_add_reset (room, reset, 0 /* Last slot */ );

        newobj = obj_create (obj_index, number_fuzzy (olevel));
        obj_give_to_room (newobj, room);

        printf_to_char (ch, "%s (%d) has been loaded and added to resets.\n\r",
            str_capitalized (obj_index->short_descr), obj_index->vnum);
    }
    /* Load into object's inventory. */
    else if (argument[0] == '\0' &&
        ((to_obj = find_obj_room (ch, room, arg2)) != NULL))
    {
        reset = reset_data_new ();
        reset->command = 'P';
        reset->v.put.obj_vnum      = obj_index->vnum;
        reset->v.put.global_limit  = 0;
        reset->v.put.into_vnum     = to_obj->index_data->vnum;
        reset->v.put.put_count     = 1;
        redit_add_reset (room, reset, 0 /* Last slot */ );

        newobj = obj_create (obj_index, number_fuzzy (olevel));
        newobj->cost = 0;
        obj_give_to_obj (newobj, to_obj);

        printf_to_char (ch, "%s (%d) has been loaded into "
            "%s (%d) and added to resets.\n\r",
            str_capitalized (newobj->short_descr), newobj->index_data->vnum,
            to_obj->short_descr, to_obj->index_data->vnum);
    }
    /* Load into mobile's inventory. */
    else if ((to_mob = find_char_same_room (ch, arg2)) != NULL) {
        const WEAR_LOC_T *wear_loc;

        /* Make sure the location on mobile is valid.  */
        RETURN_IF ((wear_loc = wear_loc_get_by_name (argument)) == NULL,
            "REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch, FALSE);

        /* Disallow loading a sword (WEAR_WIELD) into WEAR_HEAD. */
        if (!IS_SET (obj_index->wear_flags, wear_loc->wear_flag)) {
            printf_to_char (ch, "%s (%d) has wear flags: [%s]\n\r",
                str_capitalized (obj_index->short_descr), obj_index->vnum,
                flags_to_string (wear_flags, obj_index->wear_flags));
        }

        /* Can't load into same position.  */
        RETURN_IF (char_get_eq_by_wear_loc (to_mob, wear_loc->type),
            "REdit: Object already equipped.\n\r", ch, FALSE);

        reset = reset_data_new ();
        if (wear_loc->type == WEAR_LOC_NONE) {
            reset->command = 'G';
            reset->v.give.obj_vnum     = obj_index->vnum;
            reset->v.give.global_limit = 0;
        }
        else {
            reset->command = 'E';
            reset->v.equip.obj_vnum     = obj_index->vnum;
            reset->v.equip.global_limit = 0;
            reset->v.equip.wear_loc     = wear_loc->type;
        }

        redit_add_reset (room, reset, 0 /* Last slot */ );

        olevel = URANGE (0, to_mob->level - 2, LEVEL_HERO);
        newobj = obj_create (obj_index, number_fuzzy (olevel));

        /* Shop-keeper? */
        if (to_mob->index_data->shop) {
            switch (obj_index->item_type) {
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
                    if (reset->command == 'G')
                        olevel = number_range (5, 15);
                    else
                        olevel = number_fuzzy (olevel);
                    break;
            }

            newobj = obj_create (obj_index, olevel);
            if (wear_loc->type == WEAR_LOC_NONE)
                SET_BIT (newobj->extra_flags, ITEM_INVENTORY);
        }
        else
            newobj = obj_create (obj_index, number_fuzzy (olevel));

        obj_give_to_char (newobj, to_mob);
        if (reset->command == 'E')
            char_equip_obj (to_mob, newobj, reset->v.equip.wear_loc);

        printf_to_char (ch,
            "%s (%d) has been loaded %s of %s (%d) and added to resets.\n\r",
            str_capitalized (obj_index->short_descr),
            obj_index->vnum, wear_loc->phrase,
            to_mob->short_descr, to_mob->index_data->vnum);
    }
    /* Display Syntax */
    else {
        send_to_char ("REdit: That mobile isn't here.\n\r", ch);
        return FALSE;
    }

    act ("$n has created $p!", ch, newobj, NULL, TO_NOTCHAR);
    return TRUE;
}

REDIT (redit_owner) {
    ROOM_INDEX_T *room;
    EDIT_ROOM (ch, room);

    if (argument[0] == '\0') {
        send_to_char ("Syntax: owner [owner]\n\r", ch);
        send_to_char ("        owner none\n\r", ch);
        return FALSE;
    }

    str_free (&(room->owner));
    if (!str_cmp (argument, "none"))
        room->owner = str_dup ("");
    else
        room->owner = str_dup (argument);

    send_to_char ("Owner set.\n\r", ch);
    return TRUE;
}

REDIT (redit_room) {
    ROOM_INDEX_T *room;
    int value;

    EDIT_ROOM (ch, room);
    if ((value = flags_from_string (room_flags, argument)) == FLAG_NONE) {
        send_to_char ("Syntax: room [flags]\n\r", ch);
        return FALSE;
    }

    TOGGLE_BIT (room->room_flags, value);
    send_to_char ("Room flags toggled.\n\r", ch);
    return TRUE;
}

REDIT (redit_sector) {
    ROOM_INDEX_T *room;
    int value;

    EDIT_ROOM (ch, room);
    if ((value = sector_lookup (argument)) < 0) {
        send_to_char ("Syntax: sector [type]\n\r", ch);
        return FALSE;
    }

    room->sector_type = value;
    send_to_char ("Sector type set.\n\r", ch);
    return TRUE;
}
