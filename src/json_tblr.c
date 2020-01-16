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

#include <string.h>

#include "lookup.h"
#include "colour.h"
#include "memory.h"
#include "json_import.h"

#include "json_tblr.h"

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

DEFINE_JSON_READ_FUN (json_tblr_song) {
    char buf[256];
    JSON_T *array, *sub;

    JSON_TBLR_START (SONG_T, song, MAX_SONGS, song->name == NULL);

    if (!json_import_expect ("song", json,
            "name", "group", "lyrics", NULL))
        return NULL;

    song->name = str_dup (buf);
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
