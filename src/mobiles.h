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

#ifndef __ROM_MOBILES_H
#define __ROM_MOBILES_H

#include "merc.h"

/* Function prototypes. */
MOB_INDEX_T *mobile_get_index (int vnum);
CHAR_T *mobile_create (MOB_INDEX_T *mob_index);
void mobile_clone (CHAR_T *parent, CHAR_T *clone);
int mobile_should_assist_player (CHAR_T *bystander, CHAR_T *player,
    CHAR_T *victim);
bool mobile_should_assist_attacker (CHAR_T *bystander, CHAR_T *attacker,
    CHAR_T *victim);
void mobile_hit (CHAR_T *ch, CHAR_T *victim, int dt);
int mobile_get_skill_learned (const CHAR_T *ch, int sn);
bool mobile_is_friendly (const CHAR_T *ch);

#endif
