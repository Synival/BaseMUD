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
    json_prop_integer (new, "type", dam->type);
    json_prop_string  (new, "name", JSTR (dam->name));
    json_prop_string  (new, "res_flags", JBITSF (res_flags, dam->res));
    json_prop_string  (new, "dam_flags", JBITSF (dam_flags, dam->dam_flags));
    /* TODO: dam->effect? */
/*
    EFFECT_FUN *effect;
*/
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
    JSON_TBLW_START (SKILL_T, skill, skill->name == NULL);
    /* TODO: properties for SKILL_T */
#if 0
    char *name;                    /* Name of skill               */
    sh_int skill_level[CLASS_MAX]; /* Level needed by class       */
    sh_int rating[CLASS_MAX];      /* How hard it is to learn     */
    SPELL_FUN *spell_fun;          /* Spell pointer (for spells)  */
    sh_int target;                 /* Legal targets               */
    sh_int minimum_position;       /* Position for caster / user  */
    sh_int *pgsn;                  /* Pointer to associated gsn   */
    sh_int slot;                   /* Slot for #OBJECT loading    */
    sh_int min_mana;               /* Minimum mana used           */
    sh_int beats;                  /* Waiting time after use      */
    char *noun_damage;             /* Damage message              */
    char *msg_off;                 /* Wear off message            */
    char *msg_obj;                 /* Wear off message for obects */
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_group) {
    JSON_TBLW_START (GROUP_T, group, group->name == NULL);
    /* TODO: properties for GROUP_T */
#if 0
    char *name;
    sh_int rating[CLASS_MAX];
    char *spells[MAX_IN_GROUP];
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_sector) {
    JSON_TBLW_START (SECTOR_T, sector, sector->name == NULL);
    /* TODO: properties for SECTOR_T */
#if 0
    int type;
    const char *name;
    int move_loss;
    char colour_char;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_nanny) {
    JSON_TBLW_START (NANNY_HANDLER_T, nanny, nanny->name == NULL);
    /* TODO: properties for NANNY_HANDLER_T */
#if 0
    int state;
    char *name;
    NANNY_FUN *const action;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_door) {
    JSON_TBLW_START (DOOR_T, door, door->name == NULL);
    /* TODO: properties for DOOR_T */
#if 0
    int dir;
    const char *name;
    const char *from;
    const char *to_phrase;
    int reverse;
    const char *short_name;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_spec) {
    JSON_TBLW_START (SPEC_T, spec, spec->name == NULL);
    /* TODO: properties for SPEC_T */
#if 0
    char *name;         /* special function name */
    SPEC_FUN *function; /* the function          */
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_furniture) {
    JSON_TBLW_START (FURNITURE_BITS_T, furniture, furniture->name == NULL);
    /* TODO: properties for FURNITURE_BITS_T */
#if 0
    int position;
    char *name;
    flag_t bit_at;
    flag_t bit_on;
    flag_t bit_in;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_wear_loc) {
    JSON_TBLW_START (WEAR_LOC_T, wear_loc, wear_loc->name == NULL);
    /* TODO: properties for WEAR_LOC_T */
#if 0
    int type;
    const char *name;
    const char *look_msg;
    flag_t wear_flag;
    const char *msg_wear_self, *msg_wear_room;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_material) {
    JSON_TBLW_START (MATERIAL_T, material, material->name == NULL);
    /* TODO: properties for MATERIAL_T */
#if 0
    int type;
    const char *name;
    char color;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_colour_setting) {
    JSON_TBLW_START (COLOUR_SETTING_T, colour_setting, colour_setting->name == NULL);
    /* TODO: properties for COLOUR_SETTING_T */
#if 0
    int index;
    char *name;
    char act_char;
    flag_t default_colour;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_wiznet) {
    JSON_TBLW_START (WIZNET_T, wiznet, wiznet->name == NULL);
    /* TODO: properties for WIZNET_T */
#if 0
    flag_t bit;
    char *name;
    int level;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_map_lookup) {
    JSON_TBLW_START (MAP_LOOKUP_TABLE_T, map_lookup, map_lookup->name == NULL);
    /* TODO: properties for MAP_LOOKUP_TABLE_T */
#if 0
    int index;
    char *name;
    const FLAG_T *flags;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_obj_map) {
    JSON_TBLW_START (OBJ_MAP_T, obj_map, obj_map->item_type < 0);
    /* TODO: properties for OBJ_MAP_T */
#if 0
    int item_type;
    const struct obj_map_value values[OBJ_VALUE_MAX];
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_colour) {
    JSON_TBLW_START (COLOUR_T, colour, colour->name == NULL);
    /* TODO: properties for COLOUR_T */
#if 0
    flag_t mask;
    flag_t code;
    char *name;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_board) {
    JSON_TBLW_START (BOARD_T, board, board->name == NULL);
    /* TODO: properties for BOARD_T */
#if 0
    char *name;      /* Max 8 chars */
    char *long_name; /* Explanatory text, should be no more than 40 ? chars */
    int read_level;  /* minimum level to see board */
    int write_level; /* minimum level to post notes */
    char *names;     /* Default recipient */
    int force_type;  /* Default action (DEF_XXX) */
    int purge_days;  /* Default expiration */
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_affect_bit) {
    JSON_TBLW_START (AFFECT_BIT_T, affect_bit, affect_bit->name == NULL);
    /* TODO: properties for AFFECT_BIT_T */
#if 0
    char *name;
    int type;
    const FLAG_T *flags;
    char *help;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_day) {
    JSON_TBLW_START (DAY_T, day, day->name == NULL);
    /* TODO: properties for DAY_T */
#if 0
    int type;
    const char *name;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_month) {
    JSON_TBLW_START (MONTH_T, month, month->name == NULL);
    /* TODO: properties for MONTH_T */
#if 0
    int type;
    const char *name;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_sky) {
    JSON_TBLW_START (SKY_T, sky, sky->name == NULL);
    /* TODO: properties for SKY_T */
#if 0
    int type;
    const char *name;
    const char *description;
    int mmhg_min;
    int mmhg_max;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_sun) {
    JSON_TBLW_START (SUN_T, sun, sun->name == NULL);
    /* TODO: properties for SUN_T */
#if 0
    int type;
    const char *name;
    bool is_dark;
    int hour_start;
    int hour_end;
    const char *message;
#endif
    return new;
}

DEFINE_TABLE_JSON_FUN (json_tblw_pose) {
    JSON_TBLW_START (POSE_T, pose, pose->class_index < 0);
#if 0
    int class_index;
    const char *message[MAX_LEVEL * 2 + 2];
#endif
    return new;
}
