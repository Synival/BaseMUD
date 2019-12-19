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

#include <string.h>

#include "json_obj.h"
#include "lookup.h"
#include "colour.h"

#include "json_tbl.h"

#define JSON_TBLW_START(vtype, var, null_check) \
    const vtype *var = obj; \
    JSON_T *new; \
    do { \
        if (null_check) \
            return NULL; \
        new = json_new_object (obj_name, JSON_OBJ_ANY); \
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
    json_prop_string  (new, "room_msg",
        JSTR (pos->room_msg));
    if (pos->room_msg_furniture != NULL)
        json_prop_string  (new, "room_msg_furniture",
            JSTR (pos->room_msg_furniture));
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
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_dam) {
    JSON_TBLW_START (DAM_T, dam, dam->name == NULL);

    json_prop_integer (new, "type", dam->type);
    json_prop_string  (new, "name", JSTR (dam->name));

    if (dam->res > 0)
        json_prop_string  (new, "res_flags", JBITSF (res_flags, dam->res));
    if (dam->dam_flags > 0)
    json_prop_string  (new, "dam_flags", JBITSF (dam_flags, dam->dam_flags));
    if (dam->effect != EFFECT_NONE)
        json_prop_string  (new, "effect", effect_get_name (dam->effect));

    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_attack) {
    const DAM_T *dam;
    JSON_TBLW_START (ATTACK_T, attack, attack->name == NULL);

    json_prop_string (new, "name", JSTR (attack->name));
    json_prop_string (new, "noun", JSTR (attack->noun));
    if ((dam = dam_get (attack->dam_type)) != NULL)
        json_prop_string (new, "dam_type", JSTR (dam->name));
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_race) {
    JSON_TBLW_START (RACE_T, race, race->name == NULL);
    json_prop_string  (new, "name", JSTR (race->name));
    json_prop_boolean (new, "playable", race->pc_race);

    if (race->mob > 0)
        json_prop_string (new, "mob_flags", JBITSF (mob_flags, race->mob));
    if (race->aff > 0)
        json_prop_string (new, "affect_flags", JBITSF (affect_flags, race->aff));
    if (race->off > 0)
        json_prop_string (new, "offense_flags", JBITSF (off_flags, race->off));
    if (race->imm > 0)
        json_prop_string (new, "immune_flags", JBITSF (res_flags, race->imm));
    if (race->res > 0)
        json_prop_string (new, "res_flags", JBITSF (res_flags, race->res));
    if (race->vuln > 0)
        json_prop_string (new, "vuln_flags", JBITSF (res_flags, race->vuln));
    if (race->form > 0)
        json_prop_string (new, "form", JBITSF (form_flags, race->form));
    if (race->parts > 0)
        json_prop_string (new, "parts", JBITSF (part_flags, race->parts));

    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_pc_race) {
    const CLASS_T *class;
    JSON_T *sub;
    int i;

    JSON_TBLW_START (PC_RACE_T, pc_race, pc_race->name == NULL);

    json_prop_string  (new, "name",     JSTR (pc_race->name));
    json_prop_string  (new, "who_name", JSTR (pc_race->who_name));
    json_prop_integer (new, "creation_points", pc_race->points);

    sub = json_prop_object (new, "class_exp", JSON_OBJ_ANY);
    for (i = 0; i < CLASS_MAX; i++) {
        if ((class = class_get (i)) == NULL)
            continue;
        json_prop_integer (sub, class->name, pc_race->class_mult[i]);
    }

    for (i = 0; i < PC_RACE_SKILL_MAX; i++)
        if (pc_race->skills[i] != NULL && pc_race->skills[i][0] != '\0')
            break;
    if (i != PC_RACE_SKILL_MAX) {
        sub = json_prop_array (new, "skills");
        for (i = 0; i < 5; i++)
            if (pc_race->skills[i] != NULL && pc_race->skills[i][0] != '\0')
                json_prop_string (sub, NULL, pc_race->skills[i]);
    }

    sub = json_prop_object (new, "base_stats", JSON_OBJ_ANY);
    for (i = 0; i < STAT_MAX; i++)
        json_prop_integer (sub, JBITSF (stat_types, i), pc_race->stats[i]);

    sub = json_prop_object (new, "max_stats", JSON_OBJ_ANY);
    for (i = 0; i < STAT_MAX; i++)
        json_prop_integer (sub, JBITSF (stat_types, i), pc_race->max_stats[i]);

    json_prop_string (new, "size", JBITSF (size_types, pc_race->size));

    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_class) {
    JSON_TBLW_START (CLASS_T, class, class->name == NULL);

    json_prop_integer (new, "index", class->type);
    json_prop_string  (new, "name", JSTR (class->name));
    json_prop_string  (new, "who_name", JSTR (class->who_name));
    json_prop_string  (new, "primary_stat",
        JBITSF (stat_types, class->attr_prime));
    json_prop_integer (new, "skill_adept", class->skill_adept);
    json_prop_integer (new, "thac0_00", class->thac0_00);
    json_prop_integer (new, "thac0_32", class->thac0_32);
    json_prop_integer (new, "hp_gain_min", class->hp_min);
    json_prop_integer (new, "hp_gain_max", class->hp_max);
    json_prop_boolean (new, "gains_mana",  class->gains_mana);
    json_prop_string  (new, "base_group", JSTR (class->base_group));
    json_prop_string  (new, "default_group", JSTR (class->default_group));

    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_str_app) {
    JSON_TBLW_START (STR_APP_T, str_app, str_app->stat < 0);
    json_prop_integer (new, "stat", str_app->stat);
    json_prop_integer (new, "hitroll_bonus", str_app->tohit);
    json_prop_integer (new, "damroll_bonus", str_app->todam);
    json_prop_integer (new, "carry_bonus", str_app->carry);
    json_prop_integer (new, "max_wield_weight", str_app->wield);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_int_app) {
    JSON_TBLW_START (INT_APP_T, int_app, int_app->stat < 0);
    json_prop_integer (new, "stat", int_app->stat);
    json_prop_integer (new, "learn_rate", int_app->learn);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_wis_app) {
    JSON_TBLW_START (WIS_APP_T, wis_app, wis_app->stat < 0);
    json_prop_integer (new, "stat", wis_app->stat);
    json_prop_integer (new, "practices", wis_app->practice);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_dex_app) {
    JSON_TBLW_START (DEX_APP_T, dex_app, dex_app->stat < 0);
    json_prop_integer (new, "stat", dex_app->stat);
    json_prop_integer (new, "defense_bonus", dex_app->defensive);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_con_app) {
    JSON_TBLW_START (CON_APP_T, con_app, con_app->stat < 0);
    json_prop_integer (new, "stat", con_app->stat);
    json_prop_integer (new, "level_hp", con_app->hitp);
    json_prop_integer (new, "shock", con_app->shock);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_liq) {
    JSON_T *sub;
    int i;
    JSON_TBLW_START (LIQ_T, liq, liq->name == NULL);

    json_prop_string (new, "name", JSTR (liq->name));
    json_prop_string (new, "color_name", JSTR (liq->color));

    sub = json_prop_object (new, "conditions", JSON_OBJ_ANY);
    for (i = 0; i < COND_MAX; i++)
        json_prop_integer (sub, JBITSF (cond_types, i), liq->cond[i]);

    json_prop_integer (new, "serving_size", liq->serving_size);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_skill) {
    JSON_T *sub, *sub2;
    int i;
    JSON_TBLW_START (SKILL_T, skill, skill->name == NULL);

    json_prop_string (new, "name", JSTR (skill->name));

    sub = json_prop_object (new, "classes", JSON_OBJ_ANY);
    for (i = 0; i < CLASS_MAX; i++) {
        sub2 = json_prop_object (sub, class_get_name (i), JSON_OBJ_ANY);
        json_prop_integer (sub2, "level",  skill->classes[i].level);
        json_prop_integer (sub2, "effort", skill->classes[i].effort);
    }

    json_prop_string (new, "target", JBITSF (skill_target_types,
        skill->target));
    json_prop_string (new, "min_position", JBITSF (position_types,
        skill->minimum_position));

    if (skill->slot != 0)
        json_prop_integer (new, "slot", skill->slot);
    if (skill->min_mana != 0)
        json_prop_integer (new, "min_mana", skill->min_mana);
    json_prop_integer (new, "usage_beats", skill->beats);

    if (skill->noun_damage && skill->noun_damage[0] != '\0')
        json_prop_string (new, "damage_noun", JSTR (skill->noun_damage));
    if (skill->msg_off && skill->msg_off[0] != '\0')
        json_prop_string (new, "off_msg_char", JSTR (skill->msg_off));
    if (skill->msg_obj && skill->msg_obj[0] != '\0')
        json_prop_string (new, "off_msg_obj", JSTR (skill->msg_obj));

    /* TODO: pgsn?  this is internal, but should it be considered here? */
    /* TODO: skill->spell_fun */

    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_skill_group) {
    const CLASS_T *class;
    JSON_T *sub, *sub2;
    int i;
    JSON_TBLW_START (SKILL_GROUP_T, group, group->name == NULL);

    json_prop_string (new, "name", JSTR (group->name));

    sub = json_prop_object (new, "classes", JSON_OBJ_ANY);
    for (i = 0; i < CLASS_MAX; i++) {
        if ((class = class_get (i)) == NULL)
            break;
        if (group->classes[i].cost < 0)
            continue;
        sub2 = json_prop_object (sub, class->name, JSON_OBJ_ANY);
        json_prop_integer (sub2, "cost", group->classes[i].cost);
    }

    sub = json_prop_array (new, "skills");
    for (i = 0; i < MAX_IN_GROUP; i++)
        if (group->spells[i] && group->spells[i][0] != '\0')
            json_prop_string (sub, NULL, JSTR (group->spells[i]));

    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_sector) {
    char color_buf[2];
    JSON_TBLW_START (SECTOR_T, sector, sector->name == NULL);

    json_prop_integer (new, "type", sector->type);
    json_prop_string  (new, "name", JSTR (sector->name));
    json_prop_integer (new, "move_loss", sector->move_loss);

    color_buf[0] = sector->colour_char;
    color_buf[1] = '\0';
    json_prop_string (new, "color_char", color_buf);

    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_door) {
    JSON_TBLW_START (DOOR_T, door, door->name == NULL);
    json_prop_integer (new, "dir",         door->dir);
    json_prop_integer (new, "reverse",     door->reverse);
    json_prop_string  (new, "name",        JSTR (door->name));
    json_prop_string  (new, "short_name",  JSTR (door->short_name));
    json_prop_string  (new, "to_phrase",   JSTR (door->to_phrase));
    json_prop_string  (new, "from_phrase", JSTR (door->from_phrase));
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_spec) {
    JSON_TBLW_START (SPEC_T, spec, spec->name == NULL);
    json_prop_string (new, "name", spec->name);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_furniture) {
    JSON_TBLW_START (FURNITURE_BITS_T, furniture, furniture->name == NULL);
    json_prop_string (new, "name", JSTR (furniture->name));
    json_prop_string (new, "position", JBITSF (position_types,
        furniture->position));
    json_prop_string (new, "bit_at", JBITSF (furniture_flags,
        furniture->bit_at));
    json_prop_string (new, "bit_on", JBITSF (furniture_flags,
        furniture->bit_on));
    json_prop_string (new, "bit_in", JBITSF (furniture_flags,
        furniture->bit_in));
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_wear_loc) {
    JSON_TBLW_START (WEAR_LOC_T, wear_loc, wear_loc->name == NULL);
    json_prop_integer (new, "type", wear_loc->type);
    json_prop_string  (new, "name", JSTR (wear_loc->name));
    json_prop_string  (new, "wear_flag", JBITSF (wear_flags,
        wear_loc->wear_flag));
    json_prop_string  (new, "look_msg", JSTR (wear_loc->look_msg));
    json_prop_string  (new, "wear_msg_self", JSTR (wear_loc->msg_wear_self));
    json_prop_string  (new, "wear_msg_room", JSTR (wear_loc->msg_wear_room));
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_material) {
    char color_buf[2];
    JSON_TBLW_START (MATERIAL_T, material, material->name == NULL);

    json_prop_integer (new, "type", material->type);
    json_prop_string  (new, "name", JSTR (material->name));

    color_buf[0] = material->color;
    color_buf[1] = '\0';
    json_prop_string  (new, "color_char", color_buf);

    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_colour_setting) {
    char color_buf[256];
    JSON_TBLW_START (COLOUR_SETTING_T, colour_setting, colour_setting->name == NULL);

    json_prop_integer (new, "index", colour_setting->index);
    json_prop_string  (new, "name", JSTR (colour_setting->name));

    color_buf[0] = colour_setting->act_char;
    color_buf[1] = '\0';
    json_prop_string (new, "color_char", color_buf);

    colour_to_full_name (colour_setting->default_colour, color_buf,
        sizeof (color_buf));
    json_prop_string (new, "default_color", JSTR (color_buf));

    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_colour) {
    JSON_TBLW_START (COLOUR_T, colour, colour->name == NULL);
    json_prop_string  (new, "name", JSTR (colour->name));
    json_prop_integer (new, "group_mask", colour->mask);
    json_prop_integer (new, "code",       colour->code);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_board) {
    JSON_TBLW_START (BOARD_T, board, board->name == NULL);
    json_prop_string  (new, "name", JSTR (board->name));
    json_prop_string  (new, "full_name", JSTR (board->long_name));
    json_prop_integer (new, "read_level", board->read_level);
    json_prop_integer (new, "write_level", board->write_level);
    json_prop_string  (new, "recipients", JSTR (board->names));
    json_prop_string  (new, "force_type", JBITSF (board_def_types,
        board->force_type));
    json_prop_integer (new, "purge_days", board->purge_days);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_day) {
    JSON_TBLW_START (DAY_T, day, day->name == NULL);
    json_prop_integer (new, "index", day->type);
    json_prop_string  (new, "name",  JSTR (day->name));
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_month) {
    JSON_TBLW_START (MONTH_T, month, month->name == NULL);
    json_prop_integer (new, "index", month->type);
    json_prop_string  (new, "name",  JSTR (month->name));
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_sky) {
    JSON_TBLW_START (SKY_T, sky, sky->name == NULL);
    json_prop_integer (new, "index",       sky->type);
    json_prop_string  (new, "name",        JSTR (sky->name));
    json_prop_string  (new, "description", JSTR (sky->description));
    json_prop_integer (new, "mmhg_min",    sky->mmhg_min);
    json_prop_integer (new, "mmhg_max",    sky->mmhg_max);
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_sun) {
    JSON_TBLW_START (SUN_T, sun, sun->name == NULL);
    json_prop_integer (new, "index",      sun->type);
    json_prop_string  (new, "name",       JSTR (sun->name));
    json_prop_boolean (new, "is_dark",    sun->is_dark);
    json_prop_integer (new, "hour_start", sun->hour_start);
    json_prop_integer (new, "hour_end",   sun->hour_end);
    json_prop_string  (new, "message",    JSTR (sun->message));
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_pose) {
    JSON_T *sub, *sub2;
    int i;
    JSON_TBLW_START (POSE_T, pose, pose->class_index < 0);

    json_prop_string (new, "class", class_get_name (pose->class_index));
    sub = json_prop_array (new, "poses");
    for (i = 0; i < MAX_LEVEL; i++) {
        if (pose->message[i * 2] == NULL)
            continue;
        sub2 = json_prop_object (sub, NULL, JSON_OBJ_ANY);
        json_prop_string (sub2, "msg_self", pose->message[i * 2 + 0]);
        json_prop_string (sub2, "msg_others", pose->message[i * 2 + 1]);
    }
    return new;
}
