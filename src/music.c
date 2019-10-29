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

#include "recycle.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interp.h"

#include "music.h"

int channel_songs[MAX_GLOBAL + 1];
struct song_data song_table[MAX_SONGS];

void song_update (void) {
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *room;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    char *line;
    int i;

    /* do the global song, if any */
    if (channel_songs[1] >= MAX_SONGS)
        channel_songs[1] = -1;

    if (channel_songs[1] > -1) {
        if (channel_songs[0] >= MAX_LINES
            || channel_songs[0] >= song_table[channel_songs[1]].lines)
        {
            channel_songs[0] = -1;

            /* advance songs */
            for (i = 1; i < MAX_GLOBAL; i++)
                channel_songs[i] = channel_songs[i + 1];
            channel_songs[MAX_GLOBAL] = -1;
        }
        else {
            if (channel_songs[0] < 0) {
                sprintf (buf, "{eMusic: {E%s, %s{x",
                         song_table[channel_songs[1]].group,
                         song_table[channel_songs[1]].name);
                channel_songs[0] = 0;
            }
            else {
                sprintf (buf, "{eMusic: '{E%s{e'{x",
                         song_table[channel_songs[1]].lyrics[channel_songs [0]]);
                channel_songs[0]++;
            }
            for (d = descriptor_list; d != NULL; d = d->next) {
                victim = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    !IS_SET (victim->comm, COMM_NOMUSIC) &&
                    !IS_SET (victim->comm, COMM_QUIET))
                    act_new ("$t", d->character, buf, NULL, TO_CHAR,
                             POS_SLEEPING);
            }
        }
    }

    for (obj = object_list; obj != NULL; obj = obj->next) {
        if (obj->item_type != ITEM_JUKEBOX || obj->v.value[1] < 0)
            continue;
        if (obj->v.value[1] >= MAX_SONGS) {
            obj->v.value[1] = -1;
            continue;
        }

        /* find which room to play in */
        if ((room = obj->in_room) == NULL) {
            if (obj->carried_by == NULL)
                continue;
            else if ((room = obj->carried_by->in_room) == NULL)
                continue;
        }

        if (obj->v.value[0] < 0) {
            sprintf (buf, "$p starts playing %s, %s.",
                     song_table[obj->v.value[1]].group,
                     song_table[obj->v.value[1]].name);
            if (room->people != NULL)
                act (buf, room->people, obj, NULL, TO_ALL);
            obj->v.value[0] = 0;
            continue;
        }
        else {
            if (obj->v.value[0] >= MAX_LINES ||
                obj->v.value[0] >= song_table[obj->v.value[1]].lines)
            {
                obj->v.value[0] = -1;

                /* scroll songs forward */
                obj->v.value[1] = obj->v.value[2];
                obj->v.value[2] = obj->v.value[3];
                obj->v.value[3] = obj->v.value[4];
                obj->v.value[4] = -1;
                continue;
            }

            line = song_table[obj->v.value[1]].lyrics[obj->v.value[0]];
            obj->v.value[0]++;
        }

        sprintf (buf, "{e$p bops: '{E%s{e'{x", line);
        if (room->people != NULL)
            act (buf, room->people, obj, NULL, TO_ALL);
    }
}

void load_songs (void) {
    FILE *fp;
    int count = 0, lines, i;
    char letter;

    /* reset global */
    for (i = 0; i <= MAX_GLOBAL; i++)
        channel_songs[i] = -1;

    if ((fp = fopen (MUSIC_FILE, "r")) == NULL) {
        bug ("Couldn't open music file, no songs available.", 0);
        fclose (fp);
        return;
    }
    for (count = 0; count < MAX_SONGS; count++) {
        letter = fread_letter (fp);
        if (letter == '#') {
            if (count < MAX_SONGS)
                song_table[count].name = NULL;
            fclose (fp);
            return;
        }
        else
            ungetc (letter, fp);

        song_table[count].group = fread_string (fp);
        song_table[count].name = fread_string (fp);

        /* read lyrics */
        lines = 0;

        while (1) {
            letter = fread_letter (fp);
            if (letter == '~') {
                song_table[count].lines = lines;
                break;
            }
            else
                ungetc (letter, fp);
            if (lines >= MAX_LINES) {
                bug ("Too many lines in a song -- limit is  %d.", MAX_LINES);
                break;
            }
            song_table[count].lyrics[lines] = fread_string_eol (fp);
            lines++;
        }
    }
}
