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
#include <sys/stat.h>

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
        exit (1);
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

static void json_read_skip_whitespace (const char **pos) {
    while (**pos != '\0' && json_read_is_whitespace_char (**pos))
        (*pos)++;
}

JSON_T *json_read_file (const char *filename) {
    JSON_T *obj;
    FILE *file;
    char *data;
    const char *pos;
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
    pos = data;
    json_read_skip_whitespace (&pos);

    /* can we start with an object... */
    if (*pos == '{')
        obj = json_read_object (&pos, filename);
    /* ...or an array. */
    else if (*pos == '[')
        obj = json_read_array (&pos, filename);
    /* nothing else! */
    else {
        fprintf (stderr, "json_read_file(): Expected '{' or '[', "
                         "found '%c'.\n", *pos);
        obj = NULL;
    }
    free (data);

    /* hopefully it worked! return the object we found. */
    return obj;
}

JSON_T *json_read_object (const char **pos, const char *name) {
    JSON_T *obj, *sub;
    char key[1024];

    /* objects must always start with a brace. */
    json_read_skip_whitespace (pos);
    if (**pos != '{') {
        fprintf (stderr, "json_read_object(): Expected '{', found '%c'.\n",
            **pos);
        return NULL;
    }

    (*pos)++;
    json_read_skip_whitespace (pos);

    /* start building a new object. */
    obj = json_new_object (name, JSON_OBJ_ANY);

    /* we can read JSON types until EOF or a closing bracket. */
    while (**pos != '}') {
        if (**pos == '\0') {
            fprintf (stderr, "json_read_object(): Expected '}' or string, "
                             "found EOF.\n");
            json_free (obj);
            return NULL;
        }

        /* read the member name. */
        if (!json_read_string_content (pos, key, sizeof (key))) {
            json_free (obj);
            return NULL;
        }

        /* we need a column after our member name. */
        json_read_skip_whitespace (pos);
        if (**pos == '\0') {
            fprintf (stderr, "json_read_object(): Expected ':', found EOF.");
            json_free (obj);
            return NULL;
        }
        else if (**pos != ':') {
            fprintf (stderr, "json_read_object(): Expected ':', found '%c'.",
                **pos);
            json_free (obj);
            return NULL;
        }
        (*pos)++;
        json_read_skip_whitespace (pos);

        /* read the value of the member. */
        json_read_skip_whitespace (pos);
        if ((sub = json_read_any_type (pos, key)) == NULL)
            break;
        json_attach_under (sub, obj);
        json_read_skip_whitespace (pos);

        /* manually break out of loops. */
        if (**pos == ',') {
            (*pos)++;
            json_read_skip_whitespace (pos);
        }
        else if (**pos != '}' && **pos != '\0') {
            fprintf (stderr, "json_read_object(): Expected '}' or ',', "
                             "found '%c'.\n", **pos);
            json_free (obj);
            return NULL;
        }
    }

    /* we should be at '}' at this point; skip ahead. */
    (*pos)++;
    json_read_skip_whitespace (pos);
    return obj;
}

JSON_T *json_read_array (const char **pos, const char *name) {
    JSON_T *array, *sub;

    /* objects must always start with a bracket. */
    json_read_skip_whitespace (pos);
    if (**pos != '[') {
        fprintf (stderr, "json_read_array(): Expected '[', found '%c'.\n",
            **pos);
        return NULL;
    }

    (*pos)++;
    json_read_skip_whitespace (pos);

    /* start building a new array. */
    array = json_new_array (name, NULL);

    /* we can read JSON types until EOF or a closing bracket. */
    while (**pos != ']') {
        if (**pos == '\0') {
            fprintf (stderr, "json_read_array(): Expected ']' or ',', "
                             "found EOF.\n");
            json_free (array);
            return NULL;
        }

        /* read the value. */
        json_read_skip_whitespace (pos);
        if ((sub = json_read_any_type (pos, NULL)) == NULL)
            break;
        json_attach_under (sub, array);
        json_read_skip_whitespace (pos);

        /* manually break out of loops. */
        if (**pos == ',') {
            (*pos)++;
            json_read_skip_whitespace (pos);
        }
        else if (**pos != ']' && **pos != '\0') {
            fprintf (stderr, "json_read_array(): Expected ']' or ',', "
                             "found '%c'.\n", **pos);
            json_free (array);
            return NULL;
        }
    }

    /* we should be at ']' at this point; skip ahead. */
    (*pos)++;
    json_read_skip_whitespace (pos);

    return array;
}

JSON_T *json_read_any_type (const char **pos, const char *name) {
    JSON_T *obj;

    /* the next character should indicate the type of this object. */
    json_read_skip_whitespace (pos);
    if (**pos == '\0') {
        fprintf (stderr, "json_read_any_type(): Expected a type, found EOF.\n");
        return NULL;
    }

    /* TODO: numbers aren't quite read properly here... */

    /* read strings... */
    if (**pos == '"')
        obj = json_read_string (pos, name);
    /* read numbers... */
    else if ((**pos >= '0' && **pos <= '9') || **pos == '-')
        obj = json_read_number (pos, name);
    /* read objects... */
    else if (**pos == '{')
        obj = json_read_object (pos, name);
    /* read arrays... */
    else if (**pos == '[')
        obj = json_read_array (pos, name);
    /* read anything else (true, false, null). */
    else
        obj = json_read_special (pos, name);

    return obj;
}

JSON_T *json_read_string (const char **pos, const char *name) {
    char buf[8192];
    if (!json_read_string_content (pos, buf, sizeof(buf)))
        return NULL;
    return json_new_string (name, buf);
}

char *json_read_string_content (const char **pos, char *buf, size_t size) {
    char ch;
    int buf_pos;

    /* strings must always start with a double quote. */
    json_read_skip_whitespace (pos);
    if (**pos != '\"') {
        fprintf (stderr, "json_read_string_content(): Expected '\"', "
                         "found '%c'.\n", **pos);
        return NULL;
    }
    (*pos)++;

    /* keep reading until we find another quote. */
    buf_pos = 0;
    while (**pos != '"') {
        if (**pos == '\0') {
            fprintf (stderr, "json_read_string(): Premature EOF found.\n");
            return NULL;
        }
        if (**pos == '\r') {
            (*pos)++;
            continue;
        }

        /* we're going to allow a special case: if we find a newline, read
         * spaces until we find '|' or anything else.  if we found '[ ]*|',
         * we start reading after the pipe. if not, go back to our original
         * position and read like normal. */
        if (**pos == '\n') {
            const char *pos_ahead;
            (*pos)++;
            for (pos_ahead = *pos; *pos_ahead != '\0'; pos_ahead++)
                if (*pos_ahead != ' ' && *pos_ahead != '\r')
                    break;
            if (*pos_ahead == '|') {
                *pos = pos_ahead + 1;
                if (buf_pos < size - 1)
                    buf[buf_pos++] = '\n';
            }
            continue;
        }

        /* parse special characters. */
        if (**pos == '\\') {
            (*pos)++;
            ch = 0;
            switch (**pos) {
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
                    if (**pos != '\0') (*pos)++;
                    if (**pos != '\0') (*pos)++;
                    if (**pos != '\0') (*pos)++;
                    if (**pos != '\0') (*pos)++;
                    break;

                case '\0':
                    fprintf (stderr, "json_read_string(): Excepted escape "
                        "sequence character, found EOF.\n");
                    return NULL;

                default:
                    fprintf (stderr, "json_read_string(): Invalid escape "
                        "sequence character '%c'.\n", **pos);
                    return NULL;
            }

            /* if there was a valid character to read, read it! */
            if (ch != 0) {
                if (buf_pos < size - 1)
                    buf[buf_pos++] = ch;
                (*pos)++;
            }
        }
        /* normal characters - append them to the string. */
        else {
            if (buf_pos < size - 1)
                buf[buf_pos++] = **pos;
            (*pos)++;
        }
    }
    buf[buf_pos] = '\0';

    /* we should be at '"' at this point; skip ahead. */
    (*pos)++;
    json_read_skip_whitespace (pos);

    /* return our new string. */
    return buf;
}

JSON_T *json_read_number (const char **pos, const char *name)
{
    char buf[1024];
    int buf_pos;
    bool found_dec;

    /* read everything that isn't whitespace or EOF. */
    buf_pos = 0;
    found_dec = FALSE;
    while (!json_read_is_whitespace_char (**pos) && **pos != '\0' &&
            **pos != ',' && **pos != ']' && **pos != '}')
    {
        if (**pos == '.')
            found_dec = TRUE;
        if (buf_pos < sizeof(buf) - 1)
            buf[buf_pos++] = **pos;
        (*pos)++;
    }
    buf[buf_pos] = '\0';
    json_read_skip_whitespace (pos);

    /* we handle either decimals or integers here. */
    if (found_dec)
        return json_new_number (name, atof (buf));
    else
        return json_new_integer (name, atoi (buf));
}

JSON_T *json_read_special (const char **pos, const char *name)
{
    char buf[16];
    int buf_pos;

    /* read everything that isn't whitespace or EOF. */
    buf_pos = 0;
    while (!json_read_is_whitespace_char (**pos) && **pos != '\0' &&
            **pos != ',' && **pos != ']' && **pos != '}')
    {
        if (buf_pos < sizeof(buf) - 1)
            buf[buf_pos++] = **pos;
        (*pos)++;
    }
    buf[buf_pos] = '\0';
    json_read_skip_whitespace (pos);

    /* parse the three special JSON values. */
    if (strcmp (buf, "true") == 0)
        return json_new_boolean (name, 1);
    else if (strcmp (buf, "false") == 0)
        return json_new_boolean (name, 0);
    else if (strcmp (buf, "null") == 0)
        return json_new_null (name);
    else {
        fprintf (stderr, "json_read_special(): Excepted 'true', 'false' or "
            "'null', found '%s'.\n", buf);
        return NULL;
    }
}
