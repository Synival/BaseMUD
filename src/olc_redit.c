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
#include "mobiles.h"
#include "rooms.h"
#include "extra_descrs.h"
#include "resets.h"
#include "portals.h"

#include "olc_redit.h"

/* Local function. */
bool redit_change_exit (CHAR_T *ch, char *argument, int door) {
    ROOM_INDEX_T *room;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int value;

    EDIT_ROOM (ch, room);

    /* Set the exit flags, needs full argument. */
    if ((value = flags_from_string (exit_flags, argument)) != FLAG_NONE) {
        EXIT_T *ex, *rev_ex;
        ROOM_INDEX_T *to_room;
        sh_int rev; /* ROM OLC */

        RETURN_IF (!(ex = room_get_orig_exit (room, door)),
            "Exit doesn't exist.\n\r", ch, FALSE);

        /* This room. */
        TOGGLE_BIT (ex->rs_flags, value);

        /* Don't toggle exit_flags because it can be changed by players. */
        ex->exit_flags = ex->rs_flags;

        /* Connected room. */
        to_room = ex->to_room; /* ROM OLC */
        rev = door_table[door].reverse;

        if (to_room != NULL && (rev_ex = room_get_orig_exit (to_room, rev))) {
            rev_ex->exit_flags = ex->exit_flags;
            rev_ex->rs_flags   = ex->rs_flags;
        }

        printf_to_char (ch, "Exit flag toggled.\n\r");
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
        ROOM_INDEX_T *old_room;
        EXIT_T *ex, *rev_ex;
        sh_int rev; /* ROM OLC */

        RETURN_IF (!(ex = room_get_orig_exit (room, door)),
            "REdit: Cannot delete a null exit.\n\r", ch, FALSE);

        /* Get an opposite exit and automatically unlink it.
         * Don't delete it unless it points back to this room. */
        rev = door_table[door].reverse;
        old_room = ex->to_room; /* ROM OLC */

        if (old_room) {
            if ((rev_ex = room_get_orig_exit (old_room, rev)) != NULL)
                if (rev_ex->to_room != room)
                    rev_ex = NULL;
        }
        else
            rev_ex = NULL;

        /* Unlink our exit. */
        exit_to_room_index_to (ex, NULL);
        printf_to_char (ch, "Exit unlinked.\n\r");
        if (rev_ex) {
            exit_to_room_index_to (rev_ex, NULL);
            printf_to_char (ch, "Reverse exit unlinked.\n\r");
        }

        /* Update portals if necessary. */
        redit_change_exit_update_portals (ex, old_room, NULL, ch);

        /* Remove our exit and, if found, the opposite exit. */
        redit_change_exit_free (ex);
        if (rev_ex)
            redit_change_exit_free (rev_ex);

        /* Remove this exit. */
        return TRUE;
    }

    if (!str_cmp (command, "link")) {
        int rev;
        EXIT_T *ex, *rev_ex;
        ROOM_INDEX_T *old_room, *old_rev_room, *to_room;

        RETURN_IF (arg[0] == '\0' || !is_number (arg),
            "Syntax: [direction] link [vnum]\n\r", ch, FALSE);

        value = atoi (arg);
        RETURN_IF (!(to_room = room_get_index (value)),
            "REdit: Cannot link to non-existant room.\n\r", ch, FALSE);
        RETURN_IF (!IS_BUILDER (ch, to_room->area),
            "REdit: Cannot link to that area.\n\r", ch, FALSE);

        rev = door_table[door].reverse;
        rev_ex = room_get_orig_exit (to_room, rev);
        RETURN_IF (rev_ex != NULL && rev_ex->to_room,
            "REdit: Remote side's exit already exists.\n\r", ch, FALSE);

        /* Get (or create) the exit on the player side. */
        if ((ex = room_get_orig_exit (room, door)) == NULL)
            ex = room_create_exit (room, door);

        /* Create the opposite exit if necessary. */
        if (rev_ex == NULL)
            rev_ex = room_create_exit (to_room, rev);

        /* Link the two exits together. */
        old_room     = ex->to_room;
        old_rev_room = rev_ex->to_room;
        exit_to_room_index_to (ex, to_room);
        exit_to_room_index_to (rev_ex, room);

        /* Update portals if necessary. */
        printf_to_char (ch, "Two-way link established.\n\r");
        redit_change_exit_update_portals (ex, old_room, old_rev_room, ch);

        return TRUE;
    }

    if (!str_cmp (command, "dig")) {
        char buf[MAX_STRING_LENGTH];
        RETURN_IF (arg[0] == '\0' || !is_number (arg),
            "Syntax: [direction] dig <vnum>\n\r", ch, FALSE);

        redit_create (ch, arg);
        sprintf (buf, "link %s", arg);
        redit_change_exit (ch, buf, door);
        return TRUE;
    }

    if (!str_cmp (command, "room")) {
        EXIT_T *ex;
        ROOM_INDEX_T *old_room, *to_room;

        RETURN_IF (arg[0] == '\0' || !is_number (arg),
            "Syntax: [direction] room [vnum]\n\r", ch, FALSE);

        value = atoi (arg);
        RETURN_IF (!(to_room = room_get_index (value)),
            "REdit: Cannot link to non-existant room.\n\r", ch, FALSE);

        /* Create or update the existing exit. */
        if ((ex = room_get_orig_exit (room, door)) == NULL)
            ex = room_create_exit (room, door);

        old_room = ex->to_room;
        exit_to_room_index_to (ex, to_room);
        printf_to_char (ch, "One-way link established.\n\r");

        /* Update portals if necessary. */
        redit_change_exit_update_portals (ex, old_room, NULL, ch);

        return TRUE;
    }

    if (!str_cmp (command, "key")) {
        EXIT_T *ex;
        OBJ_INDEX_T *key;

        RETURN_IF (arg[0] == '\0' || !is_number (arg),
            "Syntax: [direction] key [vnum]\n\r", ch, FALSE);
        RETURN_IF (!(ex = room_get_orig_exit (room, door)),
            "Exit doesn't exist.\n\r", ch, FALSE);

        value = atoi (arg);
        RETURN_IF (!(key = obj_get_index (value)),
            "REdit: Key doesn't exist.\n\r", ch, FALSE);
        RETURN_IF (key->item_type != ITEM_KEY,
            "REdit: Object is not a key.\n\r", ch, FALSE);

        ex->key = value;
        printf_to_char (ch, "Exit key set.\n\r");
        return TRUE;
    }

    if (!str_cmp (command, "name")) {
        EXIT_T *ex;

        RETURN_IF (arg[0] == '\0',
            "Syntax: [direction] name [string]\n\r"
            "        [direction] name none\n\r", ch, FALSE);
        RETURN_IF (!(ex = room_get_orig_exit (room, door)),
            "Exit doesn't exist.\n\r", ch, FALSE);

        if (!str_cmp (arg, "none"))
            str_replace_dup (&ex->keyword, "");
        else
            str_replace_dup (&ex->keyword, arg);

        printf_to_char (ch, "Exit name set.\n\r");
        return TRUE;
    }

    if (!str_prefix (command, "description")) {
        EXIT_T *ex;

        RETURN_IF (arg[0] != '\0',
            "Syntax: [direction] desc\n\r", ch, FALSE);
        RETURN_IF (!(ex = room_get_orig_exit (room, door)),
            "Exit doesn't exist.\n\r", ch, FALSE);

        string_append (ch, &ex->description);
        return TRUE;
    }

    if (!str_prefix (command, "portal")) {
        EXIT_T *ex;

        RETURN_IF (arg[0] == '\0',
            "Syntax: [direction] portal [name]\n\r", ch, FALSE);

        if ((ex = room_get_orig_exit (room, door)) == NULL)
            ex = room_create_exit (room, door);

        RETURN_IF (ex->portal && strcmp (arg, ex->portal->name) == 0,
            "The portal exit name provided is the same.\n\r", ch, FALSE);
        RETURN_IF (portal_exit_lookup_exact (arg),
            "A portal exit with that name already exists.\n\r", ch, FALSE);

        if (ex->portal) {
            RETURN_IF (!portal_exit_rename (ex->portal, arg),
                "The portal exit could not be renamed.\n\r", ch, FALSE);
            printf_to_char (ch, "Portal renamed.\n\r");
            return TRUE;
        }
        else {
            PORTAL_EXIT_T *pex;
            pex = portal_exit_create (arg);
            portal_exit_to_exit (pex, ex);
            printf_to_char (ch, "Portal created.\n\r");
            return TRUE;
        }
    }

    do_help (ch, "EXIT");
    return FALSE;
}

void redit_change_exit_free (EXIT_T *ex) {
    if (ex->portal)
        exit_to_room_index_to (ex, NULL);
    else
        exit_free (ex);
}

void redit_change_exit_update_portals (EXIT_T *ex, ROOM_INDEX_T *old_room,
    ROOM_INDEX_T *old_rev_rev_room, CHAR_T *ch)
{
    EXIT_T *new_rev_ex, *old_rev_ex, *old_rev_rev_ex;
    ROOM_INDEX_T *from_room, *new_room;
    int dir, rev_dir;

    /* Gather all data we may need to modify. */
    dir     = ex->orig_door;
    rev_dir = door_table[dir].reverse;
    from_room = ex->from_room;

    /* Get the reverse exit, if it exists. It must (now) point back to the exit
     * we've modified. */
    new_room   = ex->to_room;
    new_rev_ex = (new_room) ? room_get_orig_exit (new_room, rev_dir) : NULL;
    if (new_rev_ex && new_rev_ex->to_room != from_room)
        new_rev_ex = NULL;

    /* Get the reverse exit of the old room, if it exists. It must (still)
     * point to the exit we've modified. */
    old_rev_ex = (old_room) ? room_get_orig_exit (old_room, rev_dir) : NULL;
    if (old_rev_ex && old_rev_ex->to_room != from_room)
        old_rev_ex = NULL;

    /* If there's a new reverse exit, its target room has been overridden.
     * There's a possibility that this was a two-way portal, in which case
     * we need to fix the old reverse exit OF the reverse exit. */
    old_rev_rev_ex = (new_rev_ex && old_rev_rev_room) ?
        room_get_orig_exit (old_rev_rev_room, dir) : NULL;
    if (old_rev_rev_ex && old_rev_rev_ex->to_room != new_room)
        old_rev_rev_ex = NULL;

    /* Fix individual exits. */
    redit_change_exit_update_portal (ex,             ch);
    redit_change_exit_update_portal (new_rev_ex,     ch);
    redit_change_exit_update_portal (old_rev_ex,     ch);
    redit_change_exit_update_portal (old_rev_rev_ex, ch);
}

void redit_change_exit_update_portal (EXIT_T *ex, CHAR_T *ch) {
    EXIT_T *rev_ex;
    PORTAL_T *portal;
    const char *dir_name;
    bool needs_portal;
    int rev_dir;

    if (ex == NULL)
        return;

    /* See if this exit needs a portal at all. */
    needs_portal = (ex->to_room != NULL) &&
                   (ex->to_room->area != ex->from_room->area);
    dir_name = door_table[ex->orig_door].name;

    /* If not, delete the portal. */
    if (!needs_portal) {
        if (ex->portal) {
            if ((portal = portal_get_with_outgoing_portal_exit (ex->portal)))
                redit_change_exit_remove_portal (portal, ch);

            if (ex->to_room != NULL) {
                printf_to_char (ch,
                    "Removing portal exit '%s' from '%s' on '%s'.\n",
                    ex->portal->name, dir_name, ex->from_room->name);
                portal_exit_free (ex->portal);
                ex->portal = NULL;
            }
        }
        return;
    }

    /* A portal is required. Create the portal exit if it doesn't exist. */
    if (ex->portal == NULL) {
        portal_exit_create_on_exit (ex);
        printf_to_char (ch,
            "Created portal exit '%s' on '%s' from '%s'.\n",
            ex->portal->name, dir_name, ex->from_room->name);
    }

    /* Get the reverse direction.  It must point back to this same room. */
    rev_dir = door_table[ex->orig_door].reverse;
    rev_ex = room_get_orig_exit (ex->to_room, rev_dir);
    if (rev_ex && rev_ex->to_room != ex->from_room)
        rev_ex = NULL;

    /* Is this a one-way portal? */
    if (rev_ex == NULL) {
        /* Create the portal exit on the room if it doesn't exist. */
        if (ex->to_room->portal == NULL) {
            portal_exit_create_on_room (ex->to_room);
            printf_to_char (ch,
                "Created portal exit '%s' on room '%s'.\n",
                ex->to_room->portal->name, ex->to_room->name);
        }

        /* Make sure we have the correct portal. */
        redit_change_exit_connect_portal_exits (
            ex->portal, ex->to_room->portal, ch);
        return;
    }
    /* It's a two-way portal. Create the portal exit on the reverse exit if it
     * doesn't exist. */
    else {
        if (rev_ex->portal == NULL) {
            portal_exit_create_on_exit (rev_ex);
            printf_to_char (ch,
                "Created portal exit '%s' on '%s' from '%s'.\n",
                rev_ex->portal->name, door_table[rev_dir].name,
                rev_ex->from_room->name);
        }

        /* Make sure we have the correct portal. */
        redit_change_exit_connect_portal_exits (
            ex->portal, rev_ex->portal, ch);
    }
}

void redit_change_exit_connect_portal_exits (PORTAL_EXIT_T *from,
    PORTAL_EXIT_T *to, CHAR_T *ch)
{
    PORTAL_T *portal;
    bool two_way = (to->exit) ? TRUE : FALSE;

    /* Is there an existing outgoing portal? */
    portal = portal_get_with_outgoing_portal_exit (from);
    if (portal) {
        if (portal->two_way == two_way) {
            /* If it's one-way, do nothing more if the portal goes to
             * the same place. */
            if (!two_way && portal->to == to)
                return;

            /* If it's two-way, do nothing more if either direction has
             * our destination. */
            if (two_way && (portal->from == to || portal->to == to))
                return;
        }

        /* Validity checks didn't work - remove this portal. */
        redit_change_exit_remove_portal (portal, ch);
    }

    /* Is there a one-way portal from the "from" direction? */
    if (two_way) {
        portal = portal_get_with_outgoing_portal_exit (to);
        if (portal)
            redit_change_exit_remove_portal (portal, ch);
    }

    /* We'll need to create a new portal. */
    portal = portal_new ();
    portal_to_portal_exit_from (portal, from);
    portal_to_portal_exit_to   (portal, to);

    if (!two_way) {
        printf_to_char (ch,
            "Creating one-way portal from '%s' to '%s'.\n",
            from->name, to->name);
    }
    else {
        printf_to_char (ch,
            "Creating two-way portal between '%s' and '%s'.\n",
            from->name, to->name);
    }
}

void redit_change_exit_remove_portal (PORTAL_T *portal, CHAR_T *ch) {
    if (!portal->two_way) {
        printf_to_char (ch,
            "Removing one-way portal link from '%s' to '%s'.\n",
            portal->name_from, portal->name_to);
    }
    else {
        printf_to_char (ch,
            "Removing two-way portal link between '%s' and '%s'.\n",
            portal->name_from, portal->name_to);
    }
    portal_free (portal);
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
    found = FALSE;

    for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++) {
        if (!(room_index = room_get_index (vnum)))
            continue;

        sprintf (buf, "[%5d] %-17.16s",
            vnum, str_capitalized (room_index->name));
        buf_cat (buf1, buf);
        if (++col % 3 == 0)
            buf_cat (buf1, "\n\r");
        found = TRUE;
    }
    RETURN_IF (!found,
        "Room(s) not found in this area.\n\r", ch, FALSE);

    if (col % 3 != 0)
        buf_cat (buf1, "\n\r");

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
    RETURN_IF (arg[0] == '\0',
        "Syntax: mlist <all/name>\n\r", ch, FALSE);

    buf1 = buf_new ();
    area = ch->in_room->area;
    all = !str_cmp (arg, "all");
    found = FALSE;

    for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++) {
        if ((mob_index = mobile_get_index (vnum)) == NULL)
            continue;
        if (!(all || str_in_namelist (arg, mob_index->name)))
            continue;

        sprintf (buf, "[%5d] %-17.16s",
            mob_index->vnum, str_capitalized (mob_index->short_descr));
        buf_cat (buf1, buf);
        if (++col % 3 == 0)
            buf_cat (buf1, "\n\r");
        found = TRUE;
    }
    RETURN_IF (!found,
        "Mobile(s) not found in this area.\n\r", ch, FALSE);

    if (col % 3 != 0)
        buf_cat (buf1, "\n\r");

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
    RETURN_IF (arg[0] == '\0',
        "Syntax: olist <all/name/item_type>\n\r", ch, FALSE);

    area = ch->in_room->area;
    buf1 = buf_new ();
    all = !str_cmp (arg, "all");
    found = FALSE;

    item_type = item_lookup (arg);
    for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++) {
        if (!(obj_index = obj_get_index (vnum)))
            continue;
        if (!(all || str_in_namelist (arg, obj_index->name) ||
                item_type == obj_index->item_type))
            continue;

        sprintf (buf, "[%5d] %-17.16s",
            obj_index->vnum, str_capitalized (obj_index->short_descr));
        buf_cat (buf1, buf);
        if (++col % 3 == 0)
            buf_cat (buf1, "\n\r");
        found = TRUE;
    }
    RETURN_IF (!found,
        "Object(s) not found in this area.\n\r", ch, FALSE);

    if (col % 3 != 0)
        buf_cat (buf1, "\n\r");

    page_to_char (buf_string (buf1), ch);
    buf_free (buf1);
    return FALSE;
}

REDIT (redit_mshow) {
    MOB_INDEX_T *mob;
    int value;

    RETURN_IF (argument[0] == '\0',
        "Syntax: mshow <vnum>\n\r", ch, FALSE);
    RETURN_IF (!is_number (argument),
        "REdit: Must be a number.\n\r", ch, FALSE);

    if (is_number (argument)) {
        value = atoi (argument);
        RETURN_IF (!(mob = mobile_get_index (value)),
            "REdit: That mobile does not exist.\n\r", ch, FALSE);
        ch->desc->olc_edit = (void *) mob;
    }

    medit_show (ch, argument);
    ch->desc->olc_edit = (void *) ch->in_room;
    return FALSE;
}

REDIT (redit_oshow) {
    OBJ_INDEX_T *obj;
    int value;

    RETURN_IF (argument[0] == '\0',
        "Syntax: oshow <vnum>\n\r", ch, FALSE);
    RETURN_IF (!is_number (argument),
        "REdit: Must be a number.\n\r", ch, FALSE);

    if (is_number (argument)) {
        value = atoi (argument);
        RETURN_IF (!(obj = obj_get_index (value)),
            "REdit: That object does not exist.\n\r", ch, FALSE);
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
    int orig_door, real_door;
    bool fcnt;
    EXIT_T *pexit;
    const char *state;
    char word[MAX_INPUT_LENGTH];
    char reset_state[MAX_STRING_LENGTH];
    int i, length;

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
    if (room->extra_descr_first) {
        EXTRA_DESCR_T *ed;

        strcat (buf1, "Desc Kwds:  [");
        for (ed = room->extra_descr_first; ed; ed = ed->on_next) {
            strcat (buf1, ed->keyword);
            if (ed->on_next)
                strcat (buf1, " ");
        }
        strcat (buf1, "]\n\r");
    }

    strcat (buf1, "Characters: [");
    fcnt = FALSE;
    for (rch = room->people_first; rch; rch = rch->room_next) {
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
    for (obj = room->content_first; obj; obj = obj->content_next) {
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

    if (room->portal) {
        sprintf (buf, "Portal:     [%s]\n\r", room->portal->name);
        strcat (buf1, buf);
    }

    for (real_door = 0; real_door < DIR_MAX; real_door++) {
        if (!(pexit = room_get_orig_exit (room, real_door)))
            continue;
        orig_door = pexit->orig_door;

        sprintf (buf, "-%-5s to [%5d] Key: [%5d] ",
                 str_capitalized (door_table[orig_door].name),
                 pexit->to_room ? pexit->to_room->vnum : 0, /* ROM OLC */
                 pexit->key);
        strcat (buf1, buf);

        if (real_door != orig_door) {
            sprintf (buf, "(Currently randomized to '%s' exit)\n",
                door_table[real_door].name);
            strcat (buf1, buf);
        }

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
        strcat (buf1, "\n\r");

        if (pexit->portal) {
            PORTAL_EXIT_T *pex = pexit->portal;
            sprintf (buf, " Portal: [%s]\n\r", pex->name);
            strcat (buf1, buf);
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

    printf_to_char (ch, buf1);
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

    RETURN_IF (command[0] == '\0' || keyword[0] == '\0',
        "Syntax: ed add [keyword]\n\r"
        "        ed edit [keyword]\n\r"
        "        ed delete [keyword]\n\r"
        "        ed format [keyword]\n\r", ch, FALSE);

    if (!str_cmp (command, "add")) {
        RETURN_IF (keyword[0] == '\0',
            "Syntax: ed add [keyword]\n\r", ch, FALSE);

        ed = extra_descr_new ();
        ed->keyword = str_dup (keyword);
        ed->description = str_dup ("");
        extra_descr_to_room_index_back (ed, room);

        string_append (ch, &ed->description);
        return TRUE;
    }

    if (!str_cmp (command, "edit")) {
        RETURN_IF (keyword[0] == '\0',
            "Syntax: ed edit [keyword]\n\r", ch, FALSE);
        LIST_FIND (str_in_namelist (keyword, ed->keyword), on_next,
            room->extra_descr_first, ed);
        RETURN_IF (!ed,
            "REdit: Extra description keyword not found.\n\r", ch, FALSE);

        string_append (ch, &ed->description);
        return TRUE;
    }

    if (!str_cmp (command, "delete")) {
        RETURN_IF (keyword[0] == '\0',
            "Syntax: ed delete [keyword]\n\r", ch, FALSE);
        LIST_FIND (str_in_namelist (keyword, ed->keyword), on_next,
            room->extra_descr_first, ed);
        RETURN_IF (!ed,
            "REdit: Extra description keyword not found.\n\r", ch, FALSE);

        extra_descr_free (ed);
        printf_to_char (ch, "Extra description deleted.\n\r");
        return TRUE;
    }

    if (!str_cmp (command, "format")) {
        RETURN_IF (keyword[0] == '\0',
            "Syntax: ed format [keyword]\n\r", ch, FALSE);
        LIST_FIND (str_in_namelist (keyword, ed->keyword), on_next,
            room->extra_descr_first, ed);
        RETURN_IF (!ed,
            "REdit: Extra description keyword not found.\n\r", ch, FALSE);

        ed->description = format_string (ed->description);
        printf_to_char (ch, "Extra description formatted.\n\r");
        return TRUE;
    }

    redit_ed (ch, "");
    return FALSE;
}

REDIT (redit_create) {
    AREA_T *area;
    ROOM_INDEX_T *room;
    int value;

    EDIT_ROOM (ch, room);

    value = atoi (argument);
    RETURN_IF (argument[0] == '\0' || value <= 0,
        "Syntax: create [vnum > 0]\n\r", ch, FALSE);

    area = area_get_by_inner_vnum (value);
    RETURN_IF (!area,
        "REdit: That vnum is not assigned an area.\n\r", ch, FALSE);
    RETURN_IF (!IS_BUILDER (ch, area),
        "REdit: Vnum in an area you cannot build in.\n\r", ch, FALSE);
    RETURN_IF (room_get_index (value),
        "REdit: Room vnum already exists.\n\r", ch, FALSE);

    room = room_index_new ();
    room_to_area (room, area);

    room->vnum = value;
    room->anum = value - area->min_vnum;
    db_register_new_room (room);

    ch->desc->olc_edit = (void *) room;

    printf_to_char (ch, "Room created.\n\r");
    return TRUE;
}

REDIT (redit_name) {
    ROOM_INDEX_T *room;
    EDIT_ROOM (ch, room);

    RETURN_IF (argument[0] == '\0',
        "Syntax: name [name]\n\r", ch, FALSE);

    str_replace_dup (&(room->name), argument);
    printf_to_char (ch, "Name set.\n\r");
    return TRUE;
}

REDIT (redit_desc) {
    ROOM_INDEX_T *room;
    EDIT_ROOM (ch, room);

    RETURN_IF (argument[0] != '\0',
        "Syntax: desc\n\r", ch, FALSE);

    string_append (ch, &room->description);
    return TRUE;
}

REDIT (redit_heal) {
    ROOM_INDEX_T *room;
    EDIT_ROOM (ch, room);

    RETURN_IF (!is_number (argument),
        "Syntax: heal <#xnumber>\n\r", ch, FALSE);

    room->heal_rate = atoi (argument);
    printf_to_char (ch, "Heal rate set.\n\r");
    return TRUE;
}

REDIT (redit_mana) {
    ROOM_INDEX_T *room;
    EDIT_ROOM (ch, room);

    RETURN_IF (!is_number (argument),
        "Syntax: mana <#xnumber>\n\r", ch, FALSE);

    room->mana_rate = atoi (argument);
    printf_to_char (ch, "Mana rate set.\n\r");
    return TRUE;
}

REDIT (redit_clan) {
    ROOM_INDEX_T *room;
    EDIT_ROOM (ch, room);

    /* TODO: Show an error if the clan doesn't exist */
    room->clan = clan_lookup (argument);
    printf_to_char (ch, "Clan set.\n\r");
    return TRUE;
}

REDIT (redit_format) {
    ROOM_INDEX_T *room;
    EDIT_ROOM (ch, room);

    room->description = format_string (room->description);
    printf_to_char (ch, "String formatted.\n\r");
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

    RETURN_IF (arg[0] == '\0' || !is_number (arg),
        "Syntax: mreset <vnum> <max #x> <mix #x>\n\r", ch, FALSE);
    RETURN_IF (!(mob_index = mobile_get_index (atoi (arg))),
        "REdit: No mobile has that vnum.\n\r", ch, FALSE);
    RETURN_IF (mob_index->area != room->area,
        "REdit: No such mobile in this area.\n\r", ch, FALSE);

    /* Create the mobile reset. */
    reset = reset_data_new ();
    reset->command = 'M';
    reset->v.mob.mob_vnum     = mob_index->vnum;
    reset->v.mob.global_limit = is_number (arg2) ? atoi (arg2) : MAX_MOB;
    reset->v.mob.room_vnum    = room->vnum;
    reset->v.mob.room_limit   = is_number (argument) ? atoi (argument) : 1;
    reset_to_room (reset, room);

    /* Create the mobile. */
    newmob = mobile_create (mob_index);
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

    RETURN_IF (arg1[0] == '\0' || !is_number (arg1),
        "Syntax: oreset <vnum> <args>\n\r"
        "        -no_args               = into room\n\r"
        "        -<obj_name>            = into obj\n\r"
        "        -<mob_name> <wear_loc> = into mob\n\r", ch, FALSE);

    RETURN_IF (!(obj_index = obj_get_index (atoi (arg1))),
        "REdit: No object has that vnum.\n\r", ch, FALSE);
    RETURN_IF (obj_index->area != room->area,
        "REdit: No such object in this area.\n\r", ch, FALSE);

    /* Load into room. */
    if (arg2[0] == '\0') {
        reset = reset_data_new ();
        reset->command = 'O';
        reset->v.obj.obj_vnum     = obj_index->vnum;
        reset->v.obj.global_limit = 0;
        reset->v.obj.room_vnum    = room->vnum;
        reset_to_room (reset, room);

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
        reset->v.put.obj_vnum     = obj_index->vnum;
        reset->v.put.global_limit = 0;
        reset->v.put.into_vnum    = to_obj->obj_index->vnum;
        reset->v.put.put_count    = 1;
        reset_to_room (reset, room);

        newobj = obj_create (obj_index, number_fuzzy (olevel));
        newobj->cost = 0;
        obj_give_to_obj (newobj, to_obj);

        printf_to_char (ch, "%s (%d) has been loaded into "
            "%s (%d) and added to resets.\n\r",
            str_capitalized (newobj->short_descr), newobj->obj_index->vnum,
            to_obj->short_descr, to_obj->obj_index->vnum);
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
        reset_to_room (reset, room);

        olevel = URANGE (0, to_mob->level - 2, LEVEL_HERO);
        newobj = obj_create (obj_index, number_fuzzy (olevel));

        /* Shop-keeper? */
        if (to_mob->mob_index->shop) {
            /* TODO: migrate this to item_???() */
            switch (obj_index->item_type) {
                case ITEM_PILL:
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
                default:
                    olevel = 0;
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
            to_mob->short_descr, to_mob->mob_index->vnum);
    }
    /* Display Syntax */
    else {
        printf_to_char (ch, "REdit: That mobile isn't here.\n\r");
        return FALSE;
    }

    act ("$n has created $p!", ch, newobj, NULL, TO_NOTCHAR);
    return TRUE;
}

REDIT (redit_owner) {
    ROOM_INDEX_T *room;
    EDIT_ROOM (ch, room);

    RETURN_IF (argument[0] == '\0',
        "Syntax: owner [owner]\n\r"
        "        owner none\n\r", ch, FALSE);

    if (!str_cmp (argument, "none"))
        str_replace_dup (&room->owner, "");
    else
        str_replace_dup (&room->owner, argument);

    printf_to_char (ch, "Owner set.\n\r");
    return TRUE;
}

REDIT (redit_room) {
    ROOM_INDEX_T *room;
    int value;

    EDIT_ROOM (ch, room);

    RETURN_IF ((value = flags_from_string (room_flags, argument)) == FLAG_NONE,
        "Syntax: room [flags]\n\r", ch, FALSE);

    TOGGLE_BIT (room->room_flags, value);
    printf_to_char (ch, "Room flags toggled.\n\r");
    return TRUE;
}

REDIT (redit_sector) {
    ROOM_INDEX_T *room;
    int value;

    EDIT_ROOM (ch, room);

    RETURN_IF ((value = sector_lookup (argument)) < 0,
        "Syntax: sector [type]\n\r", ch, FALSE);

    room->sector_type = value;
    printf_to_char (ch, "Sector type set.\n\r");
    return TRUE;
}

REDIT (redit_portal) {
    ROOM_INDEX_T *room;
    EDIT_ROOM (ch, room);

    RETURN_IF (argument[0] == '\0',
        "Syntax: portal [name]\n\r", ch, FALSE);
    RETURN_IF (room->portal && strcmp (argument, room->portal->name) == 0,
        "The portal exit name provided is the same.\n\r", ch, FALSE);
    RETURN_IF (portal_exit_lookup_exact (argument),
        "A portal exit with that name already exists.\n\r", ch, FALSE);

    if (room->portal) {
        RETURN_IF (!portal_exit_rename (room->portal, argument),
            "The portal exit could not be renamed.\n\r", ch, FALSE);
        printf_to_char (ch, "Portal renamed.\n\r");
        return TRUE;
    }
    else {
        PORTAL_EXIT_T *pex;
        pex = portal_exit_create (argument);
        portal_exit_to_room (pex, room);
        printf_to_char (ch, "Portal created.\n\r");
        return TRUE;
    }
}
