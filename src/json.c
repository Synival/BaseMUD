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

#include <sys/stat.h>
#include <sys/types.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"

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

JSON_T *json_get (JSON_T *json, const char *name) {
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
    if (json->name)
        free (json->name);
    if (json->value)
        free (json->value);
    free (json);
}

JSON_PROP_FUNC (string, const char *);
JSON_T *json_new_string (const char *name, const char *str) {
    return (str == NULL)
        ? json_new_null (name)
        : json_new (name, JSON_STRING, strdup (str), strlen (str) + 1);
}

JSON_PROP_FUNC (number, json_num);
JSON_T *json_new_number (const char *name, json_num value) {
    JSON_SIMPLE (JSON_NUMBER, json_num);
    return new;
}

JSON_PROP_FUNC (integer, json_int);
JSON_T *json_new_integer (const char *name, json_int value) {
    JSON_SIMPLE (JSON_INTEGER , json_int);
    return new;
}

JSON_PROP_FUNC (boolean, bool);
JSON_T *json_new_boolean (const char *name, bool value) {
    JSON_SIMPLE (JSON_BOOLEAN, bool);
    return new;
}

JSON_PROP_FUNC_0 (null);
JSON_T *json_new_null (const char *name)
    { return json_new (name, JSON_NULL, NULL, 0); }

JSON_PROP_FUNC (object, int);
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

JSON_PROP_FUNC (dice, const sh_int *);
JSON_T *json_new_dice (const char *name, const sh_int *dice) {
    char buf[256];

    if (dice == NULL)
        return json_new_null (name);
    snprintf (buf, sizeof (buf), "%dd%d%+d", dice[0], dice[1], dice[2]);
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

static int json_indent_level = 0;
static int json_nest_level = 0;
static int json_indented = 0;

#define INDENT_SIZE 4

static void json_print_indent (FILE *fp) {
    static int last_indent = 0;
    static char space_buf[256];
    int i;

    if (json_indented)
        return;

    while (last_indent < json_indent_level) {
        int pos = last_indent * INDENT_SIZE;
        for (i = 0; i < INDENT_SIZE; i++)
            space_buf[pos++] = ' ';
        space_buf[pos] = '\0';
        last_indent++;
    }

    if (last_indent > json_indent_level) {
        space_buf[json_indent_level * INDENT_SIZE] = '\0';
        last_indent = json_indent_level;
    }

    fwrite (space_buf, sizeof (char), json_indent_level * INDENT_SIZE, fp);
    json_indented = 1;
}

static void json_next_line (FILE *fp) {
    fwrite ("\n", sizeof(char), 1, fp);
    json_indented = 0;
}

const char *json_escaped_string (const char *value) {
    static char buf[MAX_STRING_LENGTH * 4];
    char *pos = buf;

    *pos = '\0';
    while (*value != '\0') {
        switch (*value) {
            case '\b': strcat (pos, "\\b");  pos += 2; break;
            case '\f': strcat (pos, "\\f");  pos += 2; break;
            case '\n': strcat (pos, "\\n");  pos += 2; break;
            case '\t': strcat (pos, "\\t");  pos += 2; break;
            case '"':  strcat (pos, "\\\""); pos += 2; break;
            case '\\': strcat (pos, "\\\\"); pos += 2; break;

            case '\r': break;
            default:   *(pos++) = *value; *pos = '\0'; break;
        }
        ++value;
    }
    return buf;
}

void json_print_real (JSON_T *json, FILE *fp, int new_line) {
    json_print_indent (fp);
    if (json_nest_level > 0 && json->parent->type != JSON_ARRAY)
        fprintf (fp, "\"%s\": ", json->name
            ? json_escaped_string (json->name)
            : "NULL");
    switch (json->type) {
        case JSON_DICE:
        case JSON_STRING: {
            char *value = (char *) json->value;
            fprintf (fp, "\"%s\"", json_escaped_string (value));
            break;
        }
        case JSON_NUMBER: {
            json_num *value = (json_num *) json->value;
            fprintf (fp, "%g", *value);
            break;
        }
        case JSON_INTEGER: {
            json_int *value = (json_int *) json->value;
            fprintf (fp, "%ld", *value);
            break;
        }
        case JSON_BOOLEAN: {
            bool *value = (bool *) json->value;
            fprintf (fp, "%s", *value ? "true" : "false");
            break;
        }
        case JSON_NULL:
            fprintf (fp, "null");
            break;

        case JSON_OBJECT:
        case JSON_ARRAY: {
            JSON_T *j;
            char pleft  = (json->type == JSON_OBJECT) ? '{' : '[';
            char pright = (json->type == JSON_OBJECT) ? '}' : ']';

            if (json->first_child == NULL)
                fprintf (fp, "%c%c", pleft, pright);
            else if (json->child_count == 1) {
                fprintf (fp, "%c", pleft);
                json_nest_level++;
                json_print_real (json->first_child, fp, 0);
                json_nest_level--;
                fprintf (fp, "%c", pright);
            }
            else {
                fprintf (fp, "%c", pleft);
                json_next_line (fp);
                json_indent_level++;
                json_nest_level++;
                for (j = json->first_child; j != NULL; j = j->next)
                    json_print (j, fp);
                json_nest_level--;
                json_indent_level--;
                json_print_indent (fp);
                fprintf (fp, "%c", pright);
            }
            break;
        }

        default:
            bugf ("json_print: Unhandled type %d", json->type);
            fprintf (fp, "BAD-TYPE");
            break;
    }
    if (json->next)
        fprintf (fp, ",");

    #define IS_SIMPLE_TYPE(x) \
        ((x)->type == JSON_NULL || (x)->type == JSON_NUMBER || \
         (x)->type == JSON_INTEGER || ( \
            (x)->type == JSON_STRING && strlen((char *) (x)->value) <= 15))

    if (new_line) {
        if (json->parent && json->parent->type == JSON_ARRAY && json->next &&
              IS_SIMPLE_TYPE(json) && IS_SIMPLE_TYPE(json->next))
            fwrite (" ", sizeof(char), 1, fp);
        else
            json_next_line (fp);
    }

    #undef IS_SIMPLE_TYPE
}

void json_print (JSON_T *json, FILE *fp) {
    json_print_real (json, fp, 1);
}

void json_write_to_file (JSON_T *json, const char *filename) {
    FILE *fp = fopen (filename, "w");
    BAIL_IF_BUGF (fp == NULL,
        "json_write_to_file: Couldn't open '%s' for writing", filename);
    json_print (json, fp);
    fclose (fp);
}

int json_mkdir (const char *dir) {
    const char *last_slash, *end;
    if (dir == NULL || dir[0] == '\0')
        return 0;
    last_slash = strrchr (dir, '/');
    end = last_slash ? (last_slash + 1) : dir;
    if (!strcmp (end, ".") || !strcmp (end, ".."))
        return 0;
     return mkdir (dir, 0777) == 0;
}

int json_mkdir_to (const char *filename) {
    const char *pos, *next;
    char buf[256];
    int len = 0, made = 0;

    buf[0] = '\0';
    pos = filename;
    while (pos != NULL && *pos != '\0') {
        next = strchr (pos, '/');
        if (!next)
            break;
        len += snprintf (buf + len, sizeof (buf) - len, "%s%.*s",
            (buf[0] == '\0') ? "" : "/", (int) (next - pos), pos);
        json_mkdir (buf);
        pos = next + 1;
    }
    return made;
}
