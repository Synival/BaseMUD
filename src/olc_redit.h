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

#define REDIT(fun)          bool fun(CHAR_DATA *ch, char *argument)
#define EDIT_ROOM(Ch, Room) (Room = Ch->in_room)

/* Sub-routines and filters. */
void redit_add_reset (ROOM_INDEX_DATA * room, RESET_DATA * pReset, int index);
bool redit_change_exit (CHAR_DATA * ch, char *argument, int door);

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

#endif
