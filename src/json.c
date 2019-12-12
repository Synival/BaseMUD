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
 *    ROM 2.4 is copyright 1993-1998 Russ Taylor                           *
 *    ROM has been brought to you by the ROM consortium                    *
 *        Russ Taylor (rtaylor@hypercube.org)                              *
 *        Gabrielle Taylor (gtaylor@hypercube.org)                         *
 *        Brian Moore (zump@rom.org)                                       *
 *    By using this code, you have agreed to follow the terms of the       *
 *    ROM license, in the file Rom24/doc/rom.license                       *
 ***************************************************************************/

#include <sys/types.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"
#include "memory.h"

#include "json.h"

static JSON_T *json_top = NULL;

JSON_T *json_root (void) {
    if (json_top == NULL)
        json_top = json_new_object ("world", JSON_OBJ_ANY);
    return json_top;
}

JSON_T *json_root_area (const char *name) {
    JSON_T *root, *get;
    if (name == NULL)
        return NULL;

    root = json_root();
    get  = json_get (root, name);
    if (get == NULL) {
        get = json_new_array (name, NULL);
        json_attach_under (get, root);
    }
    return get;
}

void json_root_area_attach (const char *name, JSON_T *json) {
    JSON_T *area = json_root_area (name);
    json_attach_under (json, area);
}

JSON_T *json_get (const JSON_T *json, const char *name) {
    JSON_T *j;
    if (json == NULL || name == NULL || json->type != JSON_OBJECT)
        return NULL;
    for (j = json->first_child; j != NULL; j = j->next)
        if (!strcmp (j->name, name))
            return j;
    return NULL;
}

JSON_T *json_new (const char *name, int type, void *value, size_t value_size) {
    JSON_T *new = calloc (1, sizeof(JSON_T));
    new->type = type;
    if (name != NULL)
        new->name = strdup (name);
    if (value != NULL) {
        new->value = value;
        new->value_size = value_size;
    }
    return new;
}

void json_attach_file (JSON_T *json, const char *filename, int line, int col) {
    if (json->filename)
        free (json->filename);
    json->filename = strdup (filename);
    json->line = line;
    json->col  = col;
}

void json_attach_after (JSON_T *json, JSON_T *after, JSON_T *parent) {
    if (json == NULL || parent == NULL)
        return;
    json_detach (json);

    json->parent = parent;
    parent->child_count++;

    LIST2_INSERT_AFTER (json, after, prev, next,
        parent->first_child, parent->last_child);
}

void json_attach_under (JSON_T *json, JSON_T *ref) {
    if (json == NULL || ref == NULL)
        return;
    json_detach (json);

    json->parent = ref;
    ref->child_count++;

    LIST2_BACK (json, prev, next,
        ref->first_child, ref->last_child);
}

void json_detach (JSON_T *json) {
    if (json->parent != NULL) {
        json->parent->child_count--;
        if (json->parent->first_child == json)
            json->parent->first_child = json->next;
        if (json->parent->last_child == json)
            json->parent->last_child = json->prev;
    }

    if (json->prev)
        json->prev->next = json->next;
    if (json->next)
        json->next->prev = json->prev;

    json->parent = NULL;
    json->prev   = NULL;
    json->next   = NULL;
    if (json_top == json)
        json_top = NULL;
}

void json_free (JSON_T *json) {
    if (json == NULL)
        return;
    while (json->first_child)
        json_free (json->first_child);
    json_detach (json);

    if (json->name)     free (json->name);
    if (json->value)    free (json->value);
    if (json->filename) free (json->filename);

    free (json);
}

JSON_PROP_FUN (string, const char *);
JSON_T *json_new_string (const char *name, const char *str) {
    return (str == NULL)
        ? json_new_null (name)
        : json_new (name, JSON_STRING, strdup (str), strlen (str) + 1);
}

JSON_PROP_FUN (number, json_num);
JSON_T *json_new_number (const char *name, json_num value) {
    JSON_SIMPLE (JSON_NUMBER, json_num);
    return new;
}

JSON_PROP_FUN (integer, json_int);
JSON_T *json_new_integer (const char *name, json_int value) {
    JSON_SIMPLE (JSON_INTEGER , json_int);
    return new;
}

JSON_PROP_FUN (boolean, bool);
JSON_T *json_new_boolean (const char *name, bool value) {
    JSON_SIMPLE (JSON_BOOLEAN, bool);
    return new;
}

JSON_PROP_FUN_0 (null);
JSON_T *json_new_null (const char *name)
    { return json_new (name, JSON_NULL, NULL, 0); }

JSON_PROP_FUN (object, int);
JSON_T *json_new_object (const char *name, int value) {
    JSON_SIMPLE (JSON_OBJECT, int);
    return new;
}

JSON_T *json_prop_array (JSON_T *parent, const char *name) {
    JSON_T *new = json_new_array (name, NULL);
    json_attach_under (new, parent);
    return new;
}
JSON_T *json_new_array (const char *name, JSON_T *first, ...) {
    va_list ap;
    JSON_T *add = first;
    JSON_T *new = json_new (name, JSON_ARRAY, NULL, 0);

    va_start (ap, first);
    while (add != NULL) {
        json_attach_under (add, new);
        add = va_arg(ap, JSON_T *);
    }
    return new;
}

JSON_PROP_FUN (dice, const DICE_T *);
JSON_T *json_new_dice (const char *name, const DICE_T *dice) {
    char buf[256];

    if (dice == NULL)
        return json_new_null (name);
    snprintf (buf, sizeof (buf), "%dd%d%+d",
        dice->number, dice->size, dice->bonus);
    return json_new (name, JSON_DICE, strdup (buf), strlen (buf) + 1);
}

JSON_T *json_wrap_obj (JSON_T *json, char *inner_name) {
    char *outer_name;
    JSON_T *new;

    if (json == NULL)
        json = json_new_null (NULL);
    outer_name = json->name;

    new = json_new_object (outer_name, JSON_OBJ_ANY);
    if (json->name)
        free (json->name);
    json->name = strdup (inner_name);
    json_attach_under (json, new);

    return new;
}

bool json_expand_newlines (char *buf_in, size_t len) {
    char *buf_out, *ch_in, *ch_out;

    /* Don't do anything if there aren't any newlines at all. */
    if (!strchr (buf_in, '\n'))
        return FALSE;

    buf_out = malloc (sizeof (char) * len);
    for (ch_in = buf_in, ch_out = buf_out; 1; ch_in++, ch_out++) {
        if ((ch_in - buf_in) >= (len - 1))
            break;
        if ((ch_out - buf_out) >= (len - 1))
            break;
        if (*ch_in == '\0')
            break;

        *ch_out = *ch_in;
        if (*ch_in == '\n' && (ch_out - buf_out) < (len - 2)) {
            ch_out++;
            *ch_out = '\r';
        }
    }
    *ch_out = '\0';

    for (ch_in = buf_in, ch_out = buf_out; 1; ch_in++, ch_out++) {
        *ch_in = *ch_out;
        if (*ch_in == '\0')
            break;
    }

    free (buf_out);
    return TRUE;
}

char *json_value_as_string (const JSON_T *json, char *buf, size_t size) {
    if (size > 0 && buf != NULL)
        buf[0] = '\0';
    if (json == NULL)
        return NULL;

    switch (json->type) {
        case JSON_STRING:
        case JSON_DICE:
            snprintf (buf, size, "%s", (const char *) json->value);
            json_expand_newlines (buf, size);
            return buf;
        case JSON_NULL:
            return NULL;
        default:
            json_logf (json, "json_value_as_string(): Unhandled type '%d'.\n",
                json->type);
            return NULL;
    }
}

json_int json_value_as_int (const JSON_T *json) {
    if (json == NULL)
        return 0;

    switch (json->type) {
        case JSON_NUMBER: {
            json_num *value = json->value;
            return *value;
        }

        case JSON_INTEGER: {
            json_int *value = json->value;
            return *value;
        }

        case JSON_BOOLEAN: {
            bool *value = json->value;
            return (*value == TRUE) ? 1 : 0;
        }

        case JSON_NULL:
            return 0;

        default:
            json_logf (json, "json_value_as_int(): Unhandled type '%d'.\n",
                json->type);
            return 0;
    }
}

bool json_value_as_bool (const JSON_T *json) {
    if (json == NULL)
        return 0;

    switch (json->type) {
        case JSON_NUMBER: {
            json_num *value = json->value;
            return *value ? TRUE : FALSE;
        }

        case JSON_INTEGER: {
            json_int *value = json->value;
            return *value ? TRUE : FALSE;
        }

        case JSON_BOOLEAN: {
            bool *value = json->value;
            return *value;
        }

        case JSON_NULL:
            return FALSE;

        default:
            json_logf (json, "json_value_as_bool(): Unhandled type '%d'.\n",
                json->type);
            return 0;
    }
}

DICE_T json_value_as_dice (const JSON_T *json) {
    DICE_T rval;

    rval.number = 0;
    rval.size   = 0;
    rval.bonus  = 0;

    if (json == NULL)
        return rval;

    switch (json->type) {
        case JSON_STRING:
        case JSON_DICE: {
            char buf[256], *num = NULL, *size = NULL, *bonus = NULL;
            int bonus_sign = 1;
            snprintf (buf, sizeof (buf), "%s", (const char *) json->value);

            num = buf;
            if ((size = strchr (num, 'd')) != NULL) {
                *size = '\0';
                size++;

                if ((bonus = strchr (size, '+')) != NULL) {
                    *bonus = '\0';
                    bonus++;
                }
                else if ((bonus = strchr (size, '-')) != NULL) {
                    *bonus = '\0';
                    bonus++;
                    bonus_sign = -1;
                }
            }

            if (num)   rval.number = atoi (num);
            if (size)  rval.size   = atoi (size);
            if (bonus) rval.bonus  = atoi (bonus) * bonus_sign;
            return rval;
        }

        case JSON_NUMBER: {
            json_num *value = json->value;
            rval.bonus = *value;
            return rval;
        }

        case JSON_INTEGER: {
            json_int *value = json->value;
            rval.bonus = *value;
            return rval;
        }

        case JSON_NULL:
            return rval;

        default:
            json_logf (json, "json_value_as_dice(): Unhandled type '%d'.\n",
                json->type);
            return rval;
    }
}

void json_logf (const JSON_T *json, const char *format, ...) {
    char buf[2 * MSL];
    va_list args;
    va_start (args, format);
    vsnprintf (buf, sizeof(buf), format, args);
    va_end (args);
    log_f ("%s, line %d, col %d: %s", json->filename,
        json->line, json->col,buf);
}
