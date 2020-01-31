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

#ifndef __ROM_JSON_H
#define __ROM_JSON_H

#include "merc.h"

#define JSON_DIR        "json/"
#define JSON_CONFIG_DIR (JSON_DIR "config/")
#define JSON_HELP_DIR   (JSON_DIR "help/")
#define JSON_AREAS_DIR  (JSON_DIR "areas/")

/* all valid types of json_t */
#define JSON_STRING     0
#define JSON_NUMBER     1
#define JSON_INTEGER    2
#define JSON_BOOLEAN    3
#define JSON_NULL       4
#define JSON_OBJECT     5
#define JSON_ARRAY      6
#define JSON_DICE       7
#define JSON_MAX        8

/* all valid JSON_OBJECT sub-types */
#define JSON_OBJ_ANY            0
#define JSON_OBJ_ROOM           1
#define JSON_OBJ_SHOP           2
#define JSON_OBJ_MOBILE         3
#define JSON_OBJ_OBJECT         4
#define JSON_OBJ_RESET          5
#define JSON_OBJ_AREA           6
#define JSON_OBJ_EXTRA_DESCR    7
#define JSON_OBJ_EXIT           8
#define JSON_OBJ_AFFECT         9
#define JSON_OBJ_SOCIAL         10
#define JSON_OBJ_TABLE          11
#define JSON_OBJ_FLAG           12
#define JSON_OBJ_PORTAL_EXIT    13
#define JSON_OBJ_PORTAL         14
#define JSON_OBJ_HELP_AREA      15
#define JSON_OBJ_HELP           16
#define JSON_OBJ_MAX            17

/* useful macros. */
#define JSON_PROP_FUN_0(vname) \
    JSON_T *json_prop_ ## vname (JSON_T *parent, const char *name) { \
        JSON_T *new = json_new_ ## vname (name); \
        json_attach_under (new, parent); \
        return new; \
    }

#define JSON_PROP_FUN(vname, vptype) \
    JSON_T *json_prop_ ## vname (JSON_T *parent, const char *name, \
        vptype arg) \
    { \
        JSON_T *new = json_new_ ## vname (name, arg); \
        json_attach_under (new, parent); \
        return new; \
    }

#define JSON_PROP_FUN_OF(vname, fname, vptype) \
    JSON_T *json_prop_ ## vname (JSON_T *parent, const char *name, \
        vptype arg) \
    { \
        JSON_T *new = fname (name, arg); \
        json_attach_under (new, parent); \
        return new; \
    }

#define JSON_SIMPLE(jtype, ctype) \
    JSON_T *new; \
    ctype *ptr = calloc (1, sizeof (ctype)); \
    *ptr = value; \
    new = json_new (name, jtype, ptr, sizeof (ctype))

#define JSON_GET_INT(json, prop) \
    (json_value_as_int (json_get ((json), (prop))))
#define JSON_GET_STR(json, prop, buf) \
    (json_value_as_string (json_get ((json), (prop)), (buf), sizeof (buf)))
#define JSON_GET_BOOL(json, prop) \
    (json_value_as_bool (json_get ((json), (prop))))

#define JBITS(val) \
    json_not_none(val)
#define JBITSF(array, bits) \
    JBITS (flags_to_string ((array), (bits)))
#define JBITSXF(array, bits) \
    JBITS (ext_flags_to_string ((array), (bits)))
#define JSTR(val) \
    json_not_blank(val)

/* data structures */
struct json_t {
    int type;
    char *name;
    void *value;
    size_t value_size;
    struct json_t *parent, *prev, *next, *first_child, *last_child;
    int child_count;
    char *filename;
    int line, col;
};

/* function declarations */
JSON_T *json_root (void);
JSON_T *json_root_area (const char *name);
void    json_root_area_attach (const char *name, JSON_T *json);
JSON_T *json_get (const JSON_T *json, const char *name);
JSON_T *json_new (const char *name, int type, void *value, size_t value_size);
void    json_attach_after (JSON_T *json, JSON_T *after, JSON_T *parent);
void    json_attach_under (JSON_T *json, JSON_T *ref);
void    json_detach (JSON_T *json);
void    json_free (JSON_T *json);
JSON_T *json_wrap_obj (JSON_T *json, char *inner_name);
void    json_logf (const JSON_T *json, const char *format, ...);

/* node creation. */
JSON_T *json_new_string (const char *name, const char *value);
JSON_T *json_new_number (const char *name, json_num value);
JSON_T *json_new_integer (const char *name, json_int value);
JSON_T *json_new_boolean (const char *name, bool value);
JSON_T *json_new_null (const char *name);
JSON_T *json_new_object (const char *name, int value);
JSON_T *json_new_array (const char *name, JSON_T *first, ...);
JSON_T *json_new_dice (const char *name, const DICE_T *dice);

/* shorthand-functions for node creation as properties. */
JSON_T *json_prop_string (JSON_T *parent, const char *name, const char *value);
JSON_T *json_prop_number (JSON_T *parent, const char *name, json_num value);
JSON_T *json_prop_integer (JSON_T *parent, const char *name, json_int value);
JSON_T *json_prop_boolean (JSON_T *parent, const char *name, bool value);
JSON_T *json_prop_null (JSON_T *parent, const char *name);
JSON_T *json_prop_object (JSON_T *parent, const char *name, int value);
JSON_T *json_prop_array (JSON_T *parent, const char *name);
JSON_T *json_prop_dice (JSON_T *parent, const char *name,
    const DICE_T *dice);
JSON_T *json_prop_string_without_last_newline (JSON_T *json, const char *prop,
    const char *obj_name, const char *value);

/* data retrieval. */
bool     json_expand_newlines (char *buf_in, size_t len);
char    *json_value_as_string (const JSON_T *json, char *buf, size_t size);
json_int json_value_as_int (const JSON_T *json);
bool     json_value_as_bool (const JSON_T *json);
struct dice_type json_value_as_dice (const JSON_T *json);

/* utility functions. */
const char *json_not_none (const char *value);
const char *json_not_blank (const char *value);
char *json_string_without_last_newline (const char *name, const char *prop,
    const char *value);

#endif
