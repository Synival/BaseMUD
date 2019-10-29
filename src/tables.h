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
 *    ROM 2.4 is copyright 1993-1998 Russ Taylor                           *
 *    ROM has been brought to you by the ROM consortium                    *
 *        Russ Taylor (rtaylor@hypercube.org)                              *
 *        Gabrielle Taylor (gtaylor@hypercube.org)                         *
 *        Brian Moore (zump@rom.org)                                       *
 *    By using this code, you have agreed to follow the terms of the       *
 *    ROM license, in the file Rom24/doc/rom.license                       *
 ***************************************************************************/

#ifndef __ROM_TABLES_H
#define __ROM_TABLES_H

#include "merc.h"

/* Useful macros for defining rows in our master table. */
#define TFLAGS(table, desc) \
    { table, #table, TABLE_FLAG_TYPE | TABLE_BITS, desc, \
      sizeof (FLAG_TYPE), json_tblw_flag }
#define TTYPES(table, desc) \
    { table, #table, TABLE_FLAG_TYPE, desc, \
      sizeof (FLAG_TYPE), json_tblw_flag }
#define TTABLE(table, desc, jwrite) \
    { table, #table, 0, desc, sizeof(table[0]), jwrite }

/* A table containing every table of every type. */
extern const TABLE_TYPE       master_table[];

/* All tables. */
extern const CLAN_TYPE        clan_table[CLAN_MAX + 1];
extern const POSITION_TYPE    position_table[POS_MAX + 1];
extern const SEX_TYPE         sex_table[SEX_MAX + 1];
extern const SIZE_TYPE        size_table[SIZE_MAX_R + 1];
extern const ITEM_TYPE        item_table[ITEM_MAX + 1];
extern const WEAPON_TYPE      weapon_table[WEAPON_MAX + 1];
extern const DAM_TYPE         dam_table[DAM_MAX + 1];
extern const ATTACK_TYPE      attack_table[ATTACK_MAX + 1];
extern const RACE_TYPE        race_table[RACE_MAX + 1];
extern const PC_RACE_TYPE     pc_race_table[PC_RACE_MAX + 1];
extern const CLASS_TYPE       class_table[CLASS_MAX + 1];
extern const STR_APP_TYPE     str_app_table[ATTRIBUTE_HIGHEST + 2];
extern const INT_APP_TYPE     int_app_table[ATTRIBUTE_HIGHEST + 2];
extern const WIS_APP_TYPE     wis_app_table[ATTRIBUTE_HIGHEST + 2];
extern const DEX_APP_TYPE     dex_app_table[ATTRIBUTE_HIGHEST + 2];
extern const CON_APP_TYPE     con_app_table[ATTRIBUTE_HIGHEST + 2];
extern const LIQ_TYPE         liq_table[LIQ_MAX + 1];
extern const SKILL_TYPE       skill_table[SKILL_MAX + 1];
extern const GROUP_TYPE       group_table[GROUP_MAX + 1];
extern const SECTOR_TYPE      sector_table[SECT_MAX + 1];
extern const NANNY_HANDLER    nanny_table[NANNY_MAX + 1];
extern const DOOR_TYPE        door_table[DIR_MAX + 1];
extern const SPEC_TYPE        spec_table[SPEC_MAX + 1];
extern const FURNITURE_BITS   furniture_table[POS_MAX + 1];
extern const WEAR_LOC_TYPE    wear_loc_table[WEAR_LOC_MAX + 1];
extern const MATERIAL_TYPE    material_table[MATERIAL_MAX + 1];
extern const COLOUR_SETTING_TYPE colour_setting_table[COLOUR_MAX + 1];
extern const WIZNET_TYPE      wiznet_table[WIZNET_MAX + 1];
extern const DAY_TYPE         day_table[DAY_MAX + 1];
extern const MONTH_TYPE       month_table[MONTH_MAX + 1];
extern const SKY_TYPE         sky_table[SKY_MAX + 1];
extern const SUN_TYPE         sun_table[SUN_MAX + 1];
extern const AFFECT_BIT_TYPE  affect_bit_table[AFF_TO_MAX + 1];

/* Tables with flexible sizes. */
extern const MAP_LOOKUP_TABLE map_lookup_table[];
extern const MAP_LOOKUP_TABLE map_flags_table[];
extern const OBJ_MAP          obj_map_table[];
extern const COLOUR_TYPE      colour_table[];

/* Non-const types. */
extern RECYCLE_TYPE recycle_table[RECYCLE_MAX + 1];
extern BOARD_DATA   board_table[BOARD_MAX + 1];

/* Other tables. */
extern char * const title_table[CLASS_MAX][MAX_LEVEL + 1][2];

#endif
