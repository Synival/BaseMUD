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

#include "json_tblr.h"

#include "json.h"
#include "json_import.h"
#include "lookup.h"
#include "memory.h"
#include "tables.h"

#include <string.h>

#define JSON_TBLR_START(vtype, var, max, null_check) \
    vtype *var; \
    \
    int index = 0; \
    for (index = 0; index < max; index++) { \
        var = &(var ## _table[index]); \
        if (null_check) \
            break; \
    } \
    if (index == max) { \
        json_logf (json, "Too many '%s' objects.\n", obj_name); \
        return NULL; \
    } \
    var = &(var ## _table[index]);

DEFINE_JSON_READ_FUN (json_tblr_pc_race) {
    char buf[MAX_STRING_LENGTH];
    JSON_T *array, *sub;
    int i;

    JSON_TBLR_START (PC_RACE_T, pc_race, PC_RACE_MAX, pc_race->name == NULL);

    if (!json_import_expect ("pc_race", json,
            "name", "who_name", "creation_points",
            "base_stats", "max_stats", "size",
            "*class_exp", "*skills", "*bonus_max_stat", NULL))
        return NULL;

    READ_PROP_STRP (pc_race->name,     "name");
    READ_PROP_STR  (pc_race->who_name, "who_name");
    READ_PROP_INT  (pc_race->creation_points, "creation_points");

    for (i = 0; class_get (i) != NULL; i++)
        pc_race->class_mult[i] = 100;
    if ((array = json_get (json, "class_exp")) != NULL) {
        for (sub = array->first_child; sub != NULL; sub = sub->next) {
            int num;
            if ((num = class_lookup_exact (sub->name)) < 0) {
                json_logf (json, "Unknown class '%s'", sub->name);
                continue;
            }
            pc_race->class_mult[num] = json_value_as_int (sub);
        }
    }

    if ((array = json_get (json, "skills")) != NULL) {
        int num = 0;
        for (sub = array->first_child; sub != NULL; sub = sub->next)
            pc_race->skills[num++] = str_dup (
                json_value_as_string (sub, buf, sizeof (buf)));
    }

    array = json_get (json, "base_stats");
    for (sub = array->first_child; sub != NULL; sub = sub->next) {
        int stat;
        if ((stat = type_lookup_exact (stat_types, sub->name)) < 0) {
            json_logf (json, "Unknown stat '%s'", sub->name);
            continue;
        }
        pc_race->stats[stat] = json_value_as_int (sub);
    }

    array = json_get (json, "max_stats");
    for (sub = array->first_child; sub != NULL; sub = sub->next) {
        int stat;
        if ((stat = type_lookup_exact (stat_types, sub->name)) < 0) {
            json_logf (json, "Unknown stat '%s'", sub->name);
            continue;
        }
        pc_race->max_stats[stat] = json_value_as_int (sub);
    }

    READ_PROP_INT (pc_race->bonus_max, "bonus_max_stat");

    READ_PROP_STR (buf, "size");
    pc_race->size = lookup_func_backup (size_lookup_exact, buf,
        "Unknown size '%s'", SIZE_MEDIUM);

    return pc_race;
}

DEFINE_JSON_READ_FUN (json_tblr_race) {
    char buf[MAX_STRING_LENGTH];
    JSON_TBLR_START (RACE_T, race, RACE_MAX, race->name == NULL);

    if (!json_import_expect ("race", json,
            "name",
            "*mob_flags", "*affect_flags", "*offense_flags", "*immune_flags",
            "*res_flags", "*vuln_flags",   "*form",          "*parts",
            NULL))
        return NULL;

    READ_PROP_STRP (race->name, "name");
    READ_PROP_EXT_FLAGS (race->ext_mob, "mob_flags", mob_flags);
    READ_PROP_FLAGS (race->aff,   "affect_flags",  affect_flags);
    READ_PROP_FLAGS (race->off,   "offense_flags", off_flags);
    READ_PROP_FLAGS (race->imm,   "immune_flags",  res_flags);
    READ_PROP_FLAGS (race->res,   "res_flags",     res_flags);
    READ_PROP_FLAGS (race->vuln,  "vuln_flags",    res_flags);
    READ_PROP_FLAGS (race->form,  "form",          form_flags);
    READ_PROP_FLAGS (race->parts, "parts",         part_flags);

    return race;
}

DEFINE_JSON_READ_FUN (json_tblr_song) {
    char buf[256];
    JSON_T *array, *sub;

    JSON_TBLR_START (SONG_T, song, MAX_SONGS, song->name == NULL);

    if (!json_import_expect ("song", json,
            "name", "group", "lyrics", NULL))
        return NULL;

    READ_PROP_STRP (song->name,  "name");
    READ_PROP_STRP (song->group, "group");

    if ((array = json_get (json, "lyrics")) != NULL) {
        for (sub = array->first_child; sub != NULL; sub = sub->next) {
            if (song->lines == MAX_SONG_LINES)
                break;
            json_value_as_string (sub, buf, sizeof (buf));
            song->lyrics[song->lines] = str_dup (buf);
            song->lines++;
        }
    }

    return song;
}
