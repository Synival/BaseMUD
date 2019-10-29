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

JSON_T *json_tblw_flag (const void *obj) {
    JSON_TBLW_START (FLAG_TYPE, flag, flag->name == NULL);
    json_prop_string  (new, "name",     flag->name);
    json_prop_integer (new, "bit",      flag->bit);
    json_prop_boolean (new, "settable", flag->settable);
    return new;
}

JSON_T *json_tblw_clan (const void *obj) {
    JSON_TBLW_START (CLAN_TYPE, clan, clan->name == NULL);
    json_prop_string  (new, "name",        JSTR (clan->name));
    json_prop_string  (new, "who_name",    JSTR (clan->who_name));
    json_prop_integer (new, "hall",        clan->hall);
    json_prop_boolean (new, "independent", clan->independent);
    return new;
}

JSON_T *json_tblw_position (const void *obj) {
    JSON_TBLW_START (POSITION_TYPE, pos, pos->name == NULL);
    json_prop_integer (new, "position",   pos->pos);
    json_prop_string  (new, "name",       JSTR (pos->name));
    json_prop_string  (new, "long_name",  JSTR (pos->long_name));
    return new;
}

JSON_T *json_tblw_sex (const void *obj) {
    JSON_TBLW_START (SEX_TYPE, sex, sex->name == NULL);
    json_prop_integer (new, "sex",  sex->sex);
    json_prop_string  (new, "name", JSTR (sex->name));
    return new;
}

JSON_T *json_tblw_size (const void *obj) {
    JSON_TBLW_START (SIZE_TYPE, size, size->name == NULL);
    json_prop_integer (new, "size", size->size);
    json_prop_string  (new, "name", JSTR (size->name));
    return new;
}
