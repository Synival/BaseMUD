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

#ifndef __ROM_ACT_OBJ_H
#define __ROM_ACT_OBJ_H

#include "merc.h"

#define OBJ_SINGLE  0
#define OBJ_ALL_OF  1
#define OBJ_ALL     2

/* Sub-routines and filters. */
void do_drop_single_item (CHAR_T *ch, OBJ_T *obj);
void do_put_single_item (CHAR_T *ch, OBJ_T *obj, OBJ_T *container);
bool do_filter_can_put_item (CHAR_T *ch, OBJ_T *obj, OBJ_T *container,
    bool msg);
bool do_filter_put_or_get_valid_container (CHAR_T *ch, char *arg,
    OBJ_T **out_container);
bool do_filter_can_give_item (CHAR_T *ch, OBJ_T *obj, CHAR_T *victim, bool msg);
void do_get_container (CHAR_T *ch, char *arg1, char *arg2);
void do_get_room (CHAR_T *ch, char *arg);
char *do_obj_parse_arg (const char *arg, int *out_type);
void do_drop_money (CHAR_T *ch, char *arg1, char *argument);
void do_give_money (CHAR_T *ch, char *arg1, char *arg2, char *argument);

/* Commands. */
DECLARE_DO_FUN (do_get);
DECLARE_DO_FUN (do_put);
DECLARE_DO_FUN (do_drop);
DECLARE_DO_FUN (do_give);
DECLARE_DO_FUN (do_envenom);
DECLARE_DO_FUN (do_fill);
DECLARE_DO_FUN (do_pour);
DECLARE_DO_FUN (do_drink);
DECLARE_DO_FUN (do_eat);
DECLARE_DO_FUN (do_wear);
DECLARE_DO_FUN (do_remove);
DECLARE_DO_FUN (do_sacrifice);
DECLARE_DO_FUN (do_quaff);
DECLARE_DO_FUN (do_recite);
DECLARE_DO_FUN (do_brandish);
DECLARE_DO_FUN (do_zap);
DECLARE_DO_FUN (do_steal);
DECLARE_DO_FUN (do_outfit);
DECLARE_DO_FUN (do_play);

#endif
