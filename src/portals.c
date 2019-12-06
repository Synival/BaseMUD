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
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 ***************************************************************************/

#include "lookup.h"
#include "recycle.h"
#include "utils.h"
#include "memory.h"

#include "portals.h"

PORTAL_EXIT_T *portal_exit_new_from_room (ROOM_INDEX_T *room, int dir) {
    PORTAL_EXIT_T *pex;
    char buf[256];
    int num = 1;

    if (dir == DIR_NONE) {
        do {
            snprintf (buf, sizeof(buf), "%s-room-%d", room->area->name, num++);
        } while (portal_exit_lookup_exact (buf));
    }
    else {
        do {
            snprintf (buf, sizeof(buf), "%s-%s-%d", room->area->name,
                door_get_name(dir), num++);
        } while (portal_exit_lookup_exact (buf));
    }

    pex = portal_exit_new ();
    str_replace_dup (&pex->name, buf);
    pex->room = room;
    pex->dir  = dir;
    return pex;
}

void portal_assign (PORTAL_T *portal, PORTAL_EXIT_T *pex_from,
    PORTAL_EXIT_T *pex_to)
{
    str_replace_dup (&(portal->name_from), pex_from->name);
    str_replace_dup (&(portal->name_to),   pex_to->name);

    portal->from = pex_from;
    portal->to   = pex_to;

    /* If this new portal has found an opposite, mark it as
     * 'generated' so we don't save multiple two-way portals. */
    if (portal_link_opposites (portal))
        portal->generated = TRUE;
}

void portal_link_to_assignment (PORTAL_T *portal)
{
    PORTAL_EXIT_T *from, *to;
    EXIT_T *exit_from, *exit_to;

    if (portal->name_from == NULL || portal->name_to == NULL) {
        printf ("no\n");
        return;
    }

    /* Make sure both portal points exist. */
    BAIL_IF_BUGF ((from = portal_exit_lookup_exact (portal->name_from)) == NULL,
        "portal_link_to_assignment(): Cannot find portal exit from '%s'\n",
        portal->name_from);
    BAIL_IF_BUGF ((to = portal_exit_lookup_exact (portal->name_to)) == NULL,
        "portal_link_to_assignment(): Cannot find portal exit to '%s'\n",
        portal->name_to);

    BAIL_IF_BUGF (from->dir == DIR_NONE,
        "portal_link_to_assignment(): From '%s' is a room",
        from->room->name, from->name);
    BAIL_IF_BUGF ((exit_from = from->room->exit[from->dir]) == NULL,
        "portal_link_to_assignment(): Room %d does not have exit %d\n",
        from->room->vnum, from->dir);

    if (portal->two_way) {
        BAIL_IF_BUGF (to->dir == DIR_NONE,
            "portal_link_to_assignment(): To '%s' is a room",
            to->room->name, to->name);
        BAIL_IF_BUGF ((exit_to = to->room->exit[to->dir]) == NULL,
            "portal_link_to_assignment(): Room %d does not have exit %d\n",
            to->room->vnum, to->dir);

        exit_from->to_room = to->room;
        exit_from->vnum    = to->room->vnum;
        exit_to->to_room = from->room;
        exit_to->vnum    = from->room->vnum;
    }
    else {
        exit_from->to_room = to->room;
        exit_from->vnum    = to->room->vnum;
    }

    portal->from = from;
    portal->to   = to;
}

bool portal_link_opposites (PORTAL_T *portal) {
    PORTAL_T *p;

    if (portal->from == NULL || portal->to == NULL)
        return FALSE;
    if (portal->two_way == TRUE)
        return FALSE;
    if (portal->from->dir == DIR_NONE || portal->to->dir == DIR_NONE)
        return FALSE;
    if (REV_DIR(portal->from->dir) != portal->to->dir)
        return FALSE;

    for (p = portal_get_first(); p != NULL; p = portal_get_next(p)) {
        if (p == portal || p->two_way == TRUE)
            continue;
        if (p->to == portal->from && p->from == portal->to) {
            portal->two_way = TRUE;
            p->two_way = TRUE;
            return TRUE;
        }
    }

    return FALSE;
}

void portal_create_missing (void) {
    ROOM_INDEX_T *room_from, *room_to;
    EXIT_T *exit_from, *exit_to;
    PORTAL_EXIT_T *pex_from, *pex_to;
    PORTAL_T *portal;
    int dir, rev, count;

    count = 0;
    for (room_from = room_index_get_first(); room_from != NULL;
         room_from = room_index_get_next (room_from))
    {
        for (dir = 0; dir <= 5; dir++) {
            if ((exit_from = room_from->exit[dir]) == NULL)
                continue;
            if (exit_from->portal)
                continue;
            if ((exit_from->vnum >= room_from->area->min_vnum &&
                 exit_from->vnum <= room_from->area->max_vnum))
                continue;

            /* assign our portal. */
            exit_from->portal = portal_exit_new_from_room (room_from, dir);
            if (exit_from->to_room == NULL)
                continue;
            pex_from = exit_from->portal;

            /* Find an exit, but only if it's matching. */
            room_to = exit_from->to_room;
            rev     = REV_DIR(dir);
            exit_to = room_to->exit[rev];
            if (exit_to != NULL && exit_to->to_room != room_from)
                exit_to = NULL;

            if (exit_to == NULL) {
                if (room_to->portal == NULL)
                    room_to->portal = portal_exit_new_from_room (
                        room_to, DIR_NONE);
                pex_to = room_to->portal;
            }
            else {
                if (exit_to->portal == NULL) {
                    exit_to->portal = portal_exit_new_from_room (
                        room_to, rev);
                    portal = portal_new ();
                    portal_assign (portal, exit_to->portal, pex_from);
                    count++;
                }
                pex_to = exit_to->portal;
            }

            portal = portal_new ();
            portal_assign (portal, pex_from, pex_to);
            count++;
        }
    }

    log_f ("Created %d missing portals", count);
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

void portal_link_exits (void) {
    PORTAL_T *portal;
    for (portal = portal_get_first(); portal; portal = portal_get_next (portal))
        portal_link_to_assignment (portal);
}

PORTAL_EXIT_T *portal_exit_create (const char *name, ROOM_INDEX_T *room,
    int dir)
{
    PORTAL_EXIT_T *pex;
    RETURN_IF_BUGF ((pex = portal_exit_lookup_exact (name)) != NULL, NULL,
        "portal_exit_create(): Portal '%s' for '%s' dir=%d already exists "
        "for '%s', dir=%d\n",
        name, room->name, dir,
        pex->name, pex->room->name, pex->dir);

    pex = portal_exit_new ();
    str_replace_dup (&pex->name, name);
    pex->room = (ROOM_INDEX_T *) room;
    pex->dir = dir;
    return pex;
}
