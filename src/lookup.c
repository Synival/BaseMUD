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
#include <stdlib.h>

#include "utils.h"
#include "interp.h"
#include "db.h"
#include "recycle.h"

#include "lookup.h"

/* TODO: the fewer macros we can have here, the better... */
/* TODO: use more suitable names for ARRAY and HASH lookup functions. */
/* TODO: lookup distinctions between sequential arrays that correspond
 *       to their appropriate #defines */

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

flag_t flag_value (const FLAG_TYPE *flag_table, char *argument) {
    char word[MAX_INPUT_LENGTH];
    bool found = FALSE;
    flag_t bit;
    flag_t marked = 0;

    if (is_special (flag_table)) {
        bug ("flag_value: cannot yet look up values for special types :(", 0);
        return 0;
    }
    if (!is_flag (flag_table))
        return flag_lookup (argument, flag_table);

    /* Accept multiple flags. */
    while (1) {
        argument = one_argument (argument, word);
        if (word[0] == '\0')
            break;
        if ((bit = flag_lookup (word, flag_table)) != NO_FLAG) {
            SET_BIT (marked, bit);
            found = TRUE;
        }
    }
    if (found)
        return marked;
    else
        return NO_FLAG;
}

/* Increased buffers from 2 to 16! That should give us the illusion
 * of stability. -- Synival */
char *flag_string (const FLAG_TYPE *flag_table, flag_t bits) {
    static char buf[16][512];
    static int cnt = 0;
    int i;
    const TABLE_TYPE *mt;
    bool bitflag;

    if (++cnt >= 16)
        cnt = 0;

    for (i = 0; master_table[i].table; i++)
        if (master_table[i].table == flag_table)
            break;
    mt = &(master_table[i]);
    if (mt->table == NULL)
        return "none";
    if (!IS_SET(mt->flags, TABLE_FLAG_TYPE)) {
        bug ("flag_type: cannot yet lookup special tables :(", 0);
        return "none";
    }

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
    return (buf[cnt][0] != '\0') ? buf[cnt] + 1 : "none";
}

flag_t flag_lookup (const char *name, const FLAG_TYPE *flag_table)
    { SIMPLE_LOOKUP_PROP (flag_table, bit, name, NO_FLAG, 0); }
flag_t flag_lookup_exact (const char *name, const FLAG_TYPE *flag_table)
    { SIMPLE_LOOKUP_PROP_EXACT (flag_table, bit, name, NO_FLAG, 0); }
const FLAG_TYPE *flag_get_by_name (const char *name, const FLAG_TYPE *flag_table)
    { SIMPLE_GET_BY_NAME (flag_table, name, 0); }
const FLAG_TYPE *flag_get_by_name_exact (const char *name, const FLAG_TYPE *flag_table)
    { SIMPLE_GET_BY_NAME_EXACT (flag_table, name, 0); }
const FLAG_TYPE *flag_get (flag_t bit, const FLAG_TYPE *flag_table)
    { SIMPLE_GET (flag_table, bit, name, NULL, 0); }
const char *flag_get_name (flag_t bit, const FLAG_TYPE *flag_table)
    { SIMPLE_GET_NAME (FLAG_TYPE, flag_get(bit, flag_table), name); }

SIMPLE_ARRAY_BUNDLE (clan,     CLAN_TYPE,     CLAN_MAX);
SIMPLE_ARRAY_BUNDLE (position, POSITION_TYPE, POS_MAX);
SIMPLE_ARRAY_BUNDLE (sex,      SEX_TYPE,      SEX_MAX);
SIMPLE_ARRAY_BUNDLE (size,     SIZE_TYPE,     SIZE_MAX_R);
SIMPLE_ARRAY_BUNDLE (race,     RACE_TYPE,     RACE_MAX);
SIMPLE_ARRAY_BUNDLE (liq,      LIQ_TYPE,      LIQ_MAX);
SIMPLE_ARRAY_BUNDLE (attack,   ATTACK_TYPE,   ATTACK_MAX);
SIMPLE_ARRAY_BUNDLE (class,    CLASS_TYPE,    CLASS_MAX);
SIMPLE_ARRAY_BUNDLE (skill,    SKILL_TYPE,    SKILL_MAX);
SIMPLE_ARRAY_BUNDLE (spec,     SPEC_TYPE,     SPEC_MAX);
SIMPLE_ARRAY_BUNDLE (group,    GROUP_TYPE,    GROUP_MAX);
SIMPLE_ARRAY_BUNDLE (wear,     WEAR_TYPE,     WEAR_MAX);
SIMPLE_ARRAY_BUNDLE (recycle,  RECYCLE_TYPE,  RECYCLE_MAX);
SIMPLE_ARRAY_BUNDLE (board,    BOARD_DATA,    BOARD_MAX);
SIMPLE_ARRAY_BUNDLE (master,   TABLE_TYPE,    0);

SIMPLE_HASH_BUNDLE (wiznet,     WIZNET_TYPE,      bit);
SIMPLE_HASH_BUNDLE (weapon,     WEAPON_TYPE,      type);
SIMPLE_HASH_BUNDLE (item,       ITEM_TYPE,        type);
SIMPLE_HASH_BUNDLE (sector,     SECTOR_TYPE,      type);
SIMPLE_HASH_BUNDLE (map_lookup, MAP_LOOKUP_TABLE, index);
SIMPLE_HASH_BUNDLE (map_flags,  MAP_LOOKUP_TABLE, index);
SIMPLE_HASH_BUNDLE (nanny,      NANNY_HANDLER,    state);
SIMPLE_HASH_BUNDLE (furniture,  FURNITURE_BITS,   position);
SIMPLE_HASH_BUNDLE (door,       DOOR_TYPE,        dir);
SIMPLE_HASH_BUNDLE (material,   MATERIAL_TYPE,    type);
SIMPLE_HASH_BUNDLE (dam,        DAM_TYPE,         type);
SIMPLE_HASH_BUNDLE (colour,     COLOUR_TYPE,      code);
SIMPLE_HASH_BUNDLE (colour_setting, COLOUR_SETTING_TYPE, index);
SIMPLE_HASH_BUNDLE (affect_bit, AFFECT_BIT_TYPE,  type);
SIMPLE_HASH_BUNDLE (day,        DAY_TYPE,         type);
SIMPLE_HASH_BUNDLE (month,      MONTH_TYPE,       type);
SIMPLE_HASH_BUNDLE (sky,        SKY_TYPE,         type);
SIMPLE_HASH_BUNDLE (sun,        SUN_TYPE,         type);

SIMPLE_REC_BUNDLE (ban,         BAN_DATA,         RECYCLE_BAN_DATA);
SIMPLE_REC_BUNDLE (area,        AREA_DATA,        RECYCLE_AREA_DATA);
SIMPLE_REC_BUNDLE (room_index,  ROOM_INDEX_DATA,  RECYCLE_ROOM_INDEX_DATA);
SIMPLE_REC_BUNDLE (obj_index,   OBJ_INDEX_DATA,   RECYCLE_OBJ_INDEX_DATA);
SIMPLE_REC_BUNDLE (help,        HELP_DATA,        RECYCLE_HELP_DATA);
SIMPLE_REC_BUNDLE (had,         HELP_AREA,        RECYCLE_HELP_AREA);
SIMPLE_REC_BUNDLE (social,      SOCIAL_TYPE,      RECYCLE_SOCIAL_TYPE);
SIMPLE_REC_BUNDLE (portal_exit, PORTAL_EXIT_TYPE, RECYCLE_PORTAL_EXIT_TYPE);

SPEC_FUN *spec_lookup_function (const char *name)
    { SIMPLE_LOOKUP_PROP (spec_table, function, name, NULL, SPEC_MAX); }

const char *spec_function_name (SPEC_FUN *function) {
    int i;
    for (i = 0; spec_table[i].function != NULL; i++)
        if (function == spec_table[i].function)
            return spec_table[i].name;
    return NULL;
}

const OBJ_MAP *obj_map_get (int item_type)
    { SIMPLE_GET (obj_map_table, item_type, item_type, -1, 0); }

const char *map_lookup_get_string (int index, flag_t value) {
    const MAP_LOOKUP_TABLE *lookup = map_lookup_get (index);
    if (lookup == NULL)
        return NULL;
    if (lookup->flags)
        return flag_get_name (value, lookup->flags);
    else {
        switch (lookup->index) {
            case MAP_LOOKUP_WEAPON_TYPE: return weapon_get_name (value);
            case MAP_LOOKUP_ATTACK_TYPE: return attack_get_name (value);
            case MAP_LOOKUP_SKILL:       return skill_get_name (value);
            case MAP_LOOKUP_LIQUID:      return liq_get_name (value);

            default:
                bugf("map_lookup_get_string: Unhandled type '%s'",
                    lookup->name);
                return NULL;
        }
    }
}

int map_flags_get_string (int index, flag_t value, char *buf, size_t size) {
    const MAP_LOOKUP_TABLE *lookup = map_flags_get (index);
    char *str = NULL;
    if (lookup == NULL)
        return 0;
    if (lookup->flags)
        str = flag_string (lookup->flags, value);
    else {
        switch (lookup->index) {
            default:
                bugf("map_flags_get_string: Unhandled type '%s'",
                    lookup->name);
                return 0;
        }
    }

    if (str == NULL || !strcmp(str, "none"))
        buf[0] = '\0';
    else
        snprintf (buf, size, "%s", str);

    return 1;
}

const OBJ_MAP_VALUE *obj_map_value_get (const OBJ_MAP *map, int index) {
    int i;
    if (map == NULL)
        return NULL;
    for (i = 0; i < OBJ_VALUE_MAX; i++)
        if (map->values[i].index == index)
            return (map->values + i);
    return NULL;
}

const TABLE_TYPE *master_get_first (void)
    { return &(master_table[0]); }
const TABLE_TYPE *master_get_next (const TABLE_TYPE *table)
    { return table[1].name == NULL ? NULL : &(table[1]); }

AREA_DATA *area_get_by_vnum (int vnum) {
    AREA_DATA *a;
    for (a = area_get_first(); a; a = area_get_next (a))
        if (a->vnum == vnum)
            return a;
    return NULL;
}

AREA_DATA *area_get_by_inner_vnum (int vnum) {
    AREA_DATA *a;
    for (a = area_get_first(); a; a = area_get_next (a))
        if (vnum >= a->min_vnum && vnum <= a->max_vnum)
            return a;
    return NULL;
}

flag_t wear_get_type_by_loc (flag_t wear_loc) {
    int i;
    for (i = 0; wear_table[i].name != NULL; i++)
        if (wear_loc == wear_table[i].wear_loc)
            return wear_table[i].type;
    return 0;
}

flag_t wear_get_loc_by_type (flag_t type) {
    int i;
    for (i = 0; wear_table[i].name != NULL; i++)
        if (type == wear_table[i].type)
            return wear_table[i].wear_loc;
    return 0;
}

HELP_AREA *help_area_get_by_help (HELP_DATA * help) {
    HELP_AREA *had;
    HELP_DATA *h;
    for (had = had_first; had; had = had->next)
        for (h = had->first; h; h = h->next_area)
            if (h == help)
                return had;
    return NULL;
}

const DAY_TYPE *day_get_current ()
    { return day_get ((time_info.day + 1) % DAY_MAX); }
const MONTH_TYPE *month_get_current ()
    { return month_get (time_info.month); }
const SKY_TYPE *sky_get_current ()
    { return sky_get (weather_info.sky); }
const SUN_TYPE *sun_get_current ()
    { return sun_get (weather_info.sunlight); }

const SKY_TYPE *sky_get_by_mmhg (int mmhg) {
    const SKY_TYPE *sky;
    int i;
    for (i = 0; i < SKY_MAX; i++) {
        sky = &(sky_table[i]);
        if ((mmhg >= sky->mmhg_min || sky->mmhg_min == -1) &&
            (mmhg <  sky->mmhg_max || sky->mmhg_max == -1))
            return sky;
    }
    return &(sky_table[0]);
}

const SUN_TYPE *sun_get_by_hour (int hour) {
    int i;
    for (i = 0; i < SUN_MAX; i++)
        if (hour >= sun_table[i].hour_start && hour < sun_table[i].hour_end)
            return &(sun_table[i]);
    return &(sun_table[0]);
}

/* Lookup a skill by slot number. Used for object loading. */
int slot_lookup (int slot) {
    extern bool fBootDb;
    int sn;

    if (slot <= 0)
        return -1;
    for (sn = 0; sn < SKILL_MAX; sn++)
        if (slot == skill_table[sn].slot)
            return sn;

    if (fBootDb) {
        bug ("slot_lookup: bad slot %d.", slot);
        abort ();
    }
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

const char *condition_name_by_percent (int percent) {
#ifndef VANILLA
         if (percent >= 100) return "is in excellent condition";
    else if (percent >=  90) return "has a few scratches";
    else if (percent >=  80) return "has a few bruises";
    else if (percent >=  70) return "has some small wounds and bruises";
    else if (percent >=  60) return "has some large wounds";
    else if (percent >=  50) return "has quite a large few wounds";
    else if (percent >=  40) return "has some big nasty wounds and scratches";
    else if (percent >=  30) return "looks seriously wounded";
    else if (percent >=  20) return "looks pretty hurt";
    else if (percent >=  10) return "is in awful condition";
    else if (percent >    0) return "is in critical condition";
    else if (percent >  -10) return "is stunned on the floor";
    else if (percent >  -20) return "is incapacitated and bleeding to death";
    else                     return "is mortally wounded";
#else
         if (percent >= 100) return "is in excellent condition";
    else if (percent >=  90) return "has a few scratches";
    else if (percent >=  75) return "has some small wounds and bruises";
    else if (percent >=  50) return "has quite a few wounds";
    else if (percent >=  30) return "has some big nasty wounds and scratches";
    else if (percent >=  15) return "looks pretty hurt";
    else if (percent >=   0) return "is in awful condition";
    else                     return "is bleeding to death";
#endif
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
const char *wear_bit_name (flag_t flags)
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
