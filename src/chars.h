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
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

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
#define REAL_CH(ch) (((ch)->desc) ? CH((ch)->desc) : (ch))

/* Unused functions... */
#if 0
bool char_is_friend (CHAR_DATA *ch, CHAR_DATA * victim);
#endif

/* Creation / destruction. */
CHAR_DATA *char_create_mobile (MOB_INDEX_DATA * pMobIndex);
void char_clone_mobile (CHAR_DATA * parent, CHAR_DATA * clone);
void char_extract (CHAR_DATA *ch, bool fPull);

/* "Is" / "Can" functions. */
bool char_is_trusted (CHAR_DATA *ch, int level);
bool char_is_npc (CHAR_DATA *ch);
bool char_is_immortal (CHAR_DATA *ch);
bool char_is_hero (CHAR_DATA *ch);
bool char_is_affected (CHAR_DATA *ch, flag_t sn);
bool char_is_sober (CHAR_DATA *ch);
bool char_is_drunk (CHAR_DATA *ch);
bool char_is_thirsty (CHAR_DATA *ch);
bool char_is_quenched (CHAR_DATA *ch);
bool char_is_hungry (CHAR_DATA *ch);
bool char_is_fed (CHAR_DATA *ch);
bool char_is_full (CHAR_DATA *ch);
bool char_is_pet (CHAR_DATA *ch);
bool char_is_good (CHAR_DATA *ch);
bool char_is_evil (CHAR_DATA *ch);
bool char_is_really_good (CHAR_DATA *ch);
bool char_is_really_evil (CHAR_DATA *ch);
bool char_is_neutral (CHAR_DATA *ch);
bool char_is_outside (CHAR_DATA *ch);
bool char_is_awake (CHAR_DATA *ch);
bool char_is_same_align (CHAR_DATA *ch1, CHAR_DATA *ch2);
bool char_is_switched (CHAR_DATA *ch);
bool char_is_builder (CHAR_DATA *ch, AREA_DATA *area);
bool char_has_clan (CHAR_DATA *ch);
bool char_in_same_clan (CHAR_DATA *ch, CHAR_DATA *victim);
bool char_can_see_room (CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex);
bool char_can_see_anywhere (CHAR_DATA *ch, CHAR_DATA *victim);
bool char_can_see_in_room (CHAR_DATA *ch, CHAR_DATA *victim);
bool char_can_see_obj (CHAR_DATA *ch, OBJ_DATA *obj);
bool char_can_drop_obj (CHAR_DATA *ch, OBJ_DATA *obj);
bool char_can_loot (CHAR_DATA *ch, OBJ_DATA *obj);
bool char_has_key (CHAR_DATA *ch, int key);
bool char_has_available_wear_loc (CHAR_DATA *ch, flag_t wear_loc);
bool char_has_available_wear_flag (CHAR_DATA *ch, flag_t wear_flag);

/* Getter functions. */
int char_get_vnum (CHAR_DATA *ch);
int char_get_weapon_sn (CHAR_DATA *ch);
int char_get_weapon_skill (CHAR_DATA *ch, int sn);
int char_get_trust (CHAR_DATA *ch);
int char_get_curr_stat (CHAR_DATA *ch, int stat);
int char_get_max_train (CHAR_DATA *ch, int stat);
long int char_get_carry_weight (CHAR_DATA *ch);
int char_get_max_carry_count (CHAR_DATA *ch);
long int char_get_max_carry_weight (CHAR_DATA *ch);
void char_get_who_string (CHAR_DATA *ch, CHAR_DATA *wch, char *buf,
    size_t len);
int char_get_obj_cost (CHAR_DATA *ch, OBJ_DATA *obj, bool fBuy);
OBJ_DATA *char_get_eq_by_wear_loc (CHAR_DATA *ch, flag_t iWear);
SHOP_DATA *char_get_shop (CHAR_DATA *ch);
CHAR_DATA *char_get_keeper_room (CHAR_DATA *ch);
CHAR_DATA *char_get_trainer_room (CHAR_DATA *ch);
CHAR_DATA *char_get_practicer_room (CHAR_DATA *ch);
CHAR_DATA *char_get_gainer_room (CHAR_DATA *ch);
const char *char_get_class_name (CHAR_DATA * ch);
const char *char_get_position_str (CHAR_DATA * ch, int position,
    OBJ_DATA * on, int with_punct);
int char_get_age (CHAR_DATA *ch);
int char_get_ac (CHAR_DATA *ch, int type);
int char_get_hitroll (CHAR_DATA *ch);
int char_get_damroll (CHAR_DATA *ch);
char *char_get_short_descr (CHAR_DATA *ch);
char *char_get_look_short_descr_anywhere (CHAR_DATA *looker, CHAR_DATA *ch);
char *char_get_look_short_descr (CHAR_DATA *looker, CHAR_DATA *ch);

/* Stat bonuses. */
const STR_APP_TYPE *char_get_curr_str_app (CHAR_DATA *ch);
const INT_APP_TYPE *char_get_curr_int_app (CHAR_DATA *ch);
const WIS_APP_TYPE *char_get_curr_wis_app (CHAR_DATA *ch);
const DEX_APP_TYPE *char_get_curr_dex_app (CHAR_DATA *ch);
const CON_APP_TYPE *char_get_curr_con_app (CHAR_DATA *ch);
int char_str_carry_bonus (CHAR_DATA *ch);
int char_str_hitroll_bonus (CHAR_DATA *ch);
int char_str_damroll_bonus (CHAR_DATA *ch);
int char_str_max_wield_weight (CHAR_DATA *ch);
int char_int_learn_rate (CHAR_DATA *ch);
int char_wis_level_practices (CHAR_DATA *ch);
int char_dex_defense_bonus (CHAR_DATA *ch);
int char_con_level_hp (CHAR_DATA *ch);

/* Action functions. */
void char_reset (CHAR_DATA *ch);
void char_from_room (CHAR_DATA *ch);
void char_to_room_apply_plague (CHAR_DATA *ch);
void char_to_room (CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex);
bool char_equip_obj (CHAR_DATA *ch, OBJ_DATA *obj, flag_t iWear);
bool char_unequip_obj (CHAR_DATA *ch, OBJ_DATA *obj);
void char_reset_colour (CHAR_DATA *ch);
void char_move (CHAR_DATA *ch, int door, bool follow);
char *char_format_to_char (CHAR_DATA *victim, CHAR_DATA *ch);
void char_look_at_char (CHAR_DATA *victim, CHAR_DATA *ch);
void char_list_show_to_char (CHAR_DATA *list, CHAR_DATA *ch);
void char_take_obj (CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container);
bool char_remove_obj (CHAR_DATA * ch, flag_t iWear, bool fReplace, bool quiet);
bool char_wear_obj (CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace);
void char_set_title (CHAR_DATA *ch, char *title);
bool char_drop_weapon_if_too_heavy (CHAR_DATA *ch);
void char_reduce_money (CHAR_DATA *ch, int cost);
int char_exit_string (CHAR_DATA *ch, ROOM_INDEX_DATA *room, int mode,
    char *out_buf, size_t out_size);
void char_stop_idling (CHAR_DATA * ch);
int char_set_max_wait_state (CHAR_DATA *ch, int npulse);
int char_set_max_daze_state (CHAR_DATA *ch, int npulse);

#endif
