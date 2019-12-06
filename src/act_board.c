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
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

/* NOTE
 * ----
 * This code is taken directly from 'board.c'. All credits and notes are
 * at the head of that file.
 *     -- Synival */

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "comm.h"
#include "recycle.h"
#include "utils.h"
#include "lookup.h"
#include "interp.h"
#include "act_info.h"
#include "chars.h"
#include "memory.h"
#include "globals.h"

#include "act_board.h"

void do_nread_next (CHAR_T *ch, char *argument, time_t *last_note) {
    NOTE_T *p;
    char buf[200];
    int count = 1;

    for (p = ch->pcdata->board->note_first; p ; p = p->next, count++) {
        if ((p->date_stamp > *last_note) && is_note_to(ch, p)) {
            show_note_to_char (ch, p, count);
            /* Advance if new note is newer than the currently newest for that char */
            *last_note = UMAX (*last_note, p->date_stamp);
            return;
        }
    }

    send_to_char ("No new notes in this board.\n\r", ch);
    if (next_board (ch))
        sprintf (buf, "Changed to next board, %s.\n\r", ch->pcdata->board->name);
    else
        sprintf (buf, "There are no more boards.\n\r");
    send_to_char (buf, ch);
}

void do_nread_number (CHAR_T *ch, char *argument, time_t *last_note,
    int number)
{
    int count = 0;
    NOTE_T *p;
    number = atoi(argument);

    for (p = ch->pcdata->board->note_first; p; p = p->next)
        if (++count == number)
            break;
    BAIL_IF (!p || !is_note_to(ch, p),
        "No such note.\n\r", ch);

    show_note_to_char (ch, p, count);
    *last_note = UMAX (*last_note, p->date_stamp);
}

/* Start writing a note */
DEFINE_DO_FUN (do_nwrite) {
    char *strtime;
    char buf[200];

    if (IS_NPC(ch)) /* NPC cannot post notes */
        return;

    BAIL_IF (char_get_trust(ch) < ch->pcdata->board->write_level,
        "You cannot post notes on this board.\n\r", ch);

    /* continue previous note, if any text was written*/
    if (ch->pcdata->in_progress && (!ch->pcdata->in_progress->text)) {
        send_to_char ("Note in progress cancelled because you did not manage to write any text \n\r"
                      "before losing link.\n\r\n\r", ch);
        note_free (ch->pcdata->in_progress);
        ch->pcdata->in_progress = NULL;
    }

    if (!ch->pcdata->in_progress) {
        ch->pcdata->in_progress = note_new ();
        ch->pcdata->in_progress->sender = str_dup (ch->name);

        /* convert to ascii. ctime returns a string which last character is \n, so remove that */
        strtime = ctime (&current_time);
        strtime[strlen(strtime)-1] = '\0';

        ch->pcdata->in_progress->date = str_dup (strtime);
    }

    act ("{G$n starts writing a note.{x", ch, NULL, NULL, TO_NOTCHAR);

    /* Begin writing the note ! */
    printf_to_char (ch, "You are now %s a new note on the {W%s{x board.\n\r"
                  "If you are using tintin, type #verbose to turn off alias expansion!\n\r\n\r",
                   ch->pcdata->in_progress->text ? "continuing" : "posting",
                   ch->pcdata->board->name);

    printf_to_char (ch, "{YFrom{x:    %s\n\r\n\r", ch->name);

    if (!ch->pcdata->in_progress->text) { /* Are we continuing an old note or not? */
        switch (ch->pcdata->board->force_type) {
        case DEF_NORMAL:
            sprintf (buf, "If you press Return, default recipient \"{W%s{x\" will be chosen.\n\r",
                      ch->pcdata->board->names);
            break;
        case DEF_INCLUDE:
            sprintf (buf, "The recipient list MUST include \"{W%s{x\". If not, it will be added automatically.\n\r",
                           ch->pcdata->board->names);
            break;

        case DEF_EXCLUDE:
            sprintf (buf, "The recipient of this note must NOT include: \"{W%s{x\".",
                           ch->pcdata->board->names);
            break;
        }

        send_to_char (buf, ch);
        send_to_char ("\n\r{YTo{x:      ", ch);

        ch->desc->connected = CON_NOTE_TO;
        /* nanny takes over from here */
    }
    else { /* we are continuing, print out all the fields and the note so far*/
        printf_to_char (ch, "{YTo{x:      %s\n\r"
                      "{YExpires{x: %s\n\r"
                      "{YSubject{x: %s\n\r",
                       ch->pcdata->in_progress->to_list,
                       ctime(&ch->pcdata->in_progress->expire),
                       ch->pcdata->in_progress->subject);
        send_to_char ("{GYour note so far:{x\n\r", ch);
        send_to_char (ch->pcdata->in_progress->text, ch);

        send_to_char ("\n\rEnter text. Type {W~{x or {WEND{x on an empty line to end note.\n\r"
                          "=======================================================\n\r", ch);

        ch->desc->connected = CON_NOTE_TEXT;
    }
}

/* Read next note in current group. If no more notes, go to next board */
DEFINE_DO_FUN (do_nread) {
    time_t *last_note = &ch->pcdata->last_note[board_number(ch->pcdata->board)];

    /* if "again", do nothing - we'll show the same note. */
    if (!str_cmp(argument, "again"))
        return;

    /* read a specific number */
    if (is_number (argument))
        do_nread_number (ch, argument, last_note, atoi(argument));
    /* just next one */
    else
        do_nread_next(ch, argument, last_note);
}

/* Remove a note */
DEFINE_DO_FUN (do_nremove) {
    NOTE_T *p;
    BAIL_IF (!is_number(argument),
        "Remove which note?\n\r", ch);

    p = find_note (ch, ch->pcdata->board, atoi(argument));
    BAIL_IF (!p,
        "No such note.\n\r", ch);
    BAIL_IF (str_cmp(ch->name, p->sender) && (char_get_trust(ch) < MAX_LEVEL),
        "You are not authorized to remove this note.\n\r", ch);

    unlink_note (ch->pcdata->board, p);
    note_free (p);
    send_to_char ("Note removed!\n\r", ch);
    save_board(ch->pcdata->board);
}


/* List all notes or if argument given, list N of the last notes */
/* Shows REAL note numbers! */
DEFINE_DO_FUN (do_nlist) {
    int count = 0, show = 0, num = 0, has_shown = 0;
    time_t last_note;
    NOTE_T *p;

    if (is_number(argument)) { /* first, count the number of notes */
        show = atoi(argument);
        for (p = ch->pcdata->board->note_first; p; p = p->next)
            if (is_note_to (ch, p))
                count++;
    }

    send_to_char ("{WNotes on this board:{x\n\r"
                  "{rNum> Author        Subject{x\n\r", ch);

    last_note = ch->pcdata->last_note[board_number (ch->pcdata->board)];
    for (p = ch->pcdata->board->note_first; p; p = p->next) {
        num++;
        if (is_note_to(ch, p)) {
            has_shown++; /* note that we want to see X VISIBLE note, not just last X */
            if (!show || ((count-show) < has_shown)) {
                printf_to_char (ch, "{W%3d{x>{B%c{x{Y%-13s{x{y%s{x\n\r",
                    num, last_note < p->date_stamp ? '*' : ' ',
                    p->sender, p->subject);
            }
        }
    }
}

/* catch up with some notes */
DEFINE_DO_FUN (do_ncatchup) {
    NOTE_T *p;

    /* Find last note */
    for (p = ch->pcdata->board->note_first; p && p->next; p = p->next)
        ; /* empty */
    BAIL_IF (!p,
        "Alas, there are no notes in that board.\n\r", ch);

    ch->pcdata->last_note[board_number(ch->pcdata->board)] = p->date_stamp;
    send_to_char ("All mesages skipped.\n\r", ch);
}

/* Show all accessible boards with their numbers of unread messages OR
   change board. New board name can be given as a number or as a name (e.g.
   board personal or board 4 */
DEFINE_DO_FUN (do_board) {
    int i, count, number;

    if (IS_NPC(ch))
        return;

    if (!argument[0]) { /* show boards */
        int unread;

        count = 1;
        send_to_char ("{RNum          Name Unread Description{x\n\r"
                      "{R==== ============ ====== ============================={x\n\r", ch);
        for (i = 0; i < BOARD_MAX; i++) {
            unread = unread_notes (ch, &board_table[i]); /* how many unread notes? */
            if (unread != BOARD_NOACCESS) {
                printf_to_char (ch, "({W%2d{x) {g%12s{x [%s%4d{x] {y%s{x\n\r",
                    count, board_table[i].name, unread ? "{G" : "{g",
                    unread, board_table[i].long_name);
                count++;
            }
        }

        printf_to_char (ch, "\n\rYou current board is {W%s{x.\n\r", ch->pcdata->board->name);

        /* Inform of rights */
        if (ch->pcdata->board->read_level > char_get_trust(ch))
            send_to_char ("You cannot read nor write notes on this board.\n\r", ch);
        else if (ch->pcdata->board->write_level > char_get_trust(ch))
            send_to_char ("You can only read notes from this board.\n\r", ch);
        else
            send_to_char ("You can both read and write on this board.\n\r", ch);
        return;
    }

    BAIL_IF (ch->pcdata->in_progress,
        "Please finish your interrupted note first.\n\r", ch);

    /* Change board based on its number */
    if (is_number(argument)) {
        count = 0;
        number = atoi(argument);
        for (i = 0; i < BOARD_MAX; i++)
            if (unread_notes(ch, &board_table[i]) != BOARD_NOACCESS)
                if (++count == number)
                    break;

        if (count == number) { /* found the board.. change to it */
            ch->pcdata->board = &board_table[i];
            printf_to_char (ch, "Current board changed to {W%s{x. %s.\n\r",
                board_table[i].name,
                (char_get_trust(ch) < board_table[i].write_level)
                ? "You can only read here"
                : "You can both read and write here");
        }
        else
            send_to_char ("No such board.\n\r", ch);
        return;
    }

    /* Non-number given, find board with that name */
    for (i = 0; i < BOARD_MAX; i++)
        if (!str_cmp (board_table[i].name, argument))
            break;
    BAIL_IF (i == BOARD_MAX,
        "No such board.\n\r", ch);

    /* Does ch have access to this board? */
    BAIL_IF (unread_notes (ch, &board_table[i]) == BOARD_NOACCESS,
        "No such board.\n\r", ch);

    ch->pcdata->board = &board_table[i];
    printf_to_char (ch, "Current board changed to {W%s{x. %s.\n\r",
        board_table[i].name,
        (char_get_trust(ch) < board_table[i].write_level)
            ? "You can only read here"
            : "You can both read and write here");
}

/* Dispatch function for backwards compatibility */
DEFINE_DO_FUN (do_note) {
    char arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
        return;

    argument = one_argument (argument, arg);
    if ((!arg[0]) || (!str_cmp(arg, "read"))) /* 'note' or 'note read X' */
        do_nread (ch, argument);
    else if (!str_cmp (arg, "list"))
        do_nlist (ch, argument);
    else if (!str_cmp (arg, "write"))
        do_nwrite (ch, argument);
    else if (!str_cmp (arg, "remove"))
        do_nremove (ch, argument);
    else if (!str_cmp (arg, "purge"))
        send_to_char ("Obsolete.\n\r", ch);
    else if (!str_cmp (arg, "archive"))
        send_to_char ("Obsolete.\n\r", ch);
    else if (!str_cmp (arg, "catchup"))
        do_ncatchup (ch, argument);
    else
        do_help (ch, "note");
}
