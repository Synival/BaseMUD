/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@hypercube.org)                                *
 *      Gabrielle Taylor (gtaylor@hypercube.org)                           *
 *      Brian Moore (zump@rom.org)                                         *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 ***************************************************************************/

#include "portals.h"

#include "lookup.h"
#include "memory.h"
#include "recycle.h"
#include "rooms.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

/* TODO: check for duplicate portals. */

PORTAL_EXIT_T *portal_exit_create_on_room (ROOM_INDEX_T *room) {
    PORTAL_EXIT_T *pex;
    char buf[256];
    int num = 1;

    do {
        snprintf (buf, sizeof(buf), "%s-room-%d", room->area->name, num++);
    } while (portal_exit_lookup_exact (buf));

    pex = portal_exit_create (buf);
    portal_exit_to_room (pex, room);
    return pex;
}

PORTAL_EXIT_T *portal_exit_create_on_exit (EXIT_T *exit) {
    PORTAL_EXIT_T *pex;
    char buf[256];
    int num = 1;

    do {
        snprintf (buf, sizeof(buf), "%s-%s-%d", exit->from_room->area->name,
            door_get_name (exit->orig_door), num++);
    } while (portal_exit_lookup_exact (buf));

    pex = portal_exit_create (buf);
    portal_exit_to_exit (pex, exit);
    return pex;
}

PORTAL_EXIT_T *portal_exit_create (const char *name) {
    PORTAL_EXIT_T *pex;
    RETURN_IF_BUGF ((pex = portal_exit_lookup_exact (name)) != NULL, NULL,
        "portal_exit_create(): Portal '%s' already exists", name);

    pex = portal_exit_new ();
    str_replace_dup (&pex->name, name);
    return pex;
}

void portal_link_unassigned_by_names (void) {
    PORTAL_T *p;
    int link_count, fail_count;

    link_count = 0;
    fail_count = 0;
    for (p = portal_get_first(); p; p = portal_get_next (p)) {
        if (p->from && p->to)
            continue;
        if (portal_to_portal_exits_by_name (p))
            link_count++;
        else
            fail_count++;
    }

    if (link_count > 0)
        log_f ("%d portals successfully linked by name", link_count);
    if (fail_count > 0)
        bugf ("%d portals failed to link by name", fail_count);
}

bool portal_to_portal_exits_by_name (PORTAL_T *portal) {
    PORTAL_EXIT_T *from, *to;

    /* Make sure the portal has labels. */
    RETURN_IF_BUGF (portal->name_from == NULL, FALSE,
        "portal_apply_to_exits: Portal is missing 'name_from'");
    RETURN_IF_BUGF (portal->name_to == NULL, FALSE,
        "portal_apply_to_exits: Portal is missing 'name_to'");

    /* Make sure both portal points exist. */
    RETURN_IF_BUGF ((from = portal_exit_lookup_exact (portal->name_from)) == NULL,
        FALSE, "portal_apply_to_exits: Cannot find 'from' portal exit named '%s'",
        portal->name_from);
    RETURN_IF_BUGF ((to = portal_exit_lookup_exact (portal->name_to)) == NULL,
        FALSE, "portal_apply_to_exits: Cannot find 'to' portal exit named '%s'",
        portal->name_to);

    /* We found the portal exits - link them. */
    portal_to_portal_exits (portal, from, to);
    return TRUE;
}

void portal_to_portal_exits (PORTAL_T *portal, PORTAL_EXIT_T *pex_from,
    PORTAL_EXIT_T *pex_to)
{
    BAIL_IF_BUGF (pex_from == pex_to && pex_from != NULL,
        "portal_to_portal_exits: Portal cannot lead to the same place: '%s'",
        pex_from->name);

    portal_to_portal_exit_from (portal, pex_from);
    portal_to_portal_exit_to   (portal, pex_to);

    if (portal->from && portal->to) {
        if (portal->to->exit) {
            exit_to_room_index_to (portal->from->exit,
                                   portal->to->exit->from_room);
            exit_to_room_index_to (portal->to->exit,
                                   portal->from->exit->from_room);
        }
        else if (portal->to->room) {
            exit_to_room_index_to (portal->from->exit,
                                   portal->to->room);
        }
    }
}

void portal_to_portal_exit_from (PORTAL_T *portal, PORTAL_EXIT_T *pex) {
    if (portal->from) {
        exit_to_room_index_to (portal->from->exit, NULL);
        portal->from = NULL;
    }

    /* The "from" portal exit cannot be from a room. */
    BAIL_IF_BUGF (pex != NULL && pex->exit == NULL,
        "portal_to_portal_exit_from: From '%s' is a room",
        pex->room->name);

    str_replace_dup (&(portal->name_from), pex ? pex->name : NULL);
    portal->from = pex;
}

void portal_to_portal_exit_to (PORTAL_T *portal, PORTAL_EXIT_T *pex) {
    if (portal->to) {
        if (portal->to->exit)
            exit_to_room_index_to (portal->to->exit, NULL);
        portal->to = NULL;
        portal->two_way = FALSE;
    }

    str_replace_dup (&(portal->name_to), pex ? pex->name : NULL);
    portal->to = pex;
    portal->two_way = (pex && pex->exit) ? TRUE : FALSE;
}

static int new_exit_count,   new_portal_count;
static int new_single_count, new_two_way_count;

void portal_create_missing_all (void) {
    ROOM_INDEX_T *r;

    new_exit_count    = 0;
    new_portal_count  = 0;
    new_single_count  = 0;
    new_two_way_count = 0;

    for (r = room_index_get_first(); r; r = room_index_get_next (r))
        portal_create_missing_room (r);

    if (new_exit_count > 0 || new_portal_count > 0) {
        log_f ("Created %d portal exit(s) based on exits to other areas",
            new_exit_count);
    }
    if (new_portal_count > 0) {
        log_f ("Created %d portal(s) based on exits to other areas",
            new_portal_count);
        log_f ("    (%d one-way portals, %d two-way portals)",
            new_single_count, new_two_way_count);
    }
}

void portal_create_missing_room (ROOM_INDEX_T *room) {
    EXIT_T *exit;
    int dir;

    for (dir = 0; dir < DIR_MAX; dir++) {
        if ((exit = room_get_orig_exit (room, dir)) == NULL)
            continue;
        portal_create_missing_exit (exit);
    }
}

void portal_create_missing_exit (EXIT_T *exit_from) {
    ROOM_INDEX_T *room_from, *room_to;
    EXIT_T *exit_to;
    PORTAL_EXIT_T *pex_from, *pex_to;
    PORTAL_T *portal;
    int dir, rev;
    bool two_way;

    room_from = exit_from->from_room;
    if (exit_from->portal)
        return;
    if ((exit_from->to_vnum >= room_from->area->min_vnum &&
         exit_from->to_vnum <= room_from->area->max_vnum))
        return;

    /* assign our portal. */
    pex_from = portal_exit_create_on_exit (exit_from);
    new_exit_count++;

    /* if the connecting room couldn't be find, don't create a portal
     * for it - preserve the portal exit, however. */
    if (exit_from->to_room == NULL)
        return;

    /* Find an exit, but only use it if it's matching. */
    room_to = exit_from->to_room;
    dir     = exit_from->orig_door;
    rev     = REV_DIR(dir);
    exit_to = room_get_orig_exit (room_to, rev);
    if (exit_to != NULL && exit_to->to_room != room_from)
        exit_to = NULL;

    /* One-way portals: create a portal exit in the target room. */
    if (exit_to == NULL) {
        if (room_to->portal == NULL) {
            portal_exit_create_on_room (room_to);
            new_exit_count++;
        }
        pex_to = room_to->portal;
        two_way = FALSE;
    }
    /* Two-way portals: create a portal exit on the opposite exit. */
    else {
        if (exit_to->portal == NULL) {
            portal_exit_create_on_exit (exit_to);
            new_exit_count++;
        }
        pex_to = exit_to->portal;
        two_way = TRUE;
    }

    /* Use an existing portal if it exists. Otherwise, create one. */
    portal = portal_get_by_exit_names (pex_from->name, pex_to->name, two_way);
    if (portal == NULL) {
        portal = portal_new ();
        new_portal_count++;
        if (two_way)
            new_two_way_count++;
        else
            new_single_count++;
    }

    /* Link the portal to the portal exits. */
    portal_to_portal_exits (portal, pex_from, pex_to);
}

PORTAL_T *portal_get_by_exit_names (const char *from, const char *to,
    bool two_way)
{
    PORTAL_T *p;

    /* Names must exist. */
    if (from == NULL || to == NULL)
        return NULL;

    /* Find the first portal with matching parameters. */
    for (p = portal_get_first(); p != NULL; p = portal_get_next (p)) {
        if (p->name_from == NULL || p->name_to == NULL)
            continue;
        if (p->two_way != two_way)
            continue;

        /* "From" -> "To" is always valid... */
        if (strcmp (p->name_from, from) == 0 &&
            strcmp (p->name_to,   to)   == 0)
        {
            return p;
        }
        /* ...and "To" -> "From" is valid, but only for two-way portals. */
        if (two_way &&
            strcmp (p->name_to,   from) == 0 &&
            strcmp (p->name_from, to)   == 0)
        {
            return p;
        }
    }

    /* No match found. */
    return NULL;
}

#if 0
void portal_shuffle_all (void) {
    PORTAL_T *p1, *p2, *next;
    int count;

    for (p1 = portal_get_first(); p1 != NULL; p1 = next) {
        next = portal_get_next(p1);
        if (!next)
            continue;
        if (!p1->opposite || p1->from->dir == DIR_NONE)
            continue;
        if (p1->from->dir == DIR_DOWN || p1->from->dir == DIR_SOUTH ||
            p1->from->dir == DIR_WEST)
            continue;

        count = 0;
        for (p2 = next; p2 != NULL; p2 = portal_get_next(p2)) {
            if (!p2->opposite || p2->opposite == p1 ||
                 p2->to->dir != p1->from->dir ||
                 p1->from->room->area == p2->from->room->area)
                continue;
            count++;
        }
        if (count == 0)
            continue;

        count = number_range (1, count);
        for (p2 = next; ; p2 = portal_get_next(p2)) {
            if (!p2->opposite || p2->opposite == p1 ||
                 p2->to->dir != p1->from->dir ||
                 p1->from->room->area == p2->from->room->area)
                continue;
            if (--count == 0) {
                PORTAL_T *tmp1 = p1->opposite;
                PORTAL_T *tmp2 = p2->opposite;

                /* Link P1 and P2 together. */
                p1->opposite = p2;
                p2->opposite = p1;
                p1->to = p2->from;
                p2->to = p1->from;

                /* Link the original opposite locations of each together. */
                tmp1->opposite = tmp2;
                tmp2->opposite = tmp1;
                tmp1->to = tmp2->from;
                tmp2->to = tmp1->from;
                break;
            }
        }
    }

    for (p1 = portal_get_first(); p1 != NULL; p1 = next) {
        next = portal_get_next(p1);
        if (!next)
            continue;
        if (!p1->opposite || p1->from->dir == DIR_NONE)
            continue;
        if (p1->from->dir == DIR_DOWN || p1->from->dir == DIR_SOUTH ||
            p1->from->dir == DIR_WEST)
            continue;

        p1->from->room->exit[p1->from->dir]->to_room   = p1->to  ->room;
        p1->from->room->exit[p1->from->dir]->vnum      = p1->to  ->room->vnum;
        p1->from->room->exit[p1->from->dir]->area_vnum = p1->to  ->room->area->vnum;
        p1->from->room->exit[p1->from->dir]->room_anum = p1->to  ->room->anum;
        p1->to  ->room->exit[p1->to  ->dir]->to_room   = p1->from->room;
        p1->to  ->room->exit[p1->to  ->dir]->vnum      = p1->from->room->vnum;
        p1->to  ->room->exit[p1->to  ->dir]->area_vnum = p1->from->room->area->vnum;
        p1->to  ->room->exit[p1->to  ->dir]->room_anum = p1->from->room->anum;
        log_f ("Portals shuffled: [%s <-> %s]", p1->from->name, p1->to->name);
    }
}
#endif

void portal_exit_unlink (PORTAL_EXIT_T *portal_exit) {
    if (portal_exit->exit) {
        portal_exit->exit->portal = NULL;
        portal_exit->exit = NULL;
    }
    else if (portal_exit->room) {
        portal_exit->room->portal = NULL;
        portal_exit->room = NULL;
    }
}

void portal_exit_to_room (PORTAL_EXIT_T *portal_exit, ROOM_INDEX_T *room) {
    portal_exit_unlink (portal_exit);
    portal_exit->room = room;
    if (room != NULL)
        room->portal = portal_exit;
}

void portal_exit_to_exit (PORTAL_EXIT_T *portal_exit, EXIT_T *exit) {
    portal_exit_unlink (portal_exit);
    portal_exit->exit = exit;
    if (exit != NULL)
        exit->portal = portal_exit;
}

void portal_free_all_with_portal_exit (PORTAL_EXIT_T *pex) {
    PORTAL_T *p, *p_next;

    for (p = portal_get_first(); p; p = p_next) {
        p_next = portal_get_next (p);
        if (p->from == pex || p->to == pex)
            portal_free (p);
    }
}

bool portal_exit_is_in_area (const PORTAL_EXIT_T *pex, const AREA_T *area) {
    const AREA_T *pex_area = pex->room
        ? pex->room->area
        : pex->exit->from_room->area;
    return (pex_area == area);
}

int portal_is_in_area (const PORTAL_T *portal, const AREA_T *area) {
    AREA_T *from_area, *to_area;
    int result;

    from_area = (portal->from == NULL )
        ? NULL : (portal->from->room)
            ? portal->from->room->area
            : portal->from->exit->from_room->area;
    to_area = (portal->to == NULL )
        ? NULL : (portal->to->room)
            ? portal->to->room->area
            : portal->to->exit->from_room->area;

    result = 0;
    if (from_area == area)
        result |= PORTAL_IN_AREA_FROM;
    if (to_area == area)
        result |= PORTAL_IN_AREA_TO;
    return result;
}

bool portal_can_enter_from_portal_exit (const PORTAL_T *portal,
    const PORTAL_EXIT_T *pex)
{
    return ((portal->from == pex) || (portal->two_way && portal->to == pex))
        ? TRUE : FALSE;
}

PORTAL_T *portal_get_with_outgoing_portal_exit (const PORTAL_EXIT_T *pex) {
    PORTAL_T *p;

    if (!pex->exit)
        return NULL;
    for (p = portal_get_first(); p; p = portal_get_next (p))
        if (portal_can_enter_from_portal_exit (p, pex))
            return p;
    return NULL;
}

bool portal_exit_rename (PORTAL_EXIT_T *pex, const char *new_name) {
    PORTAL_T *p;

    if (pex == NULL || new_name == NULL)
        return FALSE;

    /* Just return success if the name is the same. */
    if (new_name == pex->name || strcmp (new_name, pex->name) == 0)
        return TRUE;

    /* Don't allow duplicate names. */
    if (portal_exit_lookup_exact (new_name))
        return FALSE;

    /* Change the name and any portal that references it. */
    str_replace_dup (&pex->name, new_name);
    for (p = portal_get_first(); p; p = portal_get_next (p)) {
        if (p->to == pex)
            str_replace_dup (&(p->name_to), new_name);
        if (p->from == pex)
            str_replace_dup (&(p->name_from), new_name);
    }
    return TRUE;
}
