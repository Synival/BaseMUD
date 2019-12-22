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

#ifndef __ROM_FIND_H
#define __ROM_FIND_H

#include "merc.h"

/* Relative-to-char lookup functions. */
int find_number_argument (const char *arg_in, char *arg_out);
void find_continue_counting (void);
void find_stop_counting (void);

ROOM_INDEX_T *find_location (CHAR_T *ch, const char *arg);

CHAR_T *find_char_same_room (CHAR_T *ch, const char *argument);
CHAR_T *find_char_world (CHAR_T *ch, const char *argument);

OBJ_T *find_obj_room (CHAR_T *ch, ROOM_INDEX_T *room, const char *argument);
OBJ_T *find_obj_same_room (CHAR_T *ch, const char *argument);
OBJ_T *find_obj_container (CHAR_T *ch, OBJ_T *obj, const char *argument);
OBJ_T *find_obj_char (CHAR_T *ch, CHAR_T *victim, const char *argument);
OBJ_T *find_obj_worn (CHAR_T *ch, CHAR_T *victim, const char *argument);
OBJ_T *find_obj_inventory (CHAR_T *ch, CHAR_T *victim, const char *argument);
OBJ_T *find_obj_own_char (CHAR_T *ch, const char *argument);
OBJ_T *find_obj_own_inventory (CHAR_T *ch, const char *argument);
OBJ_T *find_obj_own_worn (CHAR_T *ch, const char *argument);

OBJ_T *find_obj_list (CHAR_T *ch, OBJ_T *list, const char *argument, int worn);
OBJ_T *find_obj_here (CHAR_T *ch, const char *argument);
OBJ_T *find_obj_world (CHAR_T *ch, const char *argument);
OBJ_T *find_obj_keeper (CHAR_T *ch, CHAR_T *keeper, const char *argument);

int find_door_same_room (CHAR_T *ch, const char *argument);

#endif
