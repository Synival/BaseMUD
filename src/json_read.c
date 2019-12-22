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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#include "json_read.h"

static int json_read_file_or_dir_filter (const struct dirent *dirent) {
    /* No hidden files or '.' / '..' entries. */
    if (dirent->d_name[0] == '.')
        return 0;
    return 1;
}

static bool json_read_filename_is_json_file (const char *filename) {
    filename = strrchr (filename, '.');
    if (filename == NULL)
        return FALSE;
    return (strcasecmp (filename, ".json") == 0) ? TRUE : FALSE;
}

JSON_T *json_read_directory_recursive (const char *path,
    int (*load_func) (JSON_T *), int *load_result)
{
    JSON_T *obj = json_new_object (NULL, JSON_OBJ_ANY);
    json_read_directory_real (obj, path, TRUE, load_func, load_result);
    return obj;
}

void json_read_directory_real (JSON_T *obj, const char *path, bool recurse,
    int (*load_func) (JSON_T *), int *load_result)
{
    JSON_T *json;
    struct dirent **files;
    struct stat stat_buf;
    char fbuf[1024];
    int count, i, len;

    count = scandir (path, &files, json_read_file_or_dir_filter, alphasort);
    if (count < 0) {
        perror ("scandir");
        return;
    }

    for (i = 0; i < count; i++) {
        len = snprintf (fbuf, sizeof(fbuf), "%s%s", path,
            files[i]->d_name);

        if (stat (fbuf, &stat_buf)) {
            perror ("stat");
            free (files[i]);
            continue;
        }

        free (files[i]);
        if (S_ISDIR (stat_buf.st_mode)) {
            if (recurse) {
                len += snprintf (fbuf + len, sizeof(fbuf) - len, "/");
                json_read_directory_real (obj, fbuf, recurse, load_func,
                    load_result);
            }
        }
        else if (json_read_filename_is_json_file (fbuf)) {
            log_f ("Loading '%s'", fbuf);
            json = json_read_file (fbuf);
            if (json != NULL) {
                if (load_func != NULL) {
                    int result = load_func (json);
                    if (load_result != NULL)
                        *load_result += result;
                    json_free (json);
                }
                else
                    json_attach_under (json, obj);
            }
        }
    }
    free (files);
}

static bool json_read_is_whitespace_char (char ch)
{
    return (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')
        ? TRUE : FALSE;
}

static void json_read_advance (JSON_READ_T *context) {
    if (*(context->pos) == '\n') {
        context->line++;
        context->col = 1;
    }
    else if (*(context->pos) == '\r')
        ;
    else
        context->col++;
    context->pos++;
}

static void json_read_skip_whitespace (JSON_READ_T *context) {
    while (*(context->pos) != '\0' &&
           json_read_is_whitespace_char (*(context->pos)))
        json_read_advance (context);
}

JSON_T *json_read_file (const char *filename) {
    JSON_T *obj;
    JSON_READ_T context;
    char *data;
    FILE *file;
    size_t file_len;

    if ((file = fopen (filename, "r")) == NULL)
        return NULL;

    /* let's be GREEDY and ready the entire file now for simplicity's sake. */
    /* start by getting file the length. */
    fseek (file, 0, SEEK_END);
    file_len = ftell (file);
    fseek (file, 0, SEEK_SET);

    /* read the entire file. if it doesn't work in one go, report an error
     * and bail. */
    data = malloc (file_len + 1);
    data[file_len] = '\0';
    if (fread (data, sizeof(char), file_len, file) != file_len) {
        perror ("fread");
        free (data);
        fclose (file);
        return NULL;
    }
    fclose (file);

    /* we should read a JSON object. */
    context.filename = filename;
    context.data = data;
    context.pos  = data;
    context.line = 1;
    context.col  = 1;
    json_read_skip_whitespace (&context);

    /* can we start with an object... */
    if (*(context.pos) == '{')
        obj = json_read_object (&context, filename);
    /* ...or an array. */
    else if (*(context.pos) == '[')
        obj = json_read_array (&context, filename);
    /* nothing else! */
    else {
        json_read_logf (&context, "json_read_file(): Expected '{' or '[', "
            "found '%c'.", *(context.pos));
        obj = NULL;
    }
    free (data);

    /* hopefully it worked! return the object we found. */
    return obj;
}

void json_read_logf (const JSON_READ_T *context, const char *format, ...) {
    char buf[2 * MSL];
    va_list args;
    va_start (args, format);
    vsnprintf (buf, sizeof(buf), format, args);
    va_end (args);
    log_f ("%s, line %d, col %d: %s", context->filename,
        context->line, context->col, buf);
}

static void json_read_apply_to_object (const JSON_READ_T *context,
    JSON_T *json)
{
    if (context->filename)
        json->filename = strdup (context->filename);
    json->line = context->line;
    json->col  = context->col;
}

JSON_T *json_read_object (JSON_READ_T *context, const char *name) {
    JSON_T *obj, *sub;
    char key[1024];

    /* objects must always start with a brace. */
    json_read_skip_whitespace (context);
    if (*(context->pos) != '{') {
        json_read_logf (context, "json_read_object(): Expected '{', "
            "found '%c'.", *(context->pos));
        return NULL;
    }

    /* start building a new object. */
    obj = json_new_object (name, JSON_OBJ_ANY);
    json_read_apply_to_object (context, obj);

    json_read_advance (context);
    json_read_skip_whitespace (context);

    /* we can read JSON types until EOF or a closing bracket. */
    while (*(context->pos) != '}') {
        if (*(context->pos) == '\0') {
            json_read_logf (context, "json_read_object(): Expected '}' or "
                "string, found EOF.");
            json_free (obj);
            return NULL;
        }

        /* read the member name. */
        if (!json_read_string_content (context, key, sizeof (key))) {
            json_free (obj);
            return NULL;
        }

        /* we need a column after our member name. */
        json_read_skip_whitespace (context);
        if (*(context->pos) == '\0') {
            json_read_logf (context, "json_read_object(): Expected ':', "
                "found EOF.");
            json_free (obj);
            return NULL;
        }
        else if (*(context->pos) != ':') {
            json_read_logf (context, "json_read_object(): Expected ':', "
                "found '%c'.", *(context->pos));
            json_free (obj);
            return NULL;
        }
        json_read_advance (context);
        json_read_skip_whitespace (context);

        /* read the value of the member. */
        json_read_skip_whitespace (context);
        if ((sub = json_read_any_type (context, key)) == NULL)
            break;
        json_attach_under (sub, obj);
        json_read_skip_whitespace (context);

        /* manually break out of loops. */
        if (*(context->pos) == ',') {
            json_read_advance (context);
            json_read_skip_whitespace (context);
        }
        else if (*(context->pos) != '}' && *(context->pos) != '\0') {
            json_read_logf (context, "json_read_object(): Expected '}' "
                "or ',', found '%c'.", *(context->pos));
            json_free (obj);
            return NULL;
        }
    }

    /* we should be at '}' at this point; skip ahead. */
    json_read_advance (context);
    json_read_skip_whitespace (context);
    return obj;
}

JSON_T *json_read_array (JSON_READ_T *context, const char *name) {
    JSON_T *array, *sub;

    /* objects must always start with a bracket. */
    json_read_skip_whitespace (context);
    if (*(context->pos) != '[') {
        json_read_logf (context, "json_read_array(): Expected '[', "
            "found '%c'.", *(context->pos));
        return NULL;
    }

    /* start building a new array. */
    array = json_new_array (name, NULL);
    json_read_apply_to_object (context, array);

    json_read_advance (context);
    json_read_skip_whitespace (context);

    /* we can read JSON types until EOF or a closing bracket. */
    while (*(context->pos) != ']') {
        if (*(context->pos) == '\0') {
            json_read_logf (context, "json_read_array(): Expected ']' or "
                "',', found EOF.");
            json_free (array);
            return NULL;
        }

        /* read the value. */
        json_read_skip_whitespace (context);
        if ((sub = json_read_any_type (context, NULL)) == NULL)
            break;
        json_attach_under (sub, array);
        json_read_skip_whitespace (context);

        /* manually break out of loops. */
        if (*(context->pos) == ',') {
            json_read_advance (context);
            json_read_skip_whitespace (context);
        }
        else if (*(context->pos) != ']' && *(context->pos) != '\0') {
            json_read_logf (context, "json_read_array(): Expected ']' or "
                "',', found '%c'.", *(context->pos));
            json_free (array);
            return NULL;
        }
    }

    /* we should be at ']' at this point; skip ahead. */
    json_read_advance (context);
    json_read_skip_whitespace (context);

    return array;
}

JSON_T *json_read_any_type (JSON_READ_T *context, const char *name) {
    JSON_T *obj;
    char ch;

    /* the next character should indicate the type of this object. */
    json_read_skip_whitespace (context);
    if (*(context->pos) == '\0') {
        json_read_logf (context, "json_read_any_type(): Expected a type, "
            "found EOF.");
        return NULL;
    }

    /* TODO: numbers aren't quite read properly here... */

    /* read strings... */
    ch = *(context->pos);
    if (ch == '"')
        obj = json_read_string (context, name);
    /* read numbers... */
    else if ((ch >= '0' && ch <= '9') || ch == '-')
        obj = json_read_number (context, name);
    /* read objects... */
    else if (ch == '{')
        obj = json_read_object (context, name);
    /* read arrays... */
    else if (ch == '[')
        obj = json_read_array (context, name);
    /* read anything else (true, false, null). */
    else
        obj = json_read_special (context, name);

    return obj;
}

JSON_T *json_read_string (JSON_READ_T *context, const char *name) {
    JSON_T *obj;
    char buf[8192];

    if (!json_read_string_content (context, buf, sizeof(buf)))
        return NULL;

    obj = json_new_string (name, buf);
    json_read_apply_to_object (context, obj);
    return obj;
}

char *json_read_string_content (JSON_READ_T *context, char *buf, size_t size) {
    char ch;
    int buf_pos;

    /* strings must always start with a double quote. */
    json_read_skip_whitespace (context);
    if (*(context->pos) != '\"') {
        json_read_logf (context, "json_read_string_content(): Expected "
            "'\"', found '%c'.", *(context->pos));
        return NULL;
    }
    json_read_advance (context);

    /* keep reading until we find another quote. */
    buf_pos = 0;
    while (*(context->pos) != '"') {
        if (*(context->pos) == '\0') {
            json_read_logf (context, "json_read_string(): Premature EOF "
                "found.");
            return NULL;
        }
        if (*(context->pos) == '\r') {
            json_read_advance (context);
            continue;
        }

        /* we're going to allow a special case: if we find a newline, read
         * spaces until we find '|' or anything else.  if we found '[ ]*|',
         * we start reading after the pipe. if not, go back to our original
         * position and read like normal. */
        if (*(context->pos) == '\n') {
            const char *pos_ahead;
            json_read_advance (context);
            for (pos_ahead = context->pos; *pos_ahead != '\0'; pos_ahead++)
                if (*pos_ahead != ' ' && *pos_ahead != '\r')
                    break;
            if (*pos_ahead == '|') {
                while (context->pos != pos_ahead)
                    json_read_advance (context);
                json_read_advance (context);
                if (buf_pos < size - 1)
                    buf[buf_pos++] = '\n';
            }
            continue;
        }

        /* parse special characters. */
        if (*(context->pos) == '\\') {
            json_read_advance (context);
            ch = 0;
            switch (*(context->pos)) {
                case '"':  ch = '"';  break;
                case '\\': ch = '\\'; break;
                case '/':  ch = '/';  break;
                case 'b':  ch = '\b'; break;
                case 'f':  ch = '\f'; break;
                case 'n':  ch = '\n'; break;
                case 'r':  ch = '\r'; break;
                case 't':  ch = '\t'; break;

                /* unicode: TODO :( */
                case 'u':
                    if (*(context->pos) != '\0') json_read_advance (context);
                    if (*(context->pos) != '\0') json_read_advance (context);
                    if (*(context->pos) != '\0') json_read_advance (context);
                    if (*(context->pos) != '\0') json_read_advance (context);
                    break;

                case '\0':
                    json_read_logf (context, "json_read_string(): Excepted "
                        "escape sequence character, found EOF.");
                    return NULL;

                default:
                    json_read_logf (context, "json_read_string(): Invalid "
                        "escape sequence character '%c'.", *(context->pos));
                    return NULL;
            }

            /* if there was a valid character to read, read it! */
            if (ch != 0) {
                if (buf_pos < size - 1)
                    buf[buf_pos++] = ch;
                json_read_advance (context);
            }
        }
        /* normal characters - append them to the string. */
        else {
            if (buf_pos < size - 1)
                buf[buf_pos++] = *(context->pos);
            json_read_advance (context);
        }
    }
    buf[buf_pos] = '\0';

    /* we should be at '"' at this point; skip ahead. */
    json_read_advance (context);
    json_read_skip_whitespace (context);

    /* return our new string. */
    return buf;
}

JSON_T *json_read_number (JSON_READ_T *context, const char *name)
{
    JSON_T *obj;
    char buf[1024], ch;
    int buf_pos;
    bool found_dec;

    /* read everything that isn't whitespace or EOF. */
    buf_pos = 0;
    found_dec = FALSE;
    while (1) {
        ch = *(context->pos);
        if (json_read_is_whitespace_char (ch) || ch == '\0' || ch == ',' ||
            ch == ']' || ch == '}')
        {
            break;
        }

        if (ch == '.')
            found_dec = TRUE;
        if (buf_pos < sizeof(buf) - 1)
            buf[buf_pos++] = ch;
        json_read_advance (context);
    }
    buf[buf_pos] = '\0';
    json_read_skip_whitespace (context);

    /* we handle either decimals or integers here. */
    if (found_dec)
        obj = json_new_number (name, atof (buf));
    else
        obj = json_new_integer (name, atoi (buf));
    json_read_apply_to_object (context, obj);
    return obj;
}

JSON_T *json_read_special (JSON_READ_T *context, const char *name)
{
    JSON_T *obj;
    char buf[16], ch;
    int buf_pos;

    /* read everything that isn't whitespace or EOF. */
    buf_pos = 0;
    while (1) {
        ch = *(context->pos);
        if (json_read_is_whitespace_char (ch) || ch == '\0' || ch == ',' ||
            ch == ']' || ch == '}')
        {
            break;
        }

        if (buf_pos < sizeof(buf) - 1)
            buf[buf_pos++] = ch;
        json_read_advance (context);
    }
    buf[buf_pos] = '\0';
    json_read_skip_whitespace (context);

    /* parse the three special JSON values. */
    if (strcmp (buf, "true") == 0)
        obj = json_new_boolean (name, 1);
    else if (strcmp (buf, "false") == 0)
        obj = json_new_boolean (name, 0);
    else if (strcmp (buf, "null") == 0)
        obj = json_new_null (name);
    else {
        json_read_logf (context, "json_read_special(): Excepted 'true', "
            "'false' or 'null', found '%s'.", buf);
        return NULL;
    }

    json_read_apply_to_object (context, obj);
    return obj;
}
