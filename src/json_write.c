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

#include <sys/stat.h>

#include <string.h>

#include "utils.h"

#include "json_write.h"

static int json_indent_level = 0;
static int json_nest_level = 0;
static int json_indented = 0;

#define INDENT_SIZE 2

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
    fputc ('\n', fp);
    json_indented = 0;
}

const char *json_escaped_string (const char *value, int newline_pos) {
    static char buf[MAX_STRING_LENGTH * 4];
    char *pos = buf;

    *pos = '\0';
    while (*value != '\0') {
        switch (*value) {
            case '\b': strcat (pos, "\\b");  pos += 2; break;
            case '\f': strcat (pos, "\\f");  pos += 2; break;
            case '\t': strcat (pos, "\\t");  pos += 2; break;
            case '"':  strcat (pos, "\\\""); pos += 2; break;
            case '\\': strcat (pos, "\\\\"); pos += 2; break;

            case '\n': {
#ifdef BASEMUD_WRITE_EXTENDED_JSON
                const char *next_ch = value + 1;
                while (*next_ch == '\r' && *next_ch != '\0')
                    next_ch++;
                pos += sprintf (pos, "\n%*c", newline_pos + 1, '|');
#else
                strcat (pos, "\\n");
                pos += 2;
#endif
                break;
            }

            case '\r':
                break;
            default:
                *(pos++) = *value;
                *pos = '\0';
                break;
        }
        ++value;
    }
    return buf;
}

void json_print_real (JSON_T *json, FILE *fp, int new_line) {
    int newline_pos;

    json_print_indent (fp);
    newline_pos = INDENT_SIZE * json_indent_level;

    if (json_nest_level > 0 && json->parent->type != JSON_ARRAY) {
        const char *json_name = json->name
            ? json_escaped_string (json->name, newline_pos) : "NULL";
        fputc ('"', fp);
        fputs (json_name, fp);
        fputs ("\": ", fp);
        newline_pos += 4 + strlen (json_name);
    }

    switch (json->type) {
        case JSON_DICE:
        case JSON_STRING: {
            char *value = (char *) json->value;
            fputc ('"', fp);
            fputs (json_escaped_string (value, newline_pos), fp);
            fputc ('"', fp);
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
            fputs (*value ? "true" : "false", fp);
            break;
        }
        case JSON_NULL:
            fputs ("null", fp);
            break;

        case JSON_OBJECT:
        case JSON_ARRAY: {
            JSON_T *j;
            char pleft  = (json->type == JSON_OBJECT) ? '{' : '[';
            char pright = (json->type == JSON_OBJECT) ? '}' : ']';

            if (json->first_child == NULL) {
                fputc (pleft, fp);
                fputc (pright, fp);
            }
            else if (json->child_count == 1) {
                fputc (pleft, fp);
                json_nest_level++;
                json_print_real (json->first_child, fp, 0);
                json_nest_level--;
                fputc (pright, fp);
            }
            else {
                fputc (pleft, fp);
                json_next_line (fp);
                json_indent_level++;
                json_nest_level++;
                for (j = json->first_child; j != NULL; j = j->next)
                    json_print (j, fp);
                json_nest_level--;
                json_indent_level--;
                json_print_indent (fp);
                fputc (pright, fp);
            }
            break;
        }

        default:
            bugf ("json_print(): Unhandled type %d", json->type);
            fputs ("BAD-TYPE", fp);
            break;
    }
    if (json->next)
        fputc (',', fp);

    #define IS_SIMPLE_TYPE(x) \
        ((x)->type == JSON_NULL || (x)->type == JSON_NUMBER || \
         (x)->type == JSON_INTEGER || ( \
            (x)->type == JSON_STRING && strlen((char *) (x)->value) <= 15))

    if (new_line) {
        if (json->parent && json->parent->type == JSON_ARRAY && json->next &&
              IS_SIMPLE_TYPE(json) && IS_SIMPLE_TYPE(json->next))
            fputc (' ', fp);
        else
            json_next_line (fp);
    }

    #undef IS_SIMPLE_TYPE
}

void json_print (JSON_T *json, FILE *fp) {
    json_print_real (json, fp, 1);
}

void json_fwrite (JSON_T *json, const char *filename) {
    FILE *fp = fopen (filename, "w");
    BAIL_IF_BUGF (fp == NULL,
        "json_fwrite(): Couldn't open '%s' for writing", filename);
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
