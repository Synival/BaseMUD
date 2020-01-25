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

#ifndef __ROM_FIGHT_H
#define __ROM_FIGHT_H

#include "merc.h"

/* Function prototypes. */
int should_assist_group (CHAR_T *bystander, CHAR_T *attacker, CHAR_T *victim);
CHAR_T *random_group_target_in_room (CHAR_T *bystander, CHAR_T *ch);
void check_assist (CHAR_T *ch, CHAR_T *victim);
void multi_hit (CHAR_T *ch, CHAR_T *victim, int dt);
void one_hit (CHAR_T *ch, CHAR_T *victim, int dt);
bool damage_quiet (CHAR_T *ch, CHAR_T *victim, int dam, int dt, int dam_type);
bool damage_visible (CHAR_T *ch, CHAR_T *victim, int dam, int dt, int dam_type,
    const char *damage_adj);
bool damage_real (CHAR_T *ch, CHAR_T *victim, int dam, int dt, int dam_type,
    bool show, const char *damage_adj);
bool set_fighting_position_if_possible (CHAR_T *ch);
bool do_filter_can_attack (CHAR_T *ch, CHAR_T *victim);
bool do_filter_can_attack_spell (CHAR_T *ch, CHAR_T *victim, bool area);
bool can_attack (CHAR_T *ch, CHAR_T *victim);
bool can_attack_spell (CHAR_T *ch, CHAR_T *victim, bool area);
bool do_filter_can_attack_real (CHAR_T *ch, CHAR_T *victim, bool area,
    bool quiet);
void check_killer (CHAR_T *ch, CHAR_T *victim);
bool check_parry (CHAR_T *ch, CHAR_T *victim);
bool check_shield_block (CHAR_T *ch, CHAR_T *victim);
bool check_dodge (CHAR_T *ch, CHAR_T *victim);
void update_pos (CHAR_T *victim);
void set_fighting_both (CHAR_T *ch, CHAR_T *victim);
void set_fighting_one (CHAR_T *ch, CHAR_T *victim);
void stop_fighting_one (CHAR_T *ch);
void stop_fighting (CHAR_T *ch, bool both);
OBJ_T *make_corpse (CHAR_T *ch);
void death_cry (CHAR_T *ch);
OBJ_T *raw_kill (CHAR_T *victim);
void group_gain (CHAR_T *ch, CHAR_T *victim);
int fight_compute_kill_exp (CHAR_T *gch, CHAR_T *victim, int total_levels);
void dam_message (CHAR_T *ch, CHAR_T *victim, int dam, int dt, bool immune,
    int orig_dam, const char *damage_adj);
void disarm (CHAR_T *ch, CHAR_T *victim);

#endif
