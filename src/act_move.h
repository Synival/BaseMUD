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

#ifndef __ROM_ACT_MOVE_H
#define __ROM_ACT_MOVE_H

#include "merc.h"

/* Sub-routines and filters. */
int do_door_filter_find (CHAR_T *ch, char *argument);
bool do_door_filter_is_door (CHAR_T *ch, EXIT_T *pexit, OBJ_T *obj,
    flag_t *out_flags, bool *out_container, int *out_key);
bool do_door_filter_can_open   (CHAR_T *ch, EXIT_T *pexit, OBJ_T *obj);
bool do_door_filter_can_close  (CHAR_T *ch, EXIT_T *pexit, OBJ_T *obj);
bool do_door_filter_can_lock   (CHAR_T *ch, EXIT_T *pexit, OBJ_T *obj);
bool do_door_filter_can_unlock (CHAR_T *ch, EXIT_T *pexit, OBJ_T *obj);
bool do_door_filter_can_pick   (CHAR_T *ch, EXIT_T *pexit, OBJ_T *obj);
void do_open_object   (CHAR_T *ch, OBJ_T *obj);
void do_close_object  (CHAR_T *ch, OBJ_T *obj);
void do_unlock_object (CHAR_T *ch, OBJ_T *obj);
void do_lock_object   (CHAR_T *ch, OBJ_T *obj);
void do_pick_object   (CHAR_T *ch, OBJ_T *obj);
void do_open_door     (CHAR_T *ch, int door);
void do_close_door    (CHAR_T *ch, int door);
void do_unlock_door   (CHAR_T *ch, int door);
void do_lock_door     (CHAR_T *ch, int door);
void do_pick_door     (CHAR_T *ch, int door);
void do_door (CHAR_T *ch, char *argument, char *arg_msg,
    void (*func_obj)  (CHAR_T *, OBJ_T *),
    void (*func_door) (CHAR_T *, int));
bool do_filter_change_position (CHAR_T *ch, int pos, const char *same_msg);
void do_position_sub(CHAR_T *ch, const char *argument, int pos, bool stay_on,
    const char *msg_cant_on, const char *msg_cant);

/* Commands. */
DECLARE_DO_FUN (do_north);
DECLARE_DO_FUN (do_east);
DECLARE_DO_FUN (do_south);
DECLARE_DO_FUN (do_west);
DECLARE_DO_FUN (do_up);
DECLARE_DO_FUN (do_down);
DECLARE_DO_FUN (do_open);
DECLARE_DO_FUN (do_close);
DECLARE_DO_FUN (do_unlock);
DECLARE_DO_FUN (do_lock);
DECLARE_DO_FUN (do_pick);
DECLARE_DO_FUN (do_stand);
DECLARE_DO_FUN (do_rest);
DECLARE_DO_FUN (do_sit);
DECLARE_DO_FUN (do_sleep);
DECLARE_DO_FUN (do_wake);
DECLARE_DO_FUN (do_sneak);
DECLARE_DO_FUN (do_hide);
DECLARE_DO_FUN (do_visible);
DECLARE_DO_FUN (do_recall);
DECLARE_DO_FUN (do_enter);

#endif
