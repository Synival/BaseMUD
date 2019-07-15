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
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

#ifndef __ROM_ROOMS_H
#define __ROM_ROOMS_H

#include "merc.h"

/* Room functions. */
bool room_is_dark (ROOM_INDEX_DATA * pRoomIndex);
bool room_is_private (ROOM_INDEX_DATA * pRoomIndex);
char room_colour_char (ROOM_INDEX_DATA * room);
void room_add_money (ROOM_INDEX_DATA *room, int gold, int silver);
bool room_is_owner (ROOM_INDEX_DATA *room, CHAR_DATA *ch);
EXIT_DATA *room_get_opposite_exit (ROOM_INDEX_DATA *from_room, int dir,
    ROOM_INDEX_DATA **out_room);

/* Utilities. */
char *door_keyword_to_name (const char *keyword, char *out_buf, size_t size);

#endif
