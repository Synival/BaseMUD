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

/*

 Note Board system, (c) 1995-96 Erwin S. Andreasen, erwin@pip.dknet.dk
 =====================================================================

 Basically, the notes are split up into several boards. The boards do not
 exist physically, they can be read anywhere and in any position.

 Each of the note boards has its own file. Each of the boards can have its own
 "rights": who can read/write.

 Each character has an extra field added, namele the timestamp of the last note
 read by him/her on a certain board.

 The note entering system is changed too, making it more interactive. When
 entering a note, a character is put AFK and into a special CON_ state.
 Everything typed goes into the note.

 For the immortals it is possible to purge notes based on age. An Archive
 options is available which moves the notes older than X days into a special
 board. The file of this board should then be moved into some other directory
 during e.g. the startup script and perhaps renamed depending on date.

 Note that write_level MUST be >= read_level or else there will be strange
 output in certain functions.

 Board DEFAULT_BOARD must be at least readable by *everyone*.

*/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "interp.h"
#include "db.h"
#include "utils.h"
#include "comm.h"
#include "recycle.h"
#include "lookup.h"
#include "chars.h"
#include "descs.h"
#include "memory.h"
#include "globals.h"

#include "board.h"

#define L_SUP (MAX_LEVEL - 1) /* if not already defined */

/* The prompt that the character is given after finishing a note with ~ or END */
static const char *note_finish_prompt =
    "({WC{x)ontinue, ({WV{x)iew, ({WP{x)ost or ({WF{x)orget it?";
static long last_note_stamp = 0; /* To generate unique timestamps on notes */

#define BOARD_NOACCESS -1
#define BOARD_NOTFOUND -1

/* append this note to the given file */
void note_write (NOTE_T *note, FILE *fp) {
    fprintf (fp, "Sender  %s~\n", note->sender);
    fprintf (fp, "Date    %s~\n", note->date);
    fprintf (fp, "Stamp   %ld\n", note->date_stamp);
    fprintf (fp, "Expire  %ld\n", note->expire);
    fprintf (fp, "To      %s~\n", note->to_list);
    fprintf (fp, "Subject %s~\n", note->subject);
    fprintf (fp, "Text\n%s~\n\n", note->text);
}

/* Save a note in a given board */
void note_finish (NOTE_T *note, BOARD_T *board) {
    FILE *fp;
    char filename[200];

    /* The following is done in order to generate unique date_stamps */
    if (last_note_stamp >= current_time)
        note->date_stamp = ++last_note_stamp;
    else {
        note->date_stamp = current_time;
        last_note_stamp = current_time;
    }

    /* place this note at the end of the note list. */
    LIST_BACK (note, next, board->note_first, NOTE_T);

    /* append note to note file */
    sprintf (filename, "%s/%s", NOTE_DIR, board->name);
    if ((fp = fopen (filename, "a")) == NULL) {
        bug ("Could not open one of the note files in append mode", 0);
        board->changed = TRUE; /* set it to TRUE hope it will be OK later? */
        return;
    }

    note_write (note, fp);
    fclose (fp);
}

/* Find the number of a board */
int board_number (const BOARD_T *board) {
    int i;
    for (i = 0; i < BOARD_MAX; i++)
        if (board == &board_table[i])
            return i;
    return -1;
}

/* Remove list from the list. Do not free note */
void note_unlink (NOTE_T *note, BOARD_T *board) {
    LIST_REMOVE (note, next, board->note_first, NOTE_T, NO_FAIL);
}

/* Find the nth note on a board. Return NULL if ch has no access to that note */
NOTE_T *board_find_note (BOARD_T *board, CHAR_T *ch, int num) {
    int count = 0;
    NOTE_T *p;

    for (p = board->note_first; p ; p = p->next)
        if (++count == num)
            break;
    if ((count == num) && note_is_for_char (p, ch))
        return p;
    else
        return NULL;
}

/* save a single board */
void board_save (BOARD_T *board) {
    FILE *fp;
    char filename[200];
    NOTE_T *note;

    sprintf (filename, "%s/%s", NOTE_DIR, board->name);
    BAIL_IF_BUGF ((fp = fopen (filename, "w")) == NULL,
        "Error writing to: %s", filename);

    for (note = board->note_first; note ; note = note->next)
        note_write (note, fp);
    fclose (fp);
}

/* Show one not to a character */
void note_show_to_char (NOTE_T *note, CHAR_T *ch, int num) {
    /* Ugly colors ? */
    printf_to_char (ch,
        "[{W%4d{x] {Y%s{x: {g%s{x\n\r"
        "{YDate{x:  %s\n\r"
        "{YTo{x:    %s\n\r"
        "{g==========================================================================={x\n\r"
        "%s\n\r",
        num, note->sender, note->subject, note->date,
        note->to_list, note->text);
}

/* Save changed boards */
void board_save_all () {
    int i;
    for (i = 0; i < BOARD_MAX; i++)
        if (board_table[i].changed) /* only save changed boards */
            board_save (&board_table[i]);
}

/* Load a single board */
void board_load (BOARD_T *board) {
    FILE *fp, *fp_archive;
    NOTE_T *last_note;
    char filename[200];

    sprintf (filename, "%s/%s", NOTE_DIR, board->name);
    fp = fopen (filename, "r");

    /* Silently return */
    if (!fp)
        return;

    /* Start note fetching. copy of db.c:load_notes() */
    last_note = NULL;

    while (1) {
        NOTE_T *pnote;
        char letter;

        do {
            letter = getc(fp);
            if (feof(fp)) {
                fclose (fp);
                return;
            }
        }
        while (isspace(letter));
        ungetc (letter, fp);

        pnote = note_new ();
        if (str_cmp (fread_word_static (fp), "sender"))
            break;
        fread_string_replace (fp, &(pnote->sender));
        if (str_cmp (fread_word_static (fp), "date"))
            break;
        fread_string_replace (fp, &(pnote->date));
        if (str_cmp (fread_word_static (fp), "stamp"))
            break;
        pnote->date_stamp = fread_number (fp);
        if (str_cmp (fread_word_static (fp), "expire"))
            break;
        pnote->expire = fread_number (fp);
        if (str_cmp (fread_word_static (fp), "to"))
            break;
        fread_string_replace (fp, &(pnote->to_list));
        if (str_cmp (fread_word_static (fp), "subject"))
            break;
        fread_string_replace (fp, &(pnote->subject));
        if (str_cmp (fread_word_static (fp), "text"))
            break;
        fread_string_replace (fp, &(pnote->text));

        /* Should this note be archived right now ? */
        if (pnote->expire < current_time) {
            char archive_name[200];

            sprintf (archive_name, "%s/%s.old", NOTE_DIR, board->name);
            fp_archive = fopen (archive_name, "a");
            if (!fp_archive)
                bug ("Could not open archive boards for writing", 0);
            else {
                note_write (pnote, fp_archive);
                fclose (fp_archive); /* it might be more efficient to close this later */
            }
            note_free (pnote);
            board->changed = TRUE;
            continue;
        }

        LISTB_BACK (pnote, next, board->note_first, last_note);
    }

    bug ("load_notes: bad key word.", 0);
}

/* Initialize structures. Load all boards. */
void board_load_all () {
    int i;
    for (i = 0; i < BOARD_MAX; i++)
        board_load (&board_table[i]);
}

/* Returns TRUE if the specified note is address to ch */
bool note_is_for_char (NOTE_T *note, CHAR_T *ch) {
    if (!str_cmp (ch->name, note->sender))
        return TRUE;
    if (str_in_namelist_exact ("all", note->to_list))
        return TRUE;

    if (IS_IMMORTAL(ch) && (
        str_in_namelist_exact ("imm", note->to_list) ||
        str_in_namelist_exact ("imms", note->to_list) ||
        str_in_namelist_exact ("immortal", note->to_list) ||
        str_in_namelist_exact ("god", note->to_list) ||
        str_in_namelist_exact ("gods", note->to_list) ||
        str_in_namelist_exact ("immortals", note->to_list)))
        return TRUE;

    if ((char_get_trust(ch) == MAX_LEVEL) && (
        str_in_namelist_exact ("imp", note->to_list) ||
        str_in_namelist_exact ("imps", note->to_list) ||
        str_in_namelist_exact ("implementor", note->to_list) ||
        str_in_namelist_exact ("implementors", note->to_list)))
        return TRUE;

    if (str_in_namelist_exact (ch->name, note->to_list))
        return TRUE;

    /* Allow a note to e.g. 40 to send to characters level 40 and above */
    if (is_number (note->to_list) && char_get_trust (ch) >= atoi (note->to_list))
        return TRUE;

    return FALSE;
}

/* Return the number of unread notes 'ch' has in 'board' */
/* Returns BOARD_NOACCESS if ch has no access to board */
int board_get_unread_notes_for_char (BOARD_T *board, CHAR_T *ch) {
    NOTE_T *note;
    time_t last_read;
    int count = 0;

    if (board->read_level > char_get_trust(ch))
        return BOARD_NOACCESS;

    last_read = ch->pcdata->last_note[board_number(board)];
    for (note = board->note_first; note; note = note->next)
        if (note_is_for_char (note, ch) && ((long)last_read < (long)note->date_stamp))
            count++;

    return count;
}

/* COMMANDS */

/* Send a note to someone on the personal board */
NOTE_T *note_create_personal (const char *sender, const char *to,
    const char *subject, const int expire_days, const char *text)
{
    return note_create ("Personal", sender, to, subject, expire_days, text);
}

NOTE_T *note_create (const char* board_name, const char *sender, const char *to,
    const char *subject, const int expire_days, const char *text)
{
    int board_index = board_lookup (board_name);
    BOARD_T *board;
    NOTE_T *note;
    char *strtime;

    RETURN_IF_BUG (board_index == BOARD_NOTFOUND,
        "note_create: board not found", 0, NULL);
    RETURN_IF_BUG (strlen (text) > MAX_NOTE_TEXT,
        "note_create: text too long (%d bytes)", strlen (text), NULL);

    board = &board_table[board_index];

    note = note_new (); /* allocate new note */
    note->sender  = str_dup (sender);
    note->to_list = str_dup (to);
    note->subject = str_dup (subject);
    note->expire  = current_time + expire_days * 60 * 60 * 24;
    note->text    = str_dup (text);

    /* convert to ascii. */
    strtime = ctime_fixed (&current_time);
    note->date = str_dup (strtime);

    note_finish (note, board);
    return note;
}

DEFINE_NANNY_FUN (handle_con_note_to) {
    char buf [MAX_INPUT_LENGTH];
    CHAR_T *ch = d->character;

    if (!ch->pcdata->in_progress) {
        d->connected = CON_PLAYING;
        bug ("nanny: In CON_NOTE_TO, but no note in progress",0);
        return;
    }

    strcpy (buf, argument);
    str_smash_tilde (buf); /* change ~ to - as we save this field as a string later */

    switch (ch->pcdata->board->force_type) {
        case DEF_NORMAL: /* default field */
            if (!buf[0]) { /* empty string? */
                ch->pcdata->in_progress->to_list = str_dup (ch->pcdata->board->names);
                printf_to_desc (d, "Assumed default recipient: {W%s{x\n\r",
                    ch->pcdata->board->names);
            }
            else
                ch->pcdata->in_progress->to_list = str_dup (buf);
            break;

        case DEF_INCLUDE: /* forced default */
            if (!str_in_namelist_exact (ch->pcdata->board->names, buf)) {
                strcat (buf, " ");
                strcat (buf, ch->pcdata->board->names);
                ch->pcdata->in_progress->to_list = str_dup (buf);

                printf_to_desc (d, "\n\rYou did not specify %s as recipient, so it was automatically added.\n\r"
                         "{YNew To{x :  %s\n\r",
                         ch->pcdata->board->names, ch->pcdata->in_progress->to_list);
            }
            else
                ch->pcdata->in_progress->to_list = str_dup (buf);
            break;

        case DEF_EXCLUDE: /* forced exclude */
            if (!buf[0]) {
                send_to_desc ("You must specify a recipient.\n\r"
                              "{YTo{x:      ", d);
                return;
            }
            if (str_in_namelist_exact (ch->pcdata->board->names, buf)) {
                printf_to_desc (d,
                    "You are not allowed to send notes to %s on this board. Try again.\n\r"
                    "{YTo{x:      ", ch->pcdata->board->names);

                /* return from nanny, not changing to the next state! */
                return;
            }
            else
                ch->pcdata->in_progress->to_list = str_dup (buf);
            break;
    }

    send_to_desc ("{Y\n\rSubject{x: ", d);
    d->connected = CON_NOTE_SUBJECT;
}

DEFINE_NANNY_FUN (handle_con_note_subject) {
    char buf [MAX_INPUT_LENGTH];
    CHAR_T *ch = d->character;

    if (!ch->pcdata->in_progress) {
        d->connected = CON_PLAYING;
        bug ("nanny: In CON_NOTE_SUBJECT, but no note in progress",0);
        return;
    }

    strcpy (buf, argument);
    str_smash_tilde (buf); /* change ~ to - as we save this field as a string later */

    /* Do not allow empty subjects */
    if (!buf[0])        {
        write_to_buffer (d, "Please find a meaningful subject!\n\r",0);
        printf_to_desc (d, "{YSubject{x: ");
    }
    else  if (strlen(buf)>60) {
        write_to_buffer (d, "No, no. This is just the Subject. "
            "You're not writing the note yet. Twit.\n\r", 0);
    }
    /* advance to next stage */
    else {
        ch->pcdata->in_progress->subject = str_dup (buf);
        if (IS_IMMORTAL(ch)) { /* immortals get to choose number of expire days */
            printf_to_desc (d, "\n\r"
                "How many days do you want this note to expire in?\n\r"
                "Press Enter for default value for this board, {W%d{x days.\n\r"
                "{YExpire{x:  ",
                ch->pcdata->board->purge_days);
            d->connected = CON_NOTE_EXPIRE;
        }
        else {
            ch->pcdata->in_progress->expire =
                current_time + ch->pcdata->board->purge_days * 24L * 3600L;
            printf_to_desc (d,
                "This note will expire %s\n\r",
                ctime_fixed (&ch->pcdata->in_progress->expire));
            send_to_desc ("\n\r"
                "Enter text. Type {W~{x or {WEND{x on an empty line to end note.\n\r"
                "=======================================================\n\r", d);
            d->connected = CON_NOTE_TEXT;
        }
    }
}

DEFINE_NANNY_FUN (handle_con_note_expire) {
    CHAR_T *ch = d->character;
    char buf[MAX_STRING_LENGTH];
    time_t expire;
    int days;

    if (!ch->pcdata->in_progress) {
        d->connected = CON_PLAYING;
        bug ("nanny: In CON_NOTE_EXPIRE, but no note in progress",0);
        return;
    }

    /* Numeric argument. no tilde smashing */
    strcpy (buf, argument);
    if (!buf[0]) /* assume default expire */
        days =     ch->pcdata->board->purge_days;
    else { /* use this expire */
        if (!is_number(buf)) {
            write_to_buffer (d,"Write the number of days!\n\r",0);
            send_to_desc ("{YExpire{x:  ",d);
            return;
        }
        else {
            days = atoi (buf);
            if (days <= 0) {
                write_to_buffer (d, "This is a positive MUD. Use positive numbers only! :)\n\r",0);
                send_to_desc ("{YExpire{x:  ",d);
                return;
            }
        }
    }

    expire = current_time + (days * 24L * 3600L); /* 24 hours, 3600 seconds */
    ch->pcdata->in_progress->expire = expire;

    send_to_desc ("\n\r"
        "Enter text. Type {W~{x or {WEND{x on an empty line to end note.\n\r"
        "=======================================================\n\r", d);
    d->connected = CON_NOTE_TEXT;
}

DEFINE_NANNY_FUN (handle_con_note_text) {
    CHAR_T *ch = d->character;
    char buf[MAX_STRING_LENGTH];
    char letter[4*MAX_STRING_LENGTH];

    if (!ch->pcdata->in_progress) {
        d->connected = CON_PLAYING;
        bug ("nanny: In CON_NOTE_TEXT, but no note in progress",0);
        return;
    }

    /* First, check for EndOfNote marker */
    strcpy (buf, argument);
    if (!str_cmp (buf, "~") || !str_cmp (buf, "END")) {
        write_to_buffer (d, "\n\r\n\r",0);
        printf_to_desc (d, "%s", note_finish_prompt);
        write_to_buffer (d, "\n\r", 0);
        d->connected = CON_NOTE_FINISH;
        return;
    }

    str_smash_tilde (buf); /* smash it now */

    /* Check for too long lines. Do not allow lines longer than 80 chars */

    /* Hey, why #define MAX_LINE_LENGTH, then put a hardcoded value in here? ;-)
     * -- JR 09/24/00
     */
    if (strlen (buf) > MAX_LINE_LENGTH) {
        printf_to_desc (d, "Too long line rejected. Do NOT go over %d characters!\n\r", MAX_LINE_LENGTH);
        return;
    }

    /* Not end of note. Copy current text into temp buffer, add new line, and copy back */

    /* How would the system react to strcpy( , NULL) ? */
    if (ch->pcdata->in_progress->text) {
        strcpy (letter, ch->pcdata->in_progress->text);
        str_free (&(ch->pcdata->in_progress->text));
    }
    else
        strcpy (letter, "");

    /* Check for overflow */
    if ((strlen(letter) + strlen (buf)) > MAX_NOTE_TEXT) {
        write_to_buffer (d, "Note too long!\n\r", 0);
        note_free (ch->pcdata->in_progress);
        ch->pcdata->in_progress = NULL;
        d->connected = CON_PLAYING;
        return;
    }

    /* Add new line to the buffer */
    strcat (letter, buf);
    strcat (letter, "\r\n");

    /* allocate dynamically */
    ch->pcdata->in_progress->text = str_dup (letter);
}

DEFINE_NANNY_FUN (handle_con_note_finish) {
    CHAR_T *ch = d->character;

    if (!ch->pcdata->in_progress) {
        d->connected = CON_PLAYING;
        bug ("nanny: In CON_NOTE_FINISH, but no note in progress",0);
        return;
    }

    switch (tolower (argument[0])) {
        case 'c': /* keep writing */
            write_to_buffer (d,"Continuing note...\n\r",0);
            d->connected = CON_NOTE_TEXT;
            break;

        case 'v': /* view note so far */
            if (ch->pcdata->in_progress->text) {
                send_to_desc ("{gText of your note so far:{x\n\r",d);
                write_to_buffer (d, ch->pcdata->in_progress->text, 0);
            }
            else
                write_to_buffer (d,"You haven't written a thing!\n\r\n\r",0);
            printf_to_desc (d, "%s", note_finish_prompt);
            write_to_buffer (d, "\n\r",0);
            break;

        case 'p': /* post note */
            note_finish (ch->pcdata->in_progress, ch->pcdata->board);
            write_to_buffer (d, "Note posted.\n\r",0);
            d->connected = CON_PLAYING;
            /* remove AFK status */
            ch->pcdata->in_progress = NULL;
            act ("{G$n finishes $s note.{x", ch, NULL, NULL, TO_NOTCHAR);
            break;

        case 'f':
            write_to_buffer (d, "Note cancelled!\n\r",0);
            note_free (ch->pcdata->in_progress);
            ch->pcdata->in_progress = NULL;
            d->connected = CON_PLAYING;
            /* remove afk status */
            break;

        default: /* invalid response */
            write_to_buffer (d, "Huh? Valid answers are:\n\r\n\r",0);
            printf_to_desc (d, "%s", note_finish_prompt);
            write_to_buffer (d, "\n\r",0);
    }
}
