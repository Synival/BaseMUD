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

#ifndef __ROM_ITEMS_H
#define __ROM_ITEMS_H

#include "merc.h"

/* Return values for item_get_door_flags(). */
#define ITEM_DOOR_NONE      0
#define ITEM_DOOR_EXIT      1
#define ITEM_DOOR_CONTAINER 2

/* "Is"/"Has" functions - returns TRUE/FALSE based on the state of the item. */
bool item_is_closed (const OBJ_T *obj);
bool item_is_edible (const OBJ_T *obj);
bool item_is_drinkable (const OBJ_T *obj);
bool item_is_comparable (const OBJ_T *obj);
bool item_is_container (const OBJ_T *obj);
bool item_is_lit (const OBJ_T *obj);
bool item_is_boat (const OBJ_T *obj);
bool item_is_visible_when_blind (const OBJ_T *obj);
bool item_is_armor (const OBJ_T *obj);
bool item_is_weapon (const OBJ_T *obj);
bool item_is_warp_stone (const OBJ_T *obj);
bool item_is_playing (const OBJ_T *obj);
bool item_has_liquid (OBJ_T *obj);
bool item_has_worth_as_room_reset (const OBJ_T *obj);
bool item_has_charges (const OBJ_T *obj);

/* "Can" functions - returns TRUE/FALSE based on whether or not an operation
 * can be performed by anyone for any reason. */
bool item_can_put_objs (const OBJ_T *obj);
bool item_can_sacrifice (OBJ_T *obj);
bool item_can_quaff (const OBJ_T *obj);
bool item_can_recite (const OBJ_T *obj);
bool item_can_brandish (const OBJ_T *obj);
bool item_can_zap (const OBJ_T *obj);
bool item_can_sell (const OBJ_T *obj);
bool item_can_wield (const OBJ_T *obj);
bool item_can_fill (const OBJ_T *obj);
bool item_can_play (const OBJ_T *obj);

/* "Can" functions with arguments - returns TRUE/FALSE based on whether or
 * not an operation can be performed under specific conditions. */
bool item_can_position_at (const OBJ_T *obj, int pos);
bool item_can_fit_obj_in (const OBJ_T *container, const OBJ_T *obj);
bool item_can_wear_flag (const OBJ_T *obj, flag_t wear_flag);
bool item_can_enter_as (const OBJ_T *obj, const CHAR_T *ch);
bool item_can_loot_as (const OBJ_T *obj, const CHAR_T *ch);

/* "Should" functions - similar to "can" functions but for internal
 * behavior that can happen at any time. */
bool item_should_save_for_level (const OBJ_T *obj, int ch_level);
bool item_should_spill_contents_when_poofed (const OBJ_T *obj);

/* Getter functions. */
int item_get_compare_value (const OBJ_T *obj);
int item_get_door_flags (const OBJ_T *obj, flag_t *flags, int *key);
const char **item_get_put_messages (const OBJ_T *obj);
bool item_get_drink_values (OBJ_T *obj, flag_t **capacity, flag_t **filled,
    flag_t **liquid, flag_t **poisoned, int *drink_amount);
const LIQ_T *item_get_liquid (const OBJ_T *obj);
int item_get_sacrifice_silver (const OBJ_T *obj);
int item_get_modified_cost (const OBJ_T *obj, int cost);
const char *item_get_corrode_message (const OBJ_T *obj);
int item_get_corrode_chance_modifier (const OBJ_T *obj);
const char *item_get_freeze_message (const OBJ_T *obj);
int item_get_freeze_chance_modifier (const OBJ_T *obj);
const char *item_get_burn_message (const OBJ_T *obj);
int item_get_burn_chance_modifier (const OBJ_T *obj);
const char *item_get_poison_message (const OBJ_T *obj);
int item_get_poison_chance_modifier (const OBJ_T *obj);
const char *item_get_shock_message (const OBJ_T *obj);
int item_get_shock_chance_modifier (const OBJ_T *obj);
bool item_get_corpse_timer_range (const OBJ_T *obj, int *timer_min,
    int *timer_max);
int item_get_ac (const OBJ_T *obj, int ac_type);
int item_get_carry_number (const OBJ_T *obj);
int item_get_furn_preposition_type (const OBJ_T *obj, int position);
int item_get_weight_mult (const OBJ_T *obj);
const char *item_get_poof_message (const OBJ_T *obj);
flag_t *item_get_poison_flag (OBJ_T *obj);
bool item_get_recharge_values (OBJ_T *obj, flag_t *level,
    flag_t **recharge_ptr, flag_t **charges_ptr);

/* Action functions. */
bool item_init (OBJ_T *obj, const OBJ_INDEX_T *obj_index, int level);
bool item_look_in (const OBJ_T *obj, CHAR_T *ch);
bool item_examine (const OBJ_T *obj, CHAR_T *ch);
bool item_eat_effect (OBJ_T *obj, CHAR_T *ch);
bool item_drink_effect (OBJ_T *obj, CHAR_T *ch);
bool item_quaff_effect (OBJ_T *obj, CHAR_T *ch);
bool item_recite_effect (OBJ_T *obj, CHAR_T *ch, CHAR_T *victim,
    OBJ_T *obj_target);
bool item_brandish_effect (OBJ_T *obj, CHAR_T *ch, bool try_improve);
bool item_zap_effect (OBJ_T *obj, CHAR_T *ch, CHAR_T *victim,
    OBJ_T *obj_target);
bool item_corrode_effect (OBJ_T *obj, int level);
bool item_freeze_effect (OBJ_T *obj, int level);
bool item_burn_effect (OBJ_T *obj, int level);
bool item_poison_effect (OBJ_T *obj, int level);
bool item_shock_effect (OBJ_T *obj, int level);
bool item_set_exit_flag (OBJ_T *obj, flag_t exit_flag);
bool item_remove_exit_flag (OBJ_T *obj, flag_t exit_flag);
bool item_write_save_data (const OBJ_T *obj, FILE *fp);
bool item_envenom_effect (OBJ_T *obj, CHAR_T *ch, int skill, int sn,
    bool as_spell);
void item_envenom_effect_food (OBJ_T *obj, CHAR_T *ch, int skill, int sn,
    bool as_spell);
void item_envenom_effect_weapon (OBJ_T *obj, CHAR_T *ch, int skill, int sn,
    bool as_spell);
bool item_light_fade (OBJ_T *obj);
bool item_enter_effect (OBJ_T *obj, CHAR_T *ch);
bool item_take_effect (OBJ_T *obj, CHAR_T *ch);
bool item_consume_charge_as (OBJ_T *obj, CHAR_T *ch);
bool item_play_effect (OBJ_T *obj, CHAR_T *ch, char *argument);
bool item_play_continue (OBJ_T *obj);

/* Functions for obj indexes. */
bool item_index_is_container (const OBJ_INDEX_T *obj);
bool item_index_can_wear_flag (const OBJ_INDEX_T *obj, flag_t wear_flag);
bool item_index_fix_old (OBJ_INDEX_T *obj_index);
bool item_index_convert_old (OBJ_INDEX_T *obj_index);
bool item_index_finalize (OBJ_INDEX_T *obj);
int item_index_get_old_convert_shop_level (const OBJ_INDEX_T *obj_index);
int item_index_get_old_reset_shop_level (const OBJ_INDEX_T *obj_index);
bool item_index_read_values_from_file (OBJ_INDEX_T *obj_index, FILE *fp);
bool item_index_write_values_to_file (OBJ_INDEX_T *obj_index, FILE *fp);
bool item_index_show_values (const OBJ_INDEX_T *obj, CHAR_T *ch);

/* Misc. utilities. */
flag_t item_exit_flag_to_cont (flag_t exit_flag);

#endif
