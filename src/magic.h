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

#ifndef __ROM_MAGIC_H
#define __ROM_MAGIC_H

#include "merc.h"

/* Function prototypes. */
int find_spell (CHAR_T *ch, const char *name);
void say_spell (CHAR_T *ch, int sn, int class);
void say_spell_name (CHAR_T *ch, const char *name, int class);
bool saves_spell (int level, CHAR_T *victim, int dam_type);
bool saves_dispel (int dis_level, int spell_level, int duration);
bool check_dispel_act (int dis_level, CHAR_T *victim, int sn,
    char *act_to_room);
bool check_dispel (int dis_level, CHAR_T *victim, int sn);
bool check_dispel_quick (int dis_level, CHAR_T *victim, char *skill,
    char *act_to_room);
int mana_cost (CHAR_T *ch, int min_mana, int level);
bool spell_fight_back_if_possible (CHAR_T *ch, CHAR_T *victim, int sn,
    int target);
void obj_cast_spell (int sn, int level, CHAR_T *ch, CHAR_T *victim,
    OBJ_T *obj);
int affect_is_char_affected_with_act (CHAR_T *victim, int sn, flag_t flag, CHAR_T *ch,
    char *to_self, char *to_victim);
int affect_isnt_char_affected_with_act (CHAR_T *victim, int sn, flag_t flag, CHAR_T *ch,
    char *to_self, char *to_victim);
void perform_breath_attack (CHAR_T *ch, ROOM_INDEX_T *room, CHAR_T *victim,
    int dam_type, int level, int dam, int sn);

/* Internal spells. */
DECLARE_SPELL_FUN (spell_null);

#endif
