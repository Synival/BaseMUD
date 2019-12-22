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

#ifndef __ROM_JSON_TBL_H
#define __ROM_JSON_TBL_H

#include "merc.h"

/* dead-simple functions for creating JSON objects. */
DECLARE_TABLE_JSON_FUN (json_tblw_flag);
DECLARE_TABLE_JSON_FUN (json_tblw_clan);
DECLARE_TABLE_JSON_FUN (json_tblw_sex);
DECLARE_TABLE_JSON_FUN (json_tblw_position);
DECLARE_TABLE_JSON_FUN (json_tblw_size);
DECLARE_TABLE_JSON_FUN (json_tblw_item);
DECLARE_TABLE_JSON_FUN (json_tblw_weapon);
DECLARE_TABLE_JSON_FUN (json_tblw_dam);
DECLARE_TABLE_JSON_FUN (json_tblw_attack);
DECLARE_TABLE_JSON_FUN (json_tblw_race);
DECLARE_TABLE_JSON_FUN (json_tblw_pc_race);
DECLARE_TABLE_JSON_FUN (json_tblw_class);
DECLARE_TABLE_JSON_FUN (json_tblw_str_app);
DECLARE_TABLE_JSON_FUN (json_tblw_int_app);
DECLARE_TABLE_JSON_FUN (json_tblw_wis_app);
DECLARE_TABLE_JSON_FUN (json_tblw_dex_app);
DECLARE_TABLE_JSON_FUN (json_tblw_con_app);
DECLARE_TABLE_JSON_FUN (json_tblw_liq);
DECLARE_TABLE_JSON_FUN (json_tblw_skill);
DECLARE_TABLE_JSON_FUN (json_tblw_skill_group);
DECLARE_TABLE_JSON_FUN (json_tblw_sector);
DECLARE_TABLE_JSON_FUN (json_tblw_door);
DECLARE_TABLE_JSON_FUN (json_tblw_spec);
DECLARE_TABLE_JSON_FUN (json_tblw_furniture);
DECLARE_TABLE_JSON_FUN (json_tblw_wear_loc);
DECLARE_TABLE_JSON_FUN (json_tblw_material);
DECLARE_TABLE_JSON_FUN (json_tblw_colour_setting);
DECLARE_TABLE_JSON_FUN (json_tblw_colour);
DECLARE_TABLE_JSON_FUN (json_tblw_board);
DECLARE_TABLE_JSON_FUN (json_tblw_day);
DECLARE_TABLE_JSON_FUN (json_tblw_month);
DECLARE_TABLE_JSON_FUN (json_tblw_sky);
DECLARE_TABLE_JSON_FUN (json_tblw_sun);
DECLARE_TABLE_JSON_FUN (json_tblw_pose);
DECLARE_TABLE_JSON_FUN (json_tblw_condition);

#endif
