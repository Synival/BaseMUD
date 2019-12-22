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

int check_immune (CHAR_T *ch, int dam_type);
void affect_modify_bits (CHAR_T *ch, AFFECT_T *paf, bool on);
void affect_modify_apply (CHAR_T *ch, AFFECT_T *paf, bool on);
void affect_modify (CHAR_T *ch, AFFECT_T *paf, bool on);
AFFECT_T *affect_find (AFFECT_T *paf, int sn);
void affect_check (CHAR_T *ch, int where, flag_t bits);
void affect_copy (AFFECT_T *dest, AFFECT_T *src);
void affect_to_char (CHAR_T *ch, AFFECT_T *paf);
void affect_to_obj (OBJ_T *obj, AFFECT_T *paf);
void affect_remove (CHAR_T *ch, AFFECT_T *paf);
void affect_remove_obj (OBJ_T *obj, AFFECT_T *paf);
void affect_strip (CHAR_T *ch, int sn);
bool is_affected (CHAR_T *ch, int sn);
void affect_join (CHAR_T *ch, AFFECT_T *paf);
void affect_init (AFFECT_T *af, sh_int target, sh_int type, sh_int level,
    sh_int duration, sh_int apply, sh_int modifier, flag_t bits);
char *affect_bit_message (int bit_type, flag_t bits);

#endif
