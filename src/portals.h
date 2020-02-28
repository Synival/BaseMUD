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

#ifndef __ROM_PORTALS_H
#define __ROM_PORTALS_H

#include "merc.h"

/* Function prototypes. */
PORTAL_EXIT_T *portal_exit_new_from_room (ROOM_INDEX_T *room, int dir);
void portal_assign (PORTAL_T *portal, PORTAL_EXIT_T *pex_from,
    PORTAL_EXIT_T *pex_to, bool two_way);
void portal_create_missing (void);
#if 0
void portal_shuffle_all (void);
#endif
void portal_link_to_assignment (PORTAL_T *portal);
void portal_link_exits (void);
PORTAL_EXIT_T *portal_exit_create (const char *name, ROOM_INDEX_T *room,
    int dir);
PORTAL_T *portal_get_by_exits (const char *from, const char *to, bool two_way);

#endif
