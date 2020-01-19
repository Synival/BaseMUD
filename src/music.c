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
#include "globals.h"
#include "lookup.h"
#include "chars.h"
#include "objs.h"
#include "memory.h"

#include "music.h"

bool music_queue_values (flag_t *line, flag_t *song_num, flag_t *queue,
    int queue_max, int song)
{
    int i;

    /* Start the first song. */
    if (*song_num < 0) {
        *song_num = song;
        *line = -1;
        return TRUE;
    }

    /* If there's already an active song, try to add it to the queue. */
    for (i = 0; i < queue_max; i++) {
        if (queue[i] < 0) {
            queue[i] = song;
            return TRUE;
        }
    }

    /* If the queue is full, return FALSE for an error. */
    return FALSE;
}

bool music_update_values (flag_t *line, flag_t *song_num, flag_t *queue,
    int queue_max, int *line_out)
{
    const SONG_T *song;
    int i;

    /* return FALSE if there's no song to play. */
    if (*song_num < 0)
        return FALSE;
    if (*song_num >= MAX_SONGS) {
        *song_num = -1;
        return FALSE;
    }

    /* is the current song finished? */
    song = song_get (*song_num);
    if (*line >= UMIN (MAX_SONG_LINES, song->lines)) {
        *line = -1;

        /* advance songs */
        *song_num = queue[0];
        for (i = 0; i < queue_max - 1; i++)
            queue[i] = queue[i + 1];
        queue[queue_max - 1] = -1;

        /* try to play the next song immediately. */
        return music_update_values (line, song_num, queue, queue_max, line_out);
    }

    *line_out = *line;
    *line = (*line < 0) ? 0 : (*line + 1);
    return TRUE;
}

void music_update_global (void) {
    const SONG_T *song;
    CHAR_T *ch;
    DESCRIPTOR_T *d;
    char buf[MAX_STRING_LENGTH];
    int line;

    /* attempt to advance our songs and get the next line. */
    if (!music_update_values (&music_line, &music_song, music_queue,
            MAX_SONG_GLOBAL, &line))
        return;

    /* have we started playing the song yet? */
    song = song_get (music_song);
    if (line < 0)
        sprintf (buf, "{eMusic: {E%s, %s{x", song->group, song->name);
    else
        sprintf (buf, "{eMusic: '{E%s{e'{x", song->lyrics[line]);

    /* play this song for all players. */
    for (d = descriptor_list; d != NULL; d = d->next) {
        ch = CH(d);
        if (d->connected == CON_PLAYING &&
            !IS_SET (ch->comm, COMM_NOMUSIC) &&
            !IS_SET (ch->comm, COMM_QUIET))
            act_new ("$t", d->character, buf, NULL, TO_CHAR,
                     POS_SLEEPING);
    }
}

void music_update_object (OBJ_T *obj) {
    const SONG_T *song;
    ROOM_INDEX_T *room;
    char buf[MAX_STRING_LENGTH];
    int line;

    /* attempt to advance our songs and get the next line. */
    if (!music_update_values (&(obj->v.jukebox.line), &(obj->v.jukebox.song),
            obj->v.jukebox.queue, JUKEBOX_QUEUE_MAX, &line))
        return;

    /* find which room to play in */
    if ((room = obj_get_room (obj)) == NULL)
        return;

    /* have we started playing the song yet? */
    if (room->people != NULL) {
        song = song_get (obj->v.jukebox.song);
        if (line < 0)
            sprintf (buf, "$p starts playing %s, %s.", song->group, song->name);
        else
            sprintf (buf, "{e$p bops: '{E%s{e'{x", song->lyrics[line]);
        act (buf, room->people, obj, NULL, TO_ALL);
    }
}

void music_load_songs (void) {
    FILE *fp;
    SONG_T *song;
    int count = 0, lines, i;
    char letter;

    /* reset global */
    music_line = -1;
    music_song = -1;
    for (i = 0; i < MAX_SONG_GLOBAL; i++)
        music_queue[i] = -1;

    if ((fp = fopen (MUSIC_FILE, "r")) == NULL) {
        bug ("Couldn't open music file, no songs available.", 0);
        fclose (fp);
        return;
    }

    for (count = 0; count < MAX_SONGS; count++) {
        song = &(song_table[count]);
        if (song->name != NULL)
            continue;

        letter = fread_letter (fp);
        if (letter == '#') {
            fclose (fp);
            return;
        }
        else
            ungetc (letter, fp);

        fread_string_replace (fp, &song->group);
        fread_string_replace (fp, &song->name);

        /* read lyrics */
        lines = 0;

        while (1) {
            letter = fread_letter (fp);
            if (letter == '~') {
                song->lines = lines;
                break;
            }
            else
                ungetc (letter, fp);
            if (lines >= MAX_SONG_LINES) {
                bug ("Too many lines in a song -- limit is %d.",
                    MAX_SONG_LINES);
                break;
            }
            fread_string_eol_replace (fp, &(song->lyrics[lines]));
            lines++;
        }

        /* make sure this song doesn't already exist. */
        if (song_lookup_exact (song->name) != count) {
            str_free (&(song->name));
            str_free (&(song->group));
            for (i = 0; i < song->lines; i++)
                str_free (&(song->lyrics[i]));
            song->lines = 0;
            count--;
        }
    }
}

void music_list_jukebox_songs (const OBJ_T *obj, CHAR_T *ch,
    const char *filter)
{
    BUFFER_T *buffer;
    const SONG_T *song;
    char arg[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    int i, col = 0;
    bool artist = FALSE, match = FALSE;

    buffer = buf_new ();
    filter = one_argument (filter, arg);

    if (!str_cmp (arg, "artist"))
        artist = TRUE;
    if (filter[0] != '\0')
        match = TRUE;

    sprintf (buf, "%s has the following songs available:\n\r",
             obj->short_descr);
    buf_cat (buffer, str_capitalized (buf));

    for (i = 0; i < MAX_SONGS; i++) {
        if ((song = &(song_table[i]))->name == NULL)
            break;

        if (artist && (!match || !str_prefix (filter, song->group)))
            sprintf (buf, "%-39s %-39s\n\r", song->group, song->name);
        else if (!artist && (!match || !str_prefix (filter, song->name)))
            sprintf (buf, "%-35s ", song->name);
        else
            continue;

        buf_cat (buffer, buf);
        if (!artist && ++col % 2 == 0)
            buf_cat (buffer, "\n\r");
    }
    if (!artist && col % 2 != 0)
        buf_cat (buffer, "\n\r");

    page_to_char (buf_string (buffer), ch);
    buf_free (buffer);
}
