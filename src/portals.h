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

/* Return values and other definitions. */
#define PORTAL_IN_AREA_FROM     0x01
#define PORTAL_IN_AREA_TO       0x02

/* Portal creation. */
PORTAL_EXIT_T *portal_exit_create (const char *name);
PORTAL_EXIT_T *portal_exit_create_on_room (ROOM_INDEX_T *room);
PORTAL_EXIT_T *portal_exit_create_on_exit (EXIT_T *exit);
void portal_create_missing_all (void);
void portal_create_missing_room (ROOM_INDEX_T *room);
void portal_create_missing_exit (EXIT_T *exit_from);

/* Modification. */
bool portal_exit_rename (PORTAL_EXIT_T *pex, const char *new_name);

/* Deletion. */
void portal_free_all_with_portal_exit (PORTAL_EXIT_T *pex);

/* Is / has / can functions. */
bool portal_exit_is_in_area (const PORTAL_EXIT_T *pex, const AREA_T *area);
int portal_is_in_area (const PORTAL_T *portal, const AREA_T *area);
bool portal_can_enter_from_portal_exit (const PORTAL_T *portal,
    const PORTAL_EXIT_T *pex);

/* Get / lookup functions. */
PORTAL_T *portal_get_by_exit_names (const char *from, const char *to,
    bool two_way);
PORTAL_T *portal_get_with_outgoing_portal_exit (const PORTAL_EXIT_T *pex);

/* Portal link functions. */
void portal_link_unassigned_by_names (void);
bool portal_to_portal_exits_by_name (PORTAL_T *portal);
void portal_to_portal_exits (PORTAL_T *portal, PORTAL_EXIT_T *pex_from,
    PORTAL_EXIT_T *pex_to);
void portal_to_portal_exit_from (PORTAL_T *portal, PORTAL_EXIT_T *pex);
void portal_to_portal_exit_to (PORTAL_T *portal, PORTAL_EXIT_T *pex);

/* Portal exit link functions. */
void portal_exit_unlink (PORTAL_EXIT_T *portal_exit);
void portal_exit_to_room (PORTAL_EXIT_T *portal_exit, ROOM_INDEX_T *room);
void portal_exit_to_exit (PORTAL_EXIT_T *portal_exit, EXIT_T *exit);

#if 0
    void portal_shuffle_all (void);
#endif

#endif
