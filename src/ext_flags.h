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

#ifndef __ROM_EXT_FLAGS_H
#define __ROM_EXT_FLAGS_H

#include "merc.h"

/* ACT bits for mobs.
 * Used in #MOBILES. */
#define MOB_IS_NPC         0   /* Auto set for mobs  */
#define MOB_SENTINEL       1   /* Stays in one room  */
#define MOB_SCAVENGER      2   /* Picks up objects   */
#define MOB_UNUSED_FLAG_1  3   /* old: isnpc */
#define MOB_UNUSED_FLAG_2  4   /* old: nice_thief */
#define MOB_AGGRESSIVE     5   /* Attacks PC's       */
#define MOB_STAY_AREA      6   /* Won't leave area   */
#define MOB_WIMPY          7
#define MOB_PET            8   /* Auto set for pets  */
#define MOB_TRAIN          9   /* Can train PC's     */
#define MOB_PRACTICE       10  /* Can practice PC's  */
#define MOB_UNUSED_FLAG_3  11
#define MOB_UNUSED_FLAG_4  12
#define MOB_UNUSED_FLAG_5  13
#define MOB_UNDEAD         14
#define MOB_UNUSED_FLAG_6  15
#define MOB_CLERIC         16
#define MOB_MAGE           17
#define MOB_THIEF          18
#define MOB_WARRIOR        19
#define MOB_NOALIGN        20
#define MOB_NOPURGE        21
#define MOB_OUTDOORS       22
#define MOB_UNUSED_FLAG_7  23
#define MOB_INDOORS        24
#define MOB_UNUSED_FLAG_8  25
#define MOB_IS_HEALER      26
#define MOB_GAIN           27
#define MOB_UPDATE_ALWAYS  28
#define MOB_IS_CHANGER     29

/* Flag tables. */
extern const EXT_FLAG_DEF_T mob_flags[];

/* Special constants. */
extern const EXT_FLAGS_T ext_flags_zero;

/* Generation / conversion functions. */
EXT_FLAGS_T ext_flags_from_flag_t (flag_t flags);
EXT_FLAGS_T ext_flags_build (int first, ...);
EXT_FLAGS_T ext_flags_from_init (const EXT_INIT_FLAGS_T *flags);
flag_t ext_flags_to_flag_t (EXT_FLAGS_T bits);

/* Bit operations. */
EXT_FLAGS_T ext_flags_with (EXT_FLAGS_T bits, int flag);
EXT_FLAGS_T ext_flags_without (EXT_FLAGS_T bits, int flag);
EXT_FLAGS_T ext_flags_with_many (EXT_FLAGS_T bits1, EXT_FLAGS_T bits2);
EXT_FLAGS_T ext_flags_without_many (EXT_FLAGS_T bits1, EXT_FLAGS_T bits2);

/* Comparison functions. */
bool ext_flags_is_set (EXT_FLAGS_T var, int flag);
bool ext_flags_is_zero (EXT_FLAGS_T bits);
bool ext_flags_equals (EXT_FLAGS_T bits1, EXT_FLAGS_T bits2);

/* Modification operations. */
void ext_flags_set (EXT_FLAGS_T *var, int flag);
void ext_flags_set_many (EXT_FLAGS_T *var, EXT_FLAGS_T to_set);
void ext_flags_unset (EXT_FLAGS_T *var, int flag);
void ext_flags_unset_many (EXT_FLAGS_T *var, EXT_FLAGS_T to_remove);
void ext_flags_toggle (EXT_FLAGS_T *var, int flag);
void ext_flags_toggle_many (EXT_FLAGS_T *var, EXT_FLAGS_T to_set);

/* Lookup functions. */
int ext_flag_lookup (const EXT_FLAG_DEF_T *flag_table, const char *name);
int ext_flag_lookup_exact (const EXT_FLAG_DEF_T *flag_table, const char *name);
const EXT_FLAG_DEF_T *ext_flag_get_by_name (const EXT_FLAG_DEF_T *flag_table, const char *name);
const EXT_FLAG_DEF_T *ext_flag_get_by_name_exact (const EXT_FLAG_DEF_T *flag_table, char const *name);
const EXT_FLAG_DEF_T *ext_flag_get (const EXT_FLAG_DEF_T *flag_table, flag_t bit);
const char *ext_flag_get_name (const EXT_FLAG_DEF_T *flag_table, flag_t bit);
EXT_FLAGS_T ext_flags_from_string (const EXT_FLAG_DEF_T *flag_table, const char *name);
EXT_FLAGS_T ext_flags_from_string_exact (const EXT_FLAG_DEF_T *flag_table, const char *name);
EXT_FLAGS_T ext_flags_from_string_real (const EXT_FLAG_DEF_T *flag_table, const char *name, bool exact);
const char *ext_flags_to_string (const EXT_FLAG_DEF_T *flag_table,
    EXT_FLAGS_T bits);
const char *ext_flags_to_string_real (const EXT_FLAG_DEF_T *flag_table,
    EXT_FLAGS_T bits, const char *none_str);

#endif
