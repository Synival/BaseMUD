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

#ifndef __ROM_SPELL_INFO_H
#define __ROM_SPELL_INFO_H

#include "merc.h"

/* Helper functions. */
long int spell_identify_seed (CHAR_T *ch, OBJ_T *obj);
int spell_identify_know_check (CHAR_T *ch, OBJ_T *obj, int pos,
    int skill, int *know_count);
void spell_identify_perform (CHAR_T *ch, OBJ_T *obj, int power);
void spell_identify_perform_seeded (CHAR_T *ch, OBJ_T *obj, int power);
const char *spell_identify_know_message (int percent);

/* Spell declarations. */
DECLARE_SPELL_FUN (spell_detect_poison);
DECLARE_SPELL_FUN (spell_identify);
DECLARE_SPELL_FUN (spell_know_alignment);
DECLARE_SPELL_FUN (spell_locate_object);
DECLARE_SPELL_FUN (spell_farsight);

#endif
