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

#ifndef __ROM_TABLES_H
#define __ROM_TABLES_H

#include "merc.h"

/* A table containing every table of every type. */
extern const TABLE_T          master_table[TABLE_MAX + 1];

/* All tables. */
extern const CLAN_T           clan_table[CLAN_MAX + 1];
extern const POSITION_T       position_table[POS_MAX + 1];
extern const SEX_T            sex_table[SEX_MAX + 1];
extern const SIZE_T           size_table[SIZE_MAX_R + 1];
extern const ITEM_T           item_table[ITEM_MAX + 1];
extern const WEAPON_T         weapon_table[WEAPON_MAX + 1];
extern const EFFECT_T         effect_table[EFFECT_MAX + 1];
extern const DAM_T            dam_table[DAM_MAX + 1];
extern const ATTACK_T         attack_table[ATTACK_MAX + 1];
extern const RACE_T           race_table[RACE_MAX + 1];
extern const PC_RACE_T        pc_race_table[PC_RACE_MAX + 1];
extern const CLASS_T          class_table[CLASS_MAX + 1];
extern const STR_APP_T        str_app_table[ATTRIBUTE_HIGHEST + 2];
extern const INT_APP_T        int_app_table[ATTRIBUTE_HIGHEST + 2];
extern const WIS_APP_T        wis_app_table[ATTRIBUTE_HIGHEST + 2];
extern const DEX_APP_T        dex_app_table[ATTRIBUTE_HIGHEST + 2];
extern const CON_APP_T        con_app_table[ATTRIBUTE_HIGHEST + 2];
extern const LIQ_T            liq_table[LIQ_MAX + 1];
extern const SKILL_T          skill_table[SKILL_MAX + 1];
extern const SKILL_GROUP_T    skill_group_table[SKILL_GROUP_MAX + 1];
extern const SECTOR_T         sector_table[SECT_MAX + 1];
extern const NANNY_HANDLER_T  nanny_table[NANNY_MAX + 1];
extern const DOOR_T           door_table[DIR_MAX + 1];
extern const SPEC_T           spec_table[SPEC_MAX + 1];
extern const FURNITURE_BITS_T furniture_table[POS_MAX + 1];
extern const WEAR_LOC_T       wear_loc_table[WEAR_LOC_MAX + 1];
extern const MATERIAL_T       material_table[MATERIAL_MAX + 1];
extern const COLOUR_SETTING_T colour_setting_table[COLOUR_MAX + 1];
extern const WIZNET_T         wiznet_table[WIZNET_MAX + 1];
extern const DAY_T            day_table[DAY_MAX + 1];
extern const MONTH_T          month_table[MONTH_MAX + 1];
extern const SKY_T            sky_table[SKY_MAX + 1];
extern const SUN_T            sun_table[SUN_MAX + 1];
extern const AFFECT_BIT_T     affect_bit_table[AFF_TO_MAX + 1];

/* Tables with flexible sizes. */
extern const CONDITION_T      condition_table[];
extern const MAP_LOOKUP_TABLE_T map_lookup_table[];
extern const MAP_LOOKUP_TABLE_T map_flags_table[];
extern const OBJ_MAP_T        obj_map_table[];
extern const COLOUR_T         colour_table[];
extern const POSE_T           pose_table[];

/* Non-const types. */
extern RECYCLE_T recycle_table[RECYCLE_MAX + 1];
extern BOARD_T   board_table[BOARD_MAX + 1];

/* Other tables. */
extern char *const title_table[CLASS_MAX][MAX_LEVEL + 1][2];

#endif
