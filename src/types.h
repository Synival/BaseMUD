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

#ifndef __ROM_TYPES_H
#define __ROM_TYPES_H

#include "merc.h"

/* Sex. */
#define SEX_NEUTRAL  0
#define SEX_MALE     1
#define SEX_FEMALE   2
#define SEX_EITHER   3
#define SEX_MAX      4

/* Positions. */
#define POS_DEAD      0
#define POS_MORTAL    1
#define POS_INCAP     2
#define POS_STUNNED   3
#define POS_SLEEPING  4
#define POS_RESTING   5
#define POS_SITTING   6
#define POS_FIGHTING  7
#define POS_STANDING  8
#define POS_MAX       9

/* Size. */
#define SIZE_TINY    0
#define SIZE_SMALL   1
#define SIZE_MEDIUM  2
#define SIZE_LARGE   3
#define SIZE_HUGE    4
#define SIZE_GIANT   5
#define SIZE_MAX_R   6

/* Item types. */
#define ITEM_NONE        0
#define ITEM_LIGHT       1
#define ITEM_SCROLL      2
#define ITEM_WAND        3
#define ITEM_STAFF       4
#define ITEM_WEAPON      5
#define ITEM_UNUSED_1    6 /* old: fireweapon */
#define ITEM_UNUSED_2    7 /* old: missile */
#define ITEM_TREASURE    8
#define ITEM_ARMOR       9
#define ITEM_POTION      10
#define ITEM_CLOTHING    11
#define ITEM_FURNITURE   12
#define ITEM_TRASH       13
#define ITEM_UNUSED_3    14 /* old: trap */
#define ITEM_CONTAINER   15
#define ITEM_UNUSED_4    16 /* old: note */
#define ITEM_DRINK_CON   17
#define ITEM_KEY         18
#define ITEM_FOOD        19
#define ITEM_MONEY       20
#define ITEM_UNUSED_5    21 /* old: pen */
#define ITEM_BOAT        22
#define ITEM_CORPSE_NPC  23
#define ITEM_CORPSE_PC   24
#define ITEM_FOUNTAIN    25
#define ITEM_PILL        26
#define ITEM_PROTECT     27
#define ITEM_MAP         28
#define ITEM_PORTAL      29
#define ITEM_WARP_STONE  30
#define ITEM_ROOM_KEY    31
#define ITEM_GEM         32
#define ITEM_JEWELRY     33
#define ITEM_JUKEBOX     34
#define ITEM_MAX         35

/* weapon class */
#define WEAPON_EXOTIC   0
#define WEAPON_SWORD    1
#define WEAPON_DAGGER   2
#define WEAPON_SPEAR    3
#define WEAPON_MACE     4
#define WEAPON_AXE      5
#define WEAPON_FLAIL    6
#define WEAPON_WHIP     7
#define WEAPON_POLEARM  8
#define WEAPON_MAX      9

/* Sector types. */
#define SECT_INSIDE         0
#define SECT_CITY           1
#define SECT_FIELD          2
#define SECT_FOREST         3
#define SECT_HILLS          4
#define SECT_MOUNTAIN       5
#define SECT_WATER_SWIM     6
#define SECT_WATER_NOSWIM   7
#define SECT_UNUSED         8 /* old: flying */
#define SECT_AIR            9
#define SECT_DESERT        10
#define SECT_MAX           11

/* Equpiment wear locations. */
#define WEAR_LOC_NONE     -1
#define WEAR_LOC_LIGHT     0
#define WEAR_LOC_FINGER_L  1
#define WEAR_LOC_FINGER_R  2
#define WEAR_LOC_NECK_1    3
#define WEAR_LOC_NECK_2    4
#define WEAR_LOC_BODY      5
#define WEAR_LOC_HEAD      6
#define WEAR_LOC_LEGS      7
#define WEAR_LOC_FEET      8
#define WEAR_LOC_HANDS     9
#define WEAR_LOC_ARMS     10
#define WEAR_LOC_SHIELD   11
#define WEAR_LOC_ABOUT    12
#define WEAR_LOC_WAIST    13
#define WEAR_LOC_WRIST_L  14
#define WEAR_LOC_WRIST_R  15
#define WEAR_LOC_WIELD    16
#define WEAR_LOC_HOLD     17
#define WEAR_LOC_FLOAT    18
#define WEAR_LOC_MAX      19

/* AC types */
#define AC_PIERCE    0
#define AC_BASH      1
#define AC_SLASH     2
#define AC_EXOTIC    3
#define AC_MAX       4

/* Apply types (for affects).
 * Used in #OBJECTS. */
#define APPLY_NONE            0
#define APPLY_STR             1
#define APPLY_DEX             2
#define APPLY_INT             3
#define APPLY_WIS             4
#define APPLY_CON             5
#define APPLY_SEX             6
#define APPLY_CLASS           7
#define APPLY_LEVEL           8
#define APPLY_AGE             9
#define APPLY_HEIGHT         10
#define APPLY_WEIGHT         11
#define APPLY_MANA           12
#define APPLY_HIT            13
#define APPLY_MOVE           14
#define APPLY_GOLD           15
#define APPLY_EXP            16
#define APPLY_AC             17
#define APPLY_HITROLL        18
#define APPLY_DAMROLL        19
#define APPLY_SAVES          20
#define APPLY_SAVING_PARA    20
#define APPLY_SAVING_ROD     21
#define APPLY_SAVING_PETRI   22
#define APPLY_SAVING_BREATH  23
#define APPLY_SAVING_SPELL   24
#define APPLY_SPELL_AFFECT   25
#define APPLY_MAX            26

/* Material types - currently unused, but neat. */
#define MATERIAL_GENERIC     0
#define MATERIAL_ADAMANTITE  1
#define MATERIAL_ALUMINUM    2
#define MATERIAL_BRASS       3
#define MATERIAL_BRONZE      4
#define MATERIAL_CHINA       5
#define MATERIAL_CLAY        6
#define MATERIAL_CLOTH       7
#define MATERIAL_COPPER      8
#define MATERIAL_CRYSTAL     9
#define MATERIAL_DIAMOND    10
#define MATERIAL_ENERGY     11
#define MATERIAL_FLESH      12
#define MATERIAL_FOOD       13
#define MATERIAL_FUR        14
#define MATERIAL_GEM        15
#define MATERIAL_GLASS      16
#define MATERIAL_GOLD       17
#define MATERIAL_ICE        18
#define MATERIAL_IRON       19
#define MATERIAL_IVORY      20
#define MATERIAL_LEAD       21
#define MATERIAL_LEATHER    22
#define MATERIAL_MEAT       23
#define MATERIAL_MITHRIL    24
#define MATERIAL_OBSIDIAN   25
#define MATERIAL_PAPER      26
#define MATERIAL_PARCHMENT  27
#define MATERIAL_PEARL      28
#define MATERIAL_PLATINUM   29
#define MATERIAL_RUBBER     30
#define MATERIAL_SHADOW     31
#define MATERIAL_SILVER     32
#define MATERIAL_STEEL      33
#define MATERIAL_TIN        34
#define MATERIAL_VELLUM     35
#define MATERIAL_WATER      36
#define MATERIAL_WOOD       37
#define MATERIAL_MAX        38

/* Door resets. */
#define RESET_OPEN      0
#define RESET_CLOSED    1
#define RESET_LOCKED    2
#define RESET_MAX       3

/* Type tables. */
extern const TYPE_T sex_types[SEX_MAX + 1];
extern const TYPE_T ac_types[AC_MAX + 1];
extern const TYPE_T size_types[SIZE_MAX_R + 1];
extern const TYPE_T weapon_types[WEAPON_MAX + 1];
extern const TYPE_T position_types[POS_MAX + 1];
extern const TYPE_T affect_apply_types[APPLY_MAX + 1];
extern const TYPE_T sector_types[SECT_MAX + 1];
extern const TYPE_T item_types[ITEM_MAX + 1];
extern const TYPE_T door_resets[RESET_MAX + 1];
extern const TYPE_T stat_types[STAT_MAX + 1];
extern const TYPE_T cond_types[COND_MAX + 1];
extern const TYPE_T target_types[SKILL_TARGET_MAX + 1];
extern const TYPE_T skill_target_types[SKILL_TARGET_MAX + 1];
extern const TYPE_T board_def_types[DEF_MAX + 1];

/* Function prototypes for type management. */
type_t type_lookup (const TYPE_T *type_table, const char *name);
type_t type_lookup_exact (const TYPE_T *type_table, const char *name);
const TYPE_T *type_get_by_name (const TYPE_T *type_table, const char *name);
const TYPE_T *type_get_by_name_exact (const TYPE_T *type_table,
    const char *name);
const TYPE_T *type_get (const TYPE_T *type_table, type_t type);
const char *type_get_name (const TYPE_T *type_table, type_t type);
type_t type_lookup_exact_backup (const TYPE_T *type_table, const char *str,
    const char *errf, type_t backup);

#endif
