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

#ifndef __ROM_LOOKUP_H
#define __ROM_LOOKUP_H

#include "merc.h"

#define SIMPLE_LOOKUP_PROP(table, get, ref, empty_val, max)           \
    int i;                                                            \
    if ((ref) == NULL || (ref)[0] == '\0')                            \
        return empty_val;                                             \
    for (i = 0; (max <= 0 || i < max) && (table)[i].ref != NULL; i++) \
        if ((table)[i].ref != NULL && LOWER (ref[0]) == LOWER ((table)[i].ref[0]) \
            && !str_prefix (ref, (table)[i].ref))                     \
            return (table)[i].get;                                    \
    return empty_val

#define SIMPLE_LOOKUP_PROP_EXACT(table, get, ref, empty_val, max)     \
    int i;                                                            \
    if ((ref) == NULL || (ref)[0] == '\0')                            \
        return empty_val;                                             \
    for (i = 0; (max <= 0 || i < max) && (table)[i].ref != NULL; i++) \
        if ((table)[i].ref != NULL && !str_cmp (ref, (table)[i].ref)) \
            return (table)[i].get;                                    \
    return empty_val

#define SIMPLE_LOOKUP(table, ref, empty_val, max)                     \
    int i;                                                            \
    if ((ref) == NULL || (ref)[0] == '\0')                            \
        return empty_val;                                             \
    for (i = 0; (max <= 0 || i < max) && (table)[i].ref != NULL; i++) \
        if ((table)[i].ref != NULL && LOWER (ref[0]) == LOWER ((table)[i].ref[0]) \
            && !str_prefix (ref, (table)[i].ref))                     \
            return i;                                                 \
    return empty_val

#define SIMPLE_LOOKUP_EXACT(table, ref, empty_val, max)               \
    int i;                                                            \
    if ((ref) == NULL || (ref)[0] == '\0')                            \
        return empty_val;                                             \
    for (i = 0; (max <= 0 || i < max) && (table)[i].ref != NULL; i++) \
        if ((table)[i].ref != NULL && !str_cmp (ref, (table)[i].ref)) \
            return i;                                                 \
    return empty_val

#define SIMPLE_GET_BY_NAME(table, ref, max)                           \
    int i;                                                            \
    if ((ref) == NULL || (ref)[0] == '\0')                            \
        return NULL;                                                  \
    for (i = 0; (max <= 0 || i < max) && (table)[i].ref != NULL; i++) \
        if ((table)[i].ref != NULL && LOWER (ref[0]) == LOWER ((table)[i].ref[0]) \
            && !str_prefix (ref, (table)[i].ref))                     \
            return ((table) + i);                                     \
    return NULL

#define SIMPLE_GET_BY_NAME_EXACT(table, ref, max)                     \
    int i;                                                            \
    if ((ref) == NULL || (ref)[0] == '\0')                            \
        return NULL;                                                  \
    for (i = 0; (max <= 0 || i < max) && (table)[i].ref != NULL; i++) \
        if ((table)[i].ref != NULL && !str_cmp (ref, (table)[i].ref)) \
            return ((table) + i);                                     \
    return NULL

#define SIMPLE_GET(table, ref, check_prop, check_val, max)            \
    int i;                                                            \
    for (i = 0; (max <= 0 || i < max) && (((table)[i]).check_prop) != (check_val); i++) \
        if (ref == ((table)[i].ref))                                  \
            return ((table) + i);                                     \
    return NULL

#define SIMPLE_GET_NAME(vtype, val, name) \
    const vtype * looked_up = val;        \
    return (looked_up != NULL) ? looked_up->name : "unknown"

#define SIMPLE_ARRAY_BUNDLE(btype, vtype, max)                      \
    int btype ## _lookup (const char *name)                         \
        { SIMPLE_LOOKUP (btype ## _table, name, -1, max); }         \
    int btype ## _lookup_exact (const char *name)                   \
        { SIMPLE_LOOKUP_EXACT (btype ## _table, name, -1, max); }   \
    const vtype * btype ## _get_by_name (const char *name)          \
        { SIMPLE_GET_BY_NAME (btype ## _table, name, max); }        \
    const vtype * btype ## _get_by_name_exact (const char *name)    \
        { SIMPLE_GET_BY_NAME_EXACT (btype ## _table, name, max); }  \
    const vtype * btype ## _get (int type)                          \
        { return (type < 0 || type >= max)                          \
            ? NULL : (btype ## _table + type); }                    \
    const char * btype ## _get_name (int type)                      \
        { return (type < 0 || type >= max)                          \
            ? NULL : (btype ## _get)(type)->name; }

#define SIMPLE_HASH_BUNDLE(btype, vtype, ref)                       \
    int btype ## _lookup (const char *name)                         \
        { SIMPLE_LOOKUP_PROP (btype ## _table, ref, name, -1, 0); } \
    int btype ## _lookup_exact (const char *name)                   \
        { SIMPLE_LOOKUP_PROP_EXACT (btype ## _table, ref, name, -1, 0); } \
    const vtype * btype ## _get_by_name (const char *name)          \
        { SIMPLE_GET_BY_NAME (btype ## _table, name, 0); }          \
    const vtype * btype ## _get_by_name_exact (const char *name)    \
        { SIMPLE_GET_BY_NAME_EXACT (btype ## _table, name, 0); }    \
    const vtype * btype ## _get (int ref)                           \
        { SIMPLE_GET (btype ## _table, ref, name, NULL, 0); }       \
    const char * btype ## _get_name (int ref)                       \
        { SIMPLE_GET_NAME (vtype, btype ## _get (ref), name); }

#define SIMPLE_REC_BUNDLE(btype, vtype, rtype)                             \
    vtype * btype ## _get_by_name (const char *name) {                     \
        const OBJ_RECYCLE_DATA *orec = btype ## _get_rec_by_name (name);   \
        return (orec == NULL) ? NULL : (vtype *) orec->obj;                \
    }                                                                      \
    const OBJ_RECYCLE_DATA *btype ## _get_rec_by_name (const char *name) { \
        const RECYCLE_TYPE *rec;                                           \
        OBJ_RECYCLE_DATA *orec;                                            \
        rec = recycle_get (rtype);                                         \
        if (rec->obj_name_off < 0)                                         \
            return NULL;                                                   \
        for (orec = rec->list_front; orec != NULL; orec = orec->next)      \
            if (!str_cmp (name, DEREF_OFFSET (                             \
                    char *, orec->obj, rec->obj_name_off)))                \
                return orec;                                               \
        return NULL;                                                       \
    }

#define DEC_SIMPLE_ARRAY_BUNDLE(btype, vtype)                \
    int           btype ## _lookup       (const char *name); \
    int           btype ## _lookup_exact (const char *name); \
    const vtype * btype ## _get_by_name  (const char *name); \
    const vtype * btype ## _get_by_name_exact (const char *name); \
    const vtype * btype ## _get          (int type);         \
    const char *  btype ## _get_name     (int type)

#define DEC_SIMPLE_HASH_BUNDLE(btype, vtype)                 \
    int           btype ## _lookup       (const char *name); \
    int           btype ## _lookup_exact (const char *name); \
    const vtype * btype ## _get_by_name  (const char *name); \
    const vtype * btype ## _get_by_name_exact (const char *name); \
    const vtype * btype ## _get          (int type);         \
    const char *  btype ## _get_name     (int type)

#define DEC_SIMPLE_REC_BUNDLE(btype, vtype)                                \
    vtype *                  btype ## _get_by_name     (const char *name); \
    const OBJ_RECYCLE_DATA * btype ## _get_rec_by_name (const char *name)

/* Function prototypes for flag management. */
int lookup_backup (int (*func) (const char *str), char *str, char *errf,
    int backup);
flag_t flag_lookup (const char *name, const FLAG_TYPE *flag_table);
flag_t flag_lookup_exact (const char *name, const FLAG_TYPE *flag_table);
const FLAG_TYPE *flag_get_by_name (const char *name, const FLAG_TYPE *flag_table);
const FLAG_TYPE *flag_get_by_name_exact (const char *name, const FLAG_TYPE *flag_table);
const FLAG_TYPE *flag_get (flag_t bit, const FLAG_TYPE *flag_table);
const char *flag_get_name (flag_t bit, const FLAG_TYPE *flag_table);
bool is_table_flagged (const void *table, flag_t t_flags, flag_t f_flags);
bool is_flag (const void *table);
bool is_type (const void *table);
bool is_special (const void *table);
flag_t flag_value  (const FLAG_TYPE *flag_table, char *argument);
char *flag_string (const FLAG_TYPE *flag_table, flag_t bits);

/* Lookup bundles. */
DEC_SIMPLE_ARRAY_BUNDLE (clan,       CLAN_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (position,   POSITION_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (sex,        SEX_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (size,       SIZE_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (race,       RACE_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (liq,        LIQ_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (class,      CLASS_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (dam,        DAM_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (attack,     ATTACK_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (skill,      SKILL_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (spec,       SPEC_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (group,      GROUP_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (wear,       WEAR_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (recycle,    RECYCLE_TYPE);
DEC_SIMPLE_ARRAY_BUNDLE (board,      BOARD_DATA);
DEC_SIMPLE_ARRAY_BUNDLE (master,     TABLE_TYPE);

DEC_SIMPLE_HASH_BUNDLE (wiznet,     WIZNET_TYPE);
DEC_SIMPLE_HASH_BUNDLE (weapon,     WEAPON_TYPE);
DEC_SIMPLE_HASH_BUNDLE (item,       ITEM_TYPE);
DEC_SIMPLE_HASH_BUNDLE (sector,     SECTOR_TYPE);
DEC_SIMPLE_HASH_BUNDLE (map_lookup, MAP_LOOKUP_TABLE);
DEC_SIMPLE_HASH_BUNDLE (map_flags,  MAP_LOOKUP_TABLE);
DEC_SIMPLE_HASH_BUNDLE (nanny,      NANNY_HANDLER);
DEC_SIMPLE_HASH_BUNDLE (furniture,  FURNITURE_BITS);
DEC_SIMPLE_HASH_BUNDLE (door,       DOOR_TYPE);
DEC_SIMPLE_HASH_BUNDLE (material,   MATERIAL_TYPE);
DEC_SIMPLE_HASH_BUNDLE (colour,     COLOUR_TYPE);
DEC_SIMPLE_HASH_BUNDLE (colour_setting, COLOUR_SETTING_TYPE);

DEC_SIMPLE_REC_BUNDLE (ban,         BAN_DATA);
DEC_SIMPLE_REC_BUNDLE (area,        AREA_DATA);
DEC_SIMPLE_REC_BUNDLE (room_index,  ROOM_INDEX_DATA);
DEC_SIMPLE_REC_BUNDLE (obj_index,   OBJ_INDEX_DATA);
DEC_SIMPLE_REC_BUNDLE (help,        HELP_DATA);
DEC_SIMPLE_REC_BUNDLE (had,         HELP_AREA);
DEC_SIMPLE_REC_BUNDLE (social,      SOCIAL_TYPE);
DEC_SIMPLE_REC_BUNDLE (portal_exit, PORTAL_EXIT_TYPE);

/* Special lookup functions. */
SPEC_FUN* spec_lookup_function (const char *name);
const char *spec_function_name (SPEC_FUN *function);

const OBJ_MAP        *obj_map_get        (int item_type);
const OBJ_MAP_VALUE  *obj_map_value_get  (const OBJ_MAP *map, int index);

const char *map_lookup_get_string (int index, flag_t value);
int         map_flags_get_string  (int index, flag_t value, char *buf, size_t size);
const TABLE_TYPE *master_get_first (void);
const TABLE_TYPE *master_get_next (const TABLE_TYPE *table);
AREA_DATA *area_get_by_vnum (int vnum);
AREA_DATA *area_get_by_inner_vnum (int vnum);
flag_t wear_get_type_by_loc (flag_t wear_loc);
flag_t wear_get_loc_by_type (flag_t type);
HELP_AREA *help_area_get_by_help (HELP_DATA * help);

/* Bit functions from handler.c */
char *affect_apply_name (flag_t type);
char *room_bit_name (flag_t flags);
char *affect_bit_name (flag_t flags);
char *extra_bit_name (flag_t flags);
char *mob_bit_name (flag_t flags);
char *plr_bit_name (flag_t flags);
char *comm_bit_name (flag_t flags);
char *res_bit_name (flag_t flags);
char *wear_bit_name (flag_t flags);
char *form_bit_name (flag_t flags);
char *part_bit_name (flag_t flags);
char *weapon_bit_name (flag_t flags);
char *cont_bit_name (flag_t flags);
char *off_bit_name (flag_t flags);

#endif
