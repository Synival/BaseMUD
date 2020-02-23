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

#ifndef __ROM_CHARS_H
#define __ROM_CHARS_H

#include "merc.h"

/* Short-hand macros. */
#define IS_NPC(ch)           (char_is_npc (ch))
#define IS_IMMORTAL(ch)      (char_is_immortal (ch))
#define IS_HERO(ch)          (char_is_hero (ch))
#define IS_TRUSTED(ch,level) (char_is_trusted (ch, level))
#define IS_AFFECTED(ch, sn)  (char_is_affected ((ch), (sn)))
#define IS_SOBER(ch)         (char_is_sober (ch))
#define IS_DRUNK(ch)         (char_is_drunk (ch))
#define IS_THIRSTY(ch)       (char_is_thirsty (ch))
#define IS_QUENCHED(ch)      (char_is_quenched (ch))
#define IS_HUNGRY(ch)        (char_is_hungry (ch))
#define IS_FED(ch)           (char_is_fed (ch))
#define IS_FULL(ch)          (char_is_full (ch))
#define IS_PET(ch)           (char_is_pet (ch))
#define IS_GOOD(ch)          (char_is_good(ch))
#define IS_EVIL(ch)          (char_is_evil(ch))
#define IS_REALLY_GOOD(ch)   (char_is_really_good (ch))
#define IS_REALLY_EVIL(ch)   (char_is_really_evil (ch))
#define IS_NEUTRAL(ch)       (char_is_neutral (ch))
#define IS_OUTSIDE(ch)       (char_is_outside (ch))
#define IS_AWAKE(ch)         (char_is_awake (ch))
#define IS_SAME_ALIGN(ch1, ch2) \
    (char_is_same_align (ch1, ch2))
#define IS_SWITCHED(ch)      (char_is_switched (ch))
#define IS_BUILDER(ch, area) (char_is_builder (ch, area))

#define GET_AGE(ch)          (char_get_age (ch))
#define GET_AC(ch, type)     (char_get_ac (ch, type))
#define GET_HITROLL(ch)      (char_get_hitroll (ch))
#define GET_DAMROLL(ch)      (char_get_damroll (ch))
#define CH_VNUM(ch)          (char_get_vnum (ch))

#define WAIT_STATE(ch, npulse) (char_set_max_wait_state (ch, npulse))
#define DAZE_STATE(ch, npulse) (char_set_max_daze_state (ch, npulse))

#define PERS(ch)           (char_get_short_descr (ch))
#define PERS_AW(ch,looker) (char_get_look_short_descr_anywhere ((looker), (ch)))
#define PERS_IR(ch,looker) (char_get_look_short_descr ((looker), (ch)))

/* Thanks Dingo for making life a bit easier ;) */
#define CH(d)       ((d)->original ? (d)->original : (d)->character)
#define REAL_CH(ch) (((ch)->desc && (ch)->desc->original) \
    ? (ch)->desc->original : (ch))

/* Unused functions... */
#if 0
bool char_is_friend (CHAR_T *ch, CHAR_T *victim);
#endif

/* Creation / destruction. */
void char_extract (CHAR_T *ch);

/* "Is" / "Can" functions. */
bool char_is_trusted (const CHAR_T *ch, int level);
bool char_is_npc (const CHAR_T *ch);
bool char_is_immortal (const CHAR_T *ch);
bool char_is_hero (const CHAR_T *ch);
bool char_is_affected (const CHAR_T *ch, flag_t sn);
bool char_is_pet (const CHAR_T *ch);
bool char_is_good (const CHAR_T *ch);
bool char_is_evil (const CHAR_T *ch);
bool char_is_really_good (const CHAR_T *ch);
bool char_is_really_evil (const CHAR_T *ch);
bool char_is_neutral (const CHAR_T *ch);
bool char_is_outside (const CHAR_T *ch);
bool char_is_awake (const CHAR_T *ch);
bool char_is_same_align (const CHAR_T *ch1, const CHAR_T *ch2);
bool char_is_switched (const CHAR_T *ch);
bool char_is_builder (const CHAR_T *ch, const AREA_T *area);
bool char_can_see_room (const CHAR_T *ch, const ROOM_INDEX_T *room_index);
bool char_can_see_anywhere (const CHAR_T *ch, const CHAR_T *victim);
bool char_can_see_in_room (const CHAR_T *ch, const CHAR_T *victim);
bool char_can_see_obj (const CHAR_T *ch, const OBJ_T *obj);
bool char_can_drop_obj (const CHAR_T *ch, const OBJ_T *obj);
bool char_can_loot (const CHAR_T *ch, const OBJ_T *obj);
bool char_has_key (const CHAR_T *ch, int key);
bool char_has_available_wear_loc (const CHAR_T *ch, flag_t wear_loc);
bool char_has_available_wear_flag (const CHAR_T *ch, flag_t wear_flag);
bool char_has_active_light (const CHAR_T *ch);
bool char_has_boat (const CHAR_T *ch);

/* Condition checking functions. */
DECLARE_CONDITION_FUN (char_is_sober);
DECLARE_CONDITION_FUN (char_is_drunk);
DECLARE_CONDITION_FUN (char_is_thirsty);
DECLARE_CONDITION_FUN (char_is_quenched);
DECLARE_CONDITION_FUN (char_is_hungry);
DECLARE_CONDITION_FUN (char_is_fed);
DECLARE_CONDITION_FUN (char_is_full);

/* Getter functions. */
int char_get_vnum (const CHAR_T *ch);
OBJ_T *char_get_weapon (const CHAR_T *ch);
int char_get_weapon_sn (const CHAR_T *ch);
int char_get_weapon_skill (const CHAR_T *ch, int sn);
int char_get_trust (const CHAR_T *ch);
int char_get_curr_stat (const CHAR_T *ch, int stat);
int char_get_max_train (const CHAR_T *ch, int stat);
long int char_get_carry_weight (const CHAR_T *ch);
int char_get_max_carry_count (const CHAR_T *ch);
long int char_get_max_carry_weight (const CHAR_T *ch);
void char_get_who_string (const CHAR_T *ch, const CHAR_T *wch, char *buf,
    size_t len);
OBJ_T *char_get_eq_by_wear_loc (const CHAR_T *ch, flag_t wear_loc);
CHAR_T *char_get_keeper_room (const CHAR_T *ch);
CHAR_T *char_get_trainer_room (const CHAR_T *ch);
CHAR_T *char_get_practicer_room (const CHAR_T *ch);
CHAR_T *char_get_gainer_room (const CHAR_T *ch);
const char *char_get_class_name (const CHAR_T *ch);
const char *char_get_position_str (const CHAR_T *ch, int position,
    const OBJ_T *on, int with_punct);
int char_get_age (const CHAR_T *ch);
int char_get_ac (const CHAR_T *ch, int type);
int char_get_hitroll (const CHAR_T *ch);
int char_get_damroll (const CHAR_T *ch);
char *char_get_short_descr (const CHAR_T *ch);
char *char_get_look_short_descr_anywhere (const CHAR_T *looker,
    const CHAR_T *ch);
char *char_get_look_short_descr (const CHAR_T *looker, const CHAR_T *ch);
int char_format_exit_string (const CHAR_T *ch, const ROOM_INDEX_T *room,
    int mode, char *out_buf, size_t out_size);
int char_get_immunity (CHAR_T *ch, int dam_type);
OBJ_T *char_get_active_light (const CHAR_T *ch);
int char_get_skill (const CHAR_T *ch, int sn);

/* Stat bonuses. */
const STR_APP_T *char_get_curr_str_app (const CHAR_T *ch);
const INT_APP_T *char_get_curr_int_app (const CHAR_T *ch);
const WIS_APP_T *char_get_curr_wis_app (const CHAR_T *ch);
const DEX_APP_T *char_get_curr_dex_app (const CHAR_T *ch);
const CON_APP_T *char_get_curr_con_app (const CHAR_T *ch);
int char_str_carry_bonus (const CHAR_T *ch);
int char_str_hitroll_bonus (const CHAR_T *ch);
int char_str_damroll_bonus (const CHAR_T *ch);
int char_str_max_wield_weight (const CHAR_T *ch);
int char_int_learn_rate (const CHAR_T *ch);
int char_wis_level_practices (const CHAR_T *ch);
int char_dex_defense_bonus (const CHAR_T *ch);
int char_con_level_hp (const CHAR_T *ch);
int char_con_shock (const CHAR_T *ch);

/* Action functions. */
void char_from_room (CHAR_T *ch);
void char_to_room_apply_plague (CHAR_T *ch);
void char_to_room (CHAR_T *ch, ROOM_INDEX_T *room_index);
bool char_equip_obj (CHAR_T *ch, OBJ_T *obj, flag_t wear_loc);
bool char_unequip_obj (CHAR_T *ch, OBJ_T *obj);
void char_move (CHAR_T *ch, int door, bool follow);
char *char_format_to_char (const CHAR_T *victim, const CHAR_T *ch);
size_t char_format_condition_or_pos_msg (char *buf, size_t size,
    const CHAR_T *ch, const CHAR_T *victim, bool use_pronoun);
size_t char_format_condition_msg (char *buf, size_t size, const CHAR_T *ch,
    const CHAR_T *victim, bool use_pronoun);
size_t char_format_pos_msg (char *buf, size_t size, const CHAR_T *ch,
    const CHAR_T *victim, bool use_pronoun);
void char_look_at_char (CHAR_T *victim, CHAR_T *ch);
void char_list_show_to_char (const CHAR_T *list, CHAR_T *ch);
void char_take_obj (CHAR_T *ch, OBJ_T *obj, OBJ_T *container);
bool char_remove_obj (CHAR_T *ch, flag_t wear_loc, bool replace, bool quiet);
bool char_wear_obj (CHAR_T *ch, OBJ_T *obj, bool replace);
bool char_drop_weapon_if_too_heavy (CHAR_T *ch);
void char_reduce_money (CHAR_T *ch, int cost);
void char_stop_idling (CHAR_T *ch);
int char_set_max_wait_state (CHAR_T *ch, int npulse);
int char_set_max_daze_state (CHAR_T *ch, int npulse);
bool char_change_to_next_board (CHAR_T *ch);
void char_change_conditions (CHAR_T *ch, int drunk, int full, int thirst,
    int hunger);
OBJ_T *char_die (CHAR_T *ch);
void char_damage_if_wounded (CHAR_T *ch);
void char_update_all (void);
void char_update (CHAR_T *ch);

#endif
