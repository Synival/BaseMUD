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

#include "portals.h"

/* TODO: use common code for finding opposite exits */

PORTAL_EXIT_TYPE *portal_exit_new_from_room (ROOM_INDEX_DATA *room, int dir) {
    PORTAL_EXIT_TYPE *pex;
    char buf[256];
    int num;

    num = ++(room->area->portal_count);
    if (dir == DIR_NONE)
        snprintf (buf, sizeof(buf), "%s-%d", room->area->name, num);
    else {
        snprintf (buf, sizeof(buf), "%s-%d-%s", room->area->name,
            num, door_get_name(dir));
    }

    pex = portal_exit_new ();
    str_replace_dup (&pex->name, buf);
    pex->room = room;
    pex->dir  = dir;
    return pex;
}

void portal_assign (PORTAL_TYPE *portal, PORTAL_EXIT_TYPE *pex_from,
    PORTAL_EXIT_TYPE *pex_to)
{
    str_replace_dup (&(portal->name_from), pex_from->name);
    str_replace_dup (&(portal->name_to),   pex_to->name);

    portal->from = pex_from;
    portal->to   = pex_to;
    portal_link_opposites (portal);
}

bool portal_link_opposites (PORTAL_TYPE *portal) {
    PORTAL_TYPE *p;

    if (portal->from == NULL || portal->to == NULL)
        return FALSE;
    if (portal->opposite != NULL)
        return FALSE;
    if (portal->from->dir == DIR_NONE || portal->to->dir == DIR_NONE)
        return FALSE;
    if (REV_DIR(portal->from->dir) != portal->to->dir)
        return FALSE;

    for (p = portal_get_first(); p != NULL; p = portal_get_next(p)) {
        if (p == portal || p->opposite)
            continue;
        if (p->to == portal->from && p->from == portal->to) {
            portal->opposite = p;
            p->opposite = portal;
            return TRUE;
        }
    }

    return FALSE;
}

void portal_create_missing (void) {
    ROOM_INDEX_DATA *room_from, *room_to;
    EXIT_DATA *exit_from, *exit_to;
    PORTAL_EXIT_TYPE *pex_from, *pex_to;
    PORTAL_TYPE *portal;
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

void portal_shuffle_all (void) {
    PORTAL_TYPE *p1, *p2, *next;
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
                PORTAL_TYPE *tmp1 = p1->opposite;
                PORTAL_TYPE *tmp2 = p2->opposite;

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

        /* TODO: somehow make doors consistent. maybe swap them? */
    }
}
