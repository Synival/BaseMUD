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

#ifndef __ROM_WIZ_L5_H
#define __ROM_WIZ_L5_H

#include "merc.h"

/* Sub-routines and filters. */
bool do_obj_load_check (CHAR_T *ch, OBJ_T *obj);
void do_clone_recurse (CHAR_T *ch, OBJ_T *obj, OBJ_T *clone);
bool do_string_char (CHAR_T *ch, char *arg1, char *arg2, char *arg3, char *argument);
bool do_string_obj (CHAR_T *ch, char *arg1, char *arg2, char *arg3, char *argument);

/* Commands. */
DECLARE_DO_FUN (do_nochannels);
DECLARE_DO_FUN (do_noemote);
DECLARE_DO_FUN (do_noshout);
DECLARE_DO_FUN (do_notell);
DECLARE_DO_FUN (do_transfer);
DECLARE_DO_FUN (do_peace);
DECLARE_DO_FUN (do_snoop);
DECLARE_DO_FUN (do_string);
DECLARE_DO_FUN (do_clone);
DECLARE_DO_FUN (do_newlock);

#endif
