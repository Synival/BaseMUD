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

/* Unused functions... */
#if 0
bool char_is_friend (CHAR_DATA *ch, CHAR_DATA * victim);
#endif

/* Creation / destruction. */
void char_extract (CHAR_DATA *ch, bool fPull);

/* "Is" / "Can" functions. */
bool char_has_clan (CHAR_DATA *ch);
bool char_in_same_clan (CHAR_DATA *ch, CHAR_DATA *victim);
bool char_can_see_room (CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex);
bool char_can_see_anywhere (CHAR_DATA *ch, CHAR_DATA *victim);
bool char_can_see_in_room (CHAR_DATA *ch, CHAR_DATA *victim);
bool char_can_see_obj (CHAR_DATA *ch, OBJ_DATA *obj);
bool char_can_drop_obj (CHAR_DATA *ch, OBJ_DATA *obj);
bool char_can_loot (CHAR_DATA *ch, OBJ_DATA *obj);
bool char_has_key (CHAR_DATA *ch, int key);

/* Getter functions. */
int char_get_weapon_sn (CHAR_DATA *ch);
int char_get_weapon_skill (CHAR_DATA *ch, int sn);
int char_get_trust (CHAR_DATA *ch);
int char_get_age (CHAR_DATA *ch);
int char_get_curr_stat (CHAR_DATA *ch, int stat);
int char_get_max_train (CHAR_DATA *ch, int stat);
long int char_get_carry_weight (CHAR_DATA *ch);
int char_get_max_carry_count (CHAR_DATA *ch);
long int char_get_max_carry_weight (CHAR_DATA *ch);
void char_get_who_string (CHAR_DATA *ch, CHAR_DATA *wch, char *buf,
    size_t len);
int char_get_obj_cost (CHAR_DATA *ch, OBJ_DATA *obj, bool fBuy);
OBJ_DATA *char_get_eq_by_wear (CHAR_DATA *ch, int iWear);
SHOP_DATA *char_get_shop (CHAR_DATA *ch);
CHAR_DATA *char_get_keeper_room (CHAR_DATA *ch);
CHAR_DATA *char_get_trainer_room (CHAR_DATA *ch);
CHAR_DATA *char_get_practicer_room (CHAR_DATA *ch);
CHAR_DATA *char_get_gainer_room (CHAR_DATA *ch);

/* Action functions. */
void char_reset (CHAR_DATA *ch);
void char_from_room (CHAR_DATA *ch);
void char_to_room_apply_plague (CHAR_DATA *ch);
void char_to_room (CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex);
void char_equip (CHAR_DATA *ch, OBJ_DATA *obj, int iWear);
void char_unequip (CHAR_DATA *ch, OBJ_DATA *obj);
void char_reset_colour (CHAR_DATA *ch);
void char_move (CHAR_DATA *ch, int door, bool follow);
char *char_format_to_char (CHAR_DATA *victim, CHAR_DATA *ch);
void char_look_at_char (CHAR_DATA *victim, CHAR_DATA *ch);
void char_list_show_to_char (CHAR_DATA *list, CHAR_DATA *ch);
void char_take_obj (CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container);
bool char_remove_obj (CHAR_DATA * ch, int iWear, bool fReplace, bool quiet);
void char_wear_obj (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace);
void char_set_title (CHAR_DATA *ch, char *title);
bool char_drop_weapon_if_too_heavy (CHAR_DATA *ch);
void char_reduce_money (CHAR_DATA *ch, int cost);

/* Misc. utility functions. */
char *condition_string (int percent);
const char *get_wiz_class (int level);

#endif
