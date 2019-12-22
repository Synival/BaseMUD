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

#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "interp.h"
#include "db.h"
#include "recycle.h"
#include "globals.h"

#include "lookup.h"

int lookup_backup (int (*func) (const char *str), char *str, char *errf,
    int backup)
{
    int num = func (str);
    if (num >= 0)
        return num;
    bugf (errf, str);
    return backup;
}

bool is_table_flagged (const void *table, flag_t t_flags, flag_t f_flags) {
    int i;
    for (i = 0; master_table[i].table; i++)
        if (master_table[i].table == table)
            return (ARE_SET (master_table[i].flags, t_flags) &&
                   NONE_SET (master_table[i].flags, f_flags)) ? TRUE : FALSE;
    return FALSE;
}

bool is_flag (const void *table)
    { return is_table_flagged (table, TABLE_FLAG_TYPE | TABLE_BITS, 0); }
bool is_type (const void *table)
    { return is_table_flagged (table, TABLE_FLAG_TYPE, TABLE_BITS); }
bool is_special (const void *table)
    { return is_table_flagged (table, 0, TABLE_FLAG_TYPE); }

flag_t flag_value (const FLAG_T *flag_table, const char *argument)
    { return flag_value_real (flag_table, argument, NO_FLAG, FALSE); }

flag_t flag_value_real (const FLAG_T *flag_table, const char *argument,
    flag_t no_flag, bool exact)
{
    char word[MAX_INPUT_LENGTH];
    bool found = FALSE;
    flag_t bit;
    flag_t marked = 0;

    RETURN_IF_BUG (is_special (flag_table),
        "flag_value: cannot yet look up values for special types :(", 0, 0);
    if (!is_flag (flag_table)) {
        bit = exact
            ? flag_lookup_exact (argument, flag_table)
            : flag_lookup (argument, flag_table);
        return (bit == NO_FLAG) ? no_flag : bit;
    }

    /* Accept multiple flags. */
    while (1) {
        argument = one_argument (argument, word);
        if (word[0] == '\0')
            break;
        bit = exact
            ? flag_lookup_exact (word, flag_table)
            : flag_lookup (word, flag_table);
        if (bit != NO_FLAG) {
            SET_BIT (marked, bit);
            found = TRUE;
        }
    }
    return (found) ? marked : no_flag;
}

/* Increased buffers from 2 to 16! That should give us the illusion
 * of stability. -- Synival */
const char *flag_string (const FLAG_T *flag_table, flag_t bits)
    { return flag_string_real (flag_table, bits, "none"); }

const char *flag_string_real (const FLAG_T *flag_table, flag_t bits,
    const char *none_str)
{
    static char buf[16][512];
    static int cnt = 0;
    int i;
    const TABLE_T *mt;
    bool bitflag;

    if (++cnt >= 16)
        cnt = 0;

    for (i = 0; master_table[i].table; i++)
        if (master_table[i].table == flag_table)
            break;
    mt = &(master_table[i]);
    if (mt->table == NULL)
        return none_str;
    RETURN_IF_BUG (!IS_SET(mt->flags, TABLE_FLAG_TYPE),
        "flag_type: cannot yet lookup special tables :(", 0, none_str);

    buf[cnt][0] = '\0';
    bitflag = IS_SET (mt->flags, TABLE_BITS);
    for (i = 0; flag_table[i].name != NULL; i++) {
        if (bitflag && IS_SET (bits, flag_table[i].bit)) {
            strcat (buf[cnt], " ");
            strcat (buf[cnt], flag_table[i].name);
        }
        else if (!bitflag && bits == flag_table[i].bit) {
            strcat (buf[cnt], " ");
            strcat (buf[cnt], flag_table[i].name);
            break;
        }
    }
    return (buf[cnt][0] != '\0') ? buf[cnt] + 1 : none_str;
}

flag_t flag_lookup (const char *name, const FLAG_T *flag_table)
    { SIMPLE_LOOKUP_PROP (flag_table, bit, name, NO_FLAG, 0); }
flag_t flag_lookup_exact (const char *name, const FLAG_T *flag_table)
    { SIMPLE_LOOKUP_PROP_EXACT (flag_table, bit, name, NO_FLAG, 0); }
const FLAG_T *flag_get_by_name (const char *name, const FLAG_T *flag_table)
    { SIMPLE_GET_BY_NAME (flag_table, name, 0); }
const FLAG_T *flag_get_by_name_exact (const char *name, const FLAG_T *flag_table)
    { SIMPLE_GET_BY_NAME_EXACT (flag_table, name, 0); }
const FLAG_T *flag_get (flag_t bit, const FLAG_T *flag_table)
    { SIMPLE_GET (flag_table, bit, name, NULL, 0); }
const char *flag_get_name (flag_t bit, const FLAG_T *flag_table)
    { SIMPLE_GET_NAME_FROM_ELEMENT (FLAG_T, flag_get (bit, flag_table), name); }

/* Lookup bundles. */
SIMPLE_INDEX_BUNDLE (master,   TABLE_T,    0);

SIMPLE_INDEX_BUNDLE (attack,   ATTACK_T,   ATTACK_MAX);
SIMPLE_INDEX_BUNDLE (board,    BOARD_T,    BOARD_MAX);
SIMPLE_INDEX_BUNDLE (clan,     CLAN_T,     CLAN_MAX);
SIMPLE_INDEX_BUNDLE (class,    CLASS_T,    CLASS_MAX);
SIMPLE_INDEX_BUNDLE (liq,      LIQ_T,      LIQ_MAX);
SIMPLE_INDEX_BUNDLE (race,     RACE_T,     RACE_MAX);
SIMPLE_INDEX_BUNDLE (skill,    SKILL_T,    SKILL_MAX);
SIMPLE_INDEX_BUNDLE (skill_group, SKILL_GROUP_T, SKILL_GROUP_MAX);
SIMPLE_INDEX_BUNDLE (spec,     SPEC_T,     SPEC_MAX);

SIMPLE_HASH_BUNDLE (affect_bit, AFFECT_BIT_T,       type);
SIMPLE_HASH_BUNDLE (colour,     COLOUR_T,           code);
SIMPLE_HASH_BUNDLE (colour_setting, COLOUR_SETTING_T, index);
SIMPLE_HASH_BUNDLE (dam,        DAM_T,              type);
SIMPLE_HASH_BUNDLE (day,        DAY_T,              type);
SIMPLE_HASH_BUNDLE (door,       DOOR_T,             dir);
SIMPLE_HASH_BUNDLE (effect,     EFFECT_T,           type);
SIMPLE_HASH_BUNDLE (furniture,  FURNITURE_BITS_T,   position);
SIMPLE_HASH_BUNDLE (item,       ITEM_T,             type);
SIMPLE_HASH_BUNDLE (map_flags,  MAP_LOOKUP_TABLE_T, index);
SIMPLE_HASH_BUNDLE (map_lookup, MAP_LOOKUP_TABLE_T, index);
SIMPLE_HASH_BUNDLE (material,   MATERIAL_T,         type);
SIMPLE_HASH_BUNDLE (month,      MONTH_T,            type);
SIMPLE_HASH_BUNDLE (nanny,      NANNY_HANDLER_T,    state);
SIMPLE_HASH_BUNDLE (position,   POSITION_T,         pos);
SIMPLE_HASH_BUNDLE (recycle,    RECYCLE_T,          type);
SIMPLE_HASH_BUNDLE (sector,     SECTOR_T,           type);
SIMPLE_HASH_BUNDLE (sex,        SEX_T,              sex);
SIMPLE_HASH_BUNDLE (size,       SIZE_T,             size);
SIMPLE_HASH_BUNDLE (sky,        SKY_T,              type);
SIMPLE_HASH_BUNDLE (sun,        SUN_T,              type);
SIMPLE_HASH_BUNDLE (weapon,     WEAPON_T,           type);
SIMPLE_HASH_BUNDLE (wear_loc,   WEAR_LOC_T,         type);
SIMPLE_HASH_BUNDLE (wiznet,     WIZNET_T,           bit);

SIMPLE_REC_BUNDLE (area,        AREA_T,        RECYCLE_AREA_T);
SIMPLE_REC_BUNDLE (ban,         BAN_T,         RECYCLE_BAN_T);
SIMPLE_REC_BUNDLE (had,         HELP_AREA_T,   RECYCLE_HELP_AREA_T);
SIMPLE_REC_BUNDLE (help,        HELP_T,        RECYCLE_HELP_T);
SIMPLE_REC_BUNDLE (obj_index,   OBJ_INDEX_T,   RECYCLE_OBJ_INDEX_T);
SIMPLE_REC_BUNDLE (portal_exit, PORTAL_EXIT_T, RECYCLE_PORTAL_EXIT_T);
SIMPLE_REC_BUNDLE (room_index,  ROOM_INDEX_T,  RECYCLE_ROOM_INDEX_T);
SIMPLE_REC_BUNDLE (social,      SOCIAL_T,      RECYCLE_SOCIAL_T);

const TABLE_T *master_table_get_exact (const char *name) {
    int i;
    for (i = 0; master_table[i].name != NULL; i++)
        if (str_cmp (master_table[i].name, name) == 0)
            return &(master_table[i]);
    return NULL;
}

const TABLE_T *master_table_get_by_obj_name (const char *name) {
    int i;
    for (i = 0; master_table[i].name != NULL; i++) {
        if (master_table[i].obj_name != NULL &&
            str_cmp (master_table[i].obj_name, name) == 0)
        {
            return &(master_table[i]);
        }
    }
    return NULL;
}

SPEC_FUN *spec_lookup_function (const char *name)
    { SIMPLE_LOOKUP_PROP (spec_table, function, name, NULL, SPEC_MAX); }

const char *spec_function_name (SPEC_FUN *function) {
    int i;
    for (i = 0; spec_table[i].function != NULL; i++)
        if (function == spec_table[i].function)
            return spec_table[i].name;
    return NULL;
}

const OBJ_MAP_T *obj_map_get (int item_type)
    { SIMPLE_GET (obj_map_table, item_type, item_type, -1, 0); }

const char *map_lookup_get_string (int index, flag_t value) {
    const MAP_LOOKUP_TABLE_T *lookup = map_lookup_get (index);

    if (lookup == NULL)
        return NULL;
    if (lookup->flags)
        return flag_get_name (value, lookup->flags);

    switch (lookup->index) {
        case MAP_LOOKUP_WEAPON_TYPE: return weapon_get_name (value);
        case MAP_LOOKUP_ATTACK_TYPE: return attack_get_name (value);
        case MAP_LOOKUP_SKILL:       return skill_get_name  (value);
        case MAP_LOOKUP_LIQUID:      return liq_get_name    (value);

        default:
            bugf ("map_lookup_get_string: Unhandled type '%s'", lookup->name);
            return NULL;
    }
}

flag_t map_lookup_get_type (int index, const char *str) {
    const MAP_LOOKUP_TABLE_T *lookup = map_lookup_get (index);

    if (lookup == NULL)
        return 0;

    switch (lookup->index) {
        case MAP_LOOKUP_WEAPON_TYPE: return weapon_lookup (str);
        case MAP_LOOKUP_ATTACK_TYPE: return attack_lookup (str);
        case MAP_LOOKUP_SKILL:       return skill_lookup  (str);
        case MAP_LOOKUP_LIQUID:      return liq_lookup    (str);

        default:
            bugf ("map_lookup_get_values(): Unhandled type '%s'", lookup->name);
            return 0;
    }
}

int map_flags_get_string (int index, flag_t value, char *buf, size_t size) {
    const MAP_LOOKUP_TABLE_T *lookup = map_flags_get (index);
    const char *str = NULL;

    if (lookup == NULL)
        return 0;

    if (lookup->flags)
        str = flag_string (lookup->flags, value);
    else {
        switch (lookup->index) {
            default:
                bugf ("map_flags_get_string: Unhandled type '%s'",
                    lookup->name);
                return 0;
        }
    }

    if (str == NULL || !strcmp (str, "none"))
        buf[0] = '\0';
    else
        snprintf (buf, size, "%s", str);

    return 1;
}

flag_t map_flags_get_value (int index, const char *str) {
    const MAP_LOOKUP_TABLE_T *lookup = map_flags_get (index);

    if (lookup == NULL)
        return 0;
    if (str == NULL || (strcmp (str, "none") == 0))
        return 0;
    if (lookup->flags == NULL) {
        fprintf (stderr, "map_flags_get_value(): MAP_LOOKUP_TABLE_T '%s' "
            "has no flags\n", lookup->name);
        return 0;
    }
    return flag_value_real (lookup->flags, str, 0, TRUE);
}

const OBJ_MAP_VALUE_T *obj_map_value_get (const OBJ_MAP_T *map, int index) {
    int i;
    if (map == NULL)
        return NULL;
    for (i = 0; i < OBJ_VALUE_MAX; i++)
        if (map->values[i].index == index)
            return (map->values + i);
    return NULL;
}

const TABLE_T *master_get_first (void)
    { return &(master_table[0]); }
const TABLE_T *master_get_next (const TABLE_T *table)
    { return table[1].name == NULL ? NULL : &(table[1]); }

AREA_T *area_get_by_vnum (int vnum) {
    AREA_T *a;
    for (a = area_get_first(); a; a = area_get_next (a))
        if (a->vnum == vnum)
            return a;
    return NULL;
}

AREA_T *area_get_by_filename (const char *filename) {
    AREA_T *a;
    for (a = area_get_first(); a; a = area_get_next (a))
        if (strcmp (filename, a->filename) == 0)
            return a;
    return NULL;
}

AREA_T *area_get_by_inner_vnum (int vnum) {
    AREA_T *a;
    for (a = area_get_first(); a; a = area_get_next (a))
        if (vnum >= a->min_vnum && vnum <= a->max_vnum)
            return a;
    return NULL;
}

flag_t wear_loc_get_flag (int wear_loc) {
    const WEAR_LOC_T *loc = wear_loc_get (wear_loc);
    return loc ? loc->wear_flag : 0;
}

HELP_AREA_T *help_area_get_by_help (HELP_T *help) {
    HELP_AREA_T *had;
    HELP_T *h;
    for (had = had_first; had; had = had->next)
        for (h = had->first; h; h = h->next_area)
            if (h == help)
                return had;
    return NULL;
}

HELP_AREA_T *help_area_get_by_filename (const char *filename) {
    HELP_AREA_T *had;
    for (had = had_first; had; had = had->next)
        if (strcmp (had->filename, filename) == 0)
            return had;
    return NULL;
}

const DAY_T *day_get_current ()
    { return day_get ((time_info.day + 1) % DAY_MAX); }
const MONTH_T *month_get_current ()
    { return month_get (time_info.month); }
const SKY_T *sky_get_current ()
    { return sky_get (weather_info.sky); }
const SUN_T *sun_get_current ()
    { return sun_get (weather_info.sunlight); }

const SKY_T *sky_get_by_mmhg (int mmhg) {
    const SKY_T *sky;
    int i;
    for (i = 0; i < SKY_MAX; i++) {
        sky = &(sky_table[i]);
        if ((mmhg >= sky->mmhg_min || sky->mmhg_min == -1) &&
            (mmhg <  sky->mmhg_max || sky->mmhg_max == -1))
            return sky;
    }
    return &(sky_table[0]);
}

const SUN_T *sun_get_by_hour (int hour) {
    int i;
    for (i = 0; i < SUN_MAX; i++)
        if (hour >= sun_table[i].hour_start && hour < sun_table[i].hour_end)
            return &(sun_table[i]);
    return &(sun_table[0]);
}

/* Lookup a skill by slot number. Used for object loading. */
int skill_get_index_by_slot (int slot) {
    extern bool in_boot_db;
    int sn;

    if (slot <= 0)
        return -1;
    for (sn = 0; sn < SKILL_MAX; sn++)
        if (slot == skill_table[sn].slot)
            return sn;

    EXIT_IF_BUG (in_boot_db,
        "skill_get_index_by_slot: bad slot %d.", slot);
    return -1;
}

const char *ac_rating_phrase (int ac) {
         if (ac >=  101) return "hopelessly vulnerable to";
    else if (ac >=   80) return "defenseless against";
    else if (ac >=   60) return "barely protected from";
    else if (ac >=   40) return "slightly armored against";
    else if (ac >=   20) return "somewhat armored against";
    else if (ac >=    0) return "armored against";
    else if (ac >=  -20) return "well-armored against";
    else if (ac >=  -40) return "very well-armored against";
    else if (ac >=  -60) return "heavily armored against";
    else if (ac >=  -80) return "superbly armored against";
    else if (ac >= -100) return "almost invulnerable to";
    else                 return "divinely armored against";
}

const char *align_name (int align) {
         if (align >  900) return "angelic";
    else if (align >  700) return "saintly";
    else if (align >  350) return "good";
    else if (align >  100) return "kind";
    else if (align > -100) return "neutral";
    else if (align > -350) return "mean";
    else if (align > -700) return "evil";
    else if (align > -900) return "demonic";
    else                   return "satanic";
}

const char *wiz_class_by_level (int level) {
    switch (level) {
        case IMPLEMENTOR: return "IMP";
        case CREATOR:     return "CRE";
        case SUPREME:     return "SUP";
        case DEITY:       return "DEI";
        case GOD:         return "GOD";
        case IMMORTAL:    return "IMM";
        case DEMI:        return "DEM";
        case ANGEL:       return "ANG";
        case AVATAR:      return "AVA";
        default:          return NULL;
    }
}

const char *position_name (int position) {
    return (position < POS_DEAD || position > POS_STANDING)
        ? "an unknown position (this is a bug!)"
        : position_table[position].long_name;
}

const char *affect_apply_name (flag_t type)
    { return flag_string (affect_apply_types, type); }
const char *room_bit_name (flag_t flags)
    { return flag_string (room_flags, flags); }
const char *affect_bit_name (flag_t flags)
    { return flag_string (affect_flags, flags); }
const char *extra_bit_name (flag_t flags)
    { return flag_string (extra_flags, flags); }
const char *mob_bit_name (flag_t flags)
    { return flag_string (mob_flags, flags); }
const char *plr_bit_name (flag_t flags)
    { return flag_string (plr_flags, flags); }
const char *comm_bit_name (flag_t flags)
    { return flag_string (comm_flags, flags); }
const char *res_bit_name (flag_t flags)
    { return flag_string (res_flags, flags); }
const char *wear_flag_name (flag_t flags)
    { return flag_string (wear_flags, flags); }
const char *form_bit_name (flag_t flags)
    { return flag_string (form_flags, flags); }
const char *part_bit_name (flag_t flags)
    { return flag_string (part_flags, flags); }
const char *weapon_bit_name (flag_t flags)
    { return flag_string (weapon_flags, flags); }
const char *cont_bit_name (flag_t flags)
    { return flag_string (container_flags, flags); }
const char *off_bit_name (flag_t flags)
    { return flag_string (off_flags, flags); }
const char *sex_name (int sex)
    { return flag_string (sex_types, sex); }
const char *ac_type_name (int type)
    { return flag_string (ac_types, type); }

const STR_APP_T *str_app_get (int attr)
    { return str_app_table + URANGE(0, attr, ATTRIBUTE_HIGHEST); }
const INT_APP_T *int_app_get (int attr)
    { return int_app_table + URANGE(0, attr, ATTRIBUTE_HIGHEST); }
const WIS_APP_T *wis_app_get (int attr)
    { return wis_app_table + URANGE(0, attr, ATTRIBUTE_HIGHEST); }
const DEX_APP_T *dex_app_get (int attr)
    { return dex_app_table + URANGE(0, attr, ATTRIBUTE_HIGHEST); }
const CON_APP_T *con_app_get (int attr)
    { return con_app_table + URANGE(0, attr, ATTRIBUTE_HIGHEST); }

PORTAL_EXIT_T *portal_exit_lookup_exact (const char *name) {
    PORTAL_EXIT_T *pex;
    for (pex = portal_exit_get_first(); pex; pex = portal_exit_get_next (pex))
        if (strcmp (pex->name, name) == 0)
            return pex;
    return NULL;
}

SOCIAL_T *social_lookup_exact (const char *name) {
    SOCIAL_T *soc;
    for (soc = social_get_first(); soc; soc = social_get_next (soc))
        if (strcmp (soc->name, name) == 0)
            return soc;
    return NULL;
}

const CONDITION_T *condition_get_for_char (const CHAR_T *ch) {
    const CONDITION_T *cond;
    int i, percent;

    percent = (ch->max_hit > 0) ? ((ch->hit * 100) / ch->max_hit) : -1;
    if (ch->hit > 0 && percent == 0)
        percent = 1;

    for (i = 0; condition_table[i].hp_percent != -999; i++) {
        cond = &(condition_table[i]);
        if (percent >= cond->hp_percent || cond->hp_percent <= -100)
            return cond;
    }
    return NULL;
}
