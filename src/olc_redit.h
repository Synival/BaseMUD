/***************************************************************************
 *  File: olc_redit.h                                                      *
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

#ifndef __ROM_OLC_REDIT_H
#define __ROM_OLC_REDIT_H

#include "merc.h"

#define REDIT(fun)          bool fun(CHAR_T *ch, char *argument)
#define EDIT_ROOM(ch, room) (room = ch->in_room)

/* Sub-routines and filters. */
bool redit_change_exit (CHAR_T *ch, char *argument, int door);
void redit_change_exit_free (EXIT_T *ex);
void redit_change_exit_update_portals (EXIT_T *ex, ROOM_INDEX_T *old_room,
    ROOM_INDEX_T *old_rev_rev_room, CHAR_T *ch);
void redit_change_exit_update_portal (EXIT_T *ex, CHAR_T *ch);
void redit_change_exit_connect_portal_exits (PORTAL_EXIT_T *from,
    PORTAL_EXIT_T *to, CHAR_T *ch);
void redit_change_exit_remove_portal (PORTAL_T *portal, CHAR_T *ch);

/* Commands (rooms). */
REDIT (redit_rlist);
REDIT (redit_mlist);
REDIT (redit_olist);
REDIT (redit_mshow);
REDIT (redit_oshow);
REDIT (redit_show);
REDIT (redit_north);
REDIT (redit_south);
REDIT (redit_east);
REDIT (redit_west);
REDIT (redit_up);
REDIT (redit_down);
REDIT (redit_ed);
REDIT (redit_create);
REDIT (redit_name);
REDIT (redit_desc);
REDIT (redit_heal);
REDIT (redit_mana);
REDIT (redit_clan);
REDIT (redit_format);
REDIT (redit_mreset);
REDIT (redit_oreset);
REDIT (redit_owner);
REDIT (redit_room);
REDIT (redit_sector);
REDIT (redit_portal);

#endif
