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

/* NOTE:
 * -----
 * This file contains helper functions for master_table[] that will output
 * any type of structure to a JSON object. This is a neat idea, but since it's
 * a new feature rather than a clean-up, it's been left unfinished.
 *    -- Synival */

#include "json_obj.h"

#include "json_tbl.h"

#define JSON_TBLW_START(vtype, var, null_check) \
    const vtype *var = obj; \
    JSON_T *new; \
    do { \
        if (null_check) \
            return NULL; \
        new = json_new_object (NULL, JSON_OBJ_ANY); \
    } while (0)

DEFINE_TABLE_JSON_FUN (json_tblw_flag) {
    JSON_TBLW_START (FLAG_T, flag, flag->name == NULL);
    json_prop_string  (new, "name",     flag->name);
    json_prop_integer (new, "bit",      flag->bit);
    json_prop_boolean (new, "settable", flag->settable);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_clan) {
    JSON_TBLW_START (CLAN_T, clan, clan->name == NULL);
    json_prop_string  (new, "name",        JSTR (clan->name));
    json_prop_string  (new, "who_name",    JSTR (clan->who_name));
    json_prop_integer (new, "hall",        clan->hall);
    json_prop_boolean (new, "independent", clan->independent);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_position) {
    JSON_TBLW_START (POSITION_T, pos, pos->name == NULL);
    json_prop_integer (new, "position",   pos->pos);
    json_prop_string  (new, "name",       JSTR (pos->name));
    json_prop_string  (new, "long_name",  JSTR (pos->long_name));
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_sex) {
    JSON_TBLW_START (SEX_T, sex, sex->name == NULL);
    json_prop_integer (new, "sex",  sex->sex);
    json_prop_string  (new, "name", JSTR (sex->name));
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_size) {
    JSON_TBLW_START (SIZE_T, size, size->name == NULL);
    json_prop_integer (new, "size", size->size);
    json_prop_string  (new, "name", JSTR (size->name));
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_item) {
    JSON_TBLW_START (ITEM_T, item, item->name == NULL);
    json_prop_integer (new, "type", item->type);
    json_prop_string  (new, "name", JSTR (item->name));
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_weapon) {
    JSON_TBLW_START (WEAPON_T, weapon, weapon->name == NULL);
    json_prop_integer (new, "type", weapon->type);
    json_prop_string  (new, "name", JSTR (weapon->name));
    json_prop_integer (new, "newbie_vnum", weapon->newbie_vnum);
    /* TODO: gsn? */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_dam) {
    JSON_TBLW_START (DAM_T, dam, dam->name == NULL);
    /* TODO: properties for DAM_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_attack) {
    JSON_TBLW_START (ATTACK_T, attack, attack->name == NULL);
    /* TODO: properties for ATTACK_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_race) {
    JSON_TBLW_START (RACE_T, race, race->name == NULL);
    /* TODO: properties for RACE_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_pc_race) {
    JSON_TBLW_START (PC_RACE_T, pc_race, pc_race->name == NULL);
    /* TODO: properties for PC_RACE_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_class) {
    JSON_TBLW_START (CLASS_T, class, class->name == NULL);
    /* TODO: properties for CLASS_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_str_app) {
    JSON_TBLW_START (STR_APP_T, str_app, str_app->stat < 0);
    /* TODO: properties for STR_APP_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_int_app) {
    JSON_TBLW_START (INT_APP_T, int_app, int_app->stat < 0);
    /* TODO: properties for INT_APP_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_wis_app) {
    JSON_TBLW_START (WIS_APP_T, wis_app, wis_app->stat < 0);
    /* TODO: properties for WIS_APP_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_dex_app) {
    JSON_TBLW_START (DEX_APP_T, dex_app, dex_app->stat < 0);
    /* TODO: properties for DEX_APP_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_con_app) {
    JSON_TBLW_START (CON_APP_T, con_app, con_app->stat < 0);
    /* TODO: properties for CON_APP_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_liq) {
    JSON_TBLW_START (LIQ_T, liq, liq->name == NULL);
    /* TODO: properties for LIQ_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_skill) {
    JSON_TBLW_START (SKILL_T, skill, skill->name == NULL);
    /* TODO: properties for SKILL_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_group) {
    JSON_TBLW_START (GROUP_T, group, group->name == NULL);
    /* TODO: properties for GROUP_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_sector) {
    JSON_TBLW_START (SECTOR_T, sector, sector->name == NULL);
    /* TODO: properties for SECTOR_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_nanny) {
    JSON_TBLW_START (NANNY_HANDLER_T, nanny, nanny->name == NULL);
    /* TODO: properties for NANNY_HANDLER_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_door) {
    JSON_TBLW_START (DOOR_T, door, door->name == NULL);
    /* TODO: properties for DOOR_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_spec) {
    JSON_TBLW_START (SPEC_T, spec, spec->name == NULL);
    /* TODO: properties for SPEC_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_furniture) {
    JSON_TBLW_START (FURNITURE_BITS_T, furniture, furniture->name == NULL);
    /* TODO: properties for FURNITURE_BITS_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_wear_loc) {
    JSON_TBLW_START (WEAR_LOC_T, wear_loc, wear_loc->name == NULL);
    /* TODO: properties for WEAR_LOC_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_material) {
    JSON_TBLW_START (MATERIAL_T, material, material->name == NULL);
    /* TODO: properties for MATERIAL_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_colour_setting) {
    JSON_TBLW_START (COLOUR_SETTING_T, colour_setting, colour_setting->name == NULL);
    /* TODO: properties for COLOUR_SETTING_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_wiznet) {
    JSON_TBLW_START (WIZNET_T, wiznet, wiznet->name == NULL);
    /* TODO: properties for WIZNET_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_map_lookup) {
    JSON_TBLW_START (MAP_LOOKUP_TABLE_T, map_lookup, map_lookup->name == NULL);
    /* TODO: properties for MAP_LOOKUP_TABLE_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_map_flags) {
    JSON_TBLW_START (MAP_LOOKUP_TABLE_T, map_flags, map_flags->name == NULL);
    /* TODO: properties for MAP_LOOKUP_TABLE_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_obj_map) {
    JSON_TBLW_START (OBJ_MAP_T, obj_map, obj_map->item_type < 0);
    /* TODO: properties for OBJ_MAP_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_colour) {
    JSON_TBLW_START (COLOUR_T, colour, colour->name == NULL);
    /* TODO: properties for COLOUR_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_recycle) {
    JSON_TBLW_START (RECYCLE_T, recycle, recycle->name == NULL);
    /* TODO: properties for RECYCLE_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_board) {
    JSON_TBLW_START (BOARD_T, board, board->name == NULL);
    /* TODO: properties for BOARD_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_affect_bit) {
    JSON_TBLW_START (AFFECT_BIT_T, affect_bit, affect_bit->name == NULL);
    /* TODO: properties for AFFECT_BIT_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_day) {
    JSON_TBLW_START (DAY_T, day, day->name == NULL);
    /* TODO: properties for DAY_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_month) {
    JSON_TBLW_START (MONTH_T, month, month->name == NULL);
    /* TODO: properties for MONTH_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_sky) {
    JSON_TBLW_START (SKY_T, sky, sky->name == NULL);
    /* TODO: properties for SKY_T */
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_sun) {
    JSON_TBLW_START (SUN_T, sun, sun->name == NULL);
    /* TODO: properties for SUN_T */
    return new;
}
