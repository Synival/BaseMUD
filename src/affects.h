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

#ifndef __ROM_AFFECTS_H
#define __ROM_AFFECTS_H

#include "merc.h"

/* Affect creation and removal. */
void affect_init (AFFECT_T *af, sh_int bit_type, sh_int type, sh_int level,
    sh_int duration, sh_int apply, sh_int modifier, flag_t bits);
void affect_copy_to_char (AFFECT_T *paf, CHAR_T *ch);
void affect_copy_to_obj (AFFECT_T *paf, OBJ_T *obj);
void affect_remove (AFFECT_T *paf);
void affect_remove_char (AFFECT_T *paf);
void affect_remove_obj (AFFECT_T *paf);
void affect_strip_char (CHAR_T *ch, int sn);
void affect_copy (AFFECT_T *dest, AFFECT_T *src);
void affect_join_char (AFFECT_T *paf, CHAR_T *ch);

/* Link management. */
void affect_to_char_front (AFFECT_T *paf, CHAR_T *ch);
void affect_to_char_back (AFFECT_T *paf, CHAR_T *ch);
void affect_to_obj_front (AFFECT_T *paf, OBJ_T *obj);
void affect_to_obj_back (AFFECT_T *paf, OBJ_T *obj);
void affect_to_obj_index_back (AFFECT_T *paf, OBJ_INDEX_T *obj_index);
void affect_unlink (AFFECT_T *paf);

/* Character/object modification. */
void affect_modify_char_bits (AFFECT_T *paf, CHAR_T *ch, bool on);
void affect_modify_char_apply (AFFECT_T *paf, CHAR_T *ch, bool on);
void affect_modify_char (AFFECT_T *paf, CHAR_T *ch, bool on);
void affect_modify_obj (AFFECT_T *paf, OBJ_T *obj);
void affect_check_char (CHAR_T *ch, int bit_type, flag_t bits);

/* Utility functions. */
AFFECT_T *affect_find (AFFECT_T *paf, int sn);
char *affect_bit_message (int bit_type, flag_t bits);
bool affect_is_char_affected (CHAR_T *ch, int sn);

#endif
