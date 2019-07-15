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
*    ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
*    ROM has been brought to you by the ROM consortium                      *
*        Russ Taylor (rtaylor@hypercube.org)                                *
*        Gabrielle Taylor (gtaylor@hypercube.org)                           *
*        Brian Moore (zump@rom.org)                                         *
*    By using this code, you have agreed to follow the terms of the         *
*    ROM license, in the file Rom24/doc/rom.license                         *
****************************************************************************/

#ifndef __ROM_FIGHT_H
#define __ROM_FIGHT_H

#include "merc.h"

/* Function prototypes. */
void advance_level (CHAR_DATA * ch, bool hide);
void gain_exp (CHAR_DATA * ch, int gain);
int should_assist_group (CHAR_DATA * bystander, CHAR_DATA * attacker,
    CHAR_DATA *victim);
int npc_should_assist_player (CHAR_DATA * bystander, CHAR_DATA * player,
    CHAR_DATA *victim);
bool npc_should_assist_attacker (CHAR_DATA * bystander, CHAR_DATA * attacker,
    CHAR_DATA * victim);
CHAR_DATA * random_group_target_in_room (CHAR_DATA * bystander,
    CHAR_DATA * ch);
void check_assist (CHAR_DATA * ch, CHAR_DATA * victim);
void multi_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt);
void mob_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt);
void one_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt);
bool damage (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
             int dam_type, bool show);
bool set_fighting_position_if_possible (CHAR_DATA * ch);
bool is_safe (CHAR_DATA * ch, CHAR_DATA * victim);
bool is_safe_spell (CHAR_DATA * ch, CHAR_DATA * victim, bool area);
void check_killer (CHAR_DATA * ch, CHAR_DATA * victim);
bool check_parry (CHAR_DATA * ch, CHAR_DATA * victim);
bool check_shield_block (CHAR_DATA * ch, CHAR_DATA * victim);
bool check_dodge (CHAR_DATA * ch, CHAR_DATA * victim);
void update_pos (CHAR_DATA * victim);
void set_fighting_both (CHAR_DATA * ch, CHAR_DATA * victim);
void set_fighting_one (CHAR_DATA * ch, CHAR_DATA * victim);
void stop_fighting_one (CHAR_DATA * ch);
void stop_fighting (CHAR_DATA * ch, bool fBoth);
void make_corpse (CHAR_DATA * ch);
void death_cry (CHAR_DATA * ch);
void raw_kill (CHAR_DATA * victim);
void group_gain (CHAR_DATA * ch, CHAR_DATA * victim);
int xp_compute (CHAR_DATA * gch, CHAR_DATA * victim, int total_levels);
void dam_message (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
                  bool immune, int orig_dam);
void disarm (CHAR_DATA * ch, CHAR_DATA * victim);
int get_exp_to_level (CHAR_DATA * ch);
int exp_per_level (CHAR_DATA * ch, int points);

#endif
