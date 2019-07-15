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

#ifndef __ROM_BOARD_H
#define __ROM_BOARD_H

#include "merc.h"

/* Definitions and global variables. */
#define L_SUP (MAX_LEVEL - 1) /* if not already defined */
#define BOARD_NOACCESS -1
#define BOARD_NOTFOUND -1

extern const char *szFinishPrompt;
extern long last_note_stamp;

/* Function prototypes. */
void append_note (FILE *fp, NOTE_DATA *note);
void finish_note (BOARD_DATA *board, NOTE_DATA *note);
int board_number (const BOARD_DATA *board);
void unlink_note (BOARD_DATA *board, NOTE_DATA *note);
NOTE_DATA* find_note (CHAR_DATA *ch, BOARD_DATA *board, int num);
void save_board (BOARD_DATA *board);
void show_note_to_char (CHAR_DATA *ch, NOTE_DATA *note, int num);
void save_notes ();
void load_board (BOARD_DATA *board);
void load_boards ();
bool is_note_to (CHAR_DATA *ch, NOTE_DATA *note);
int unread_notes (CHAR_DATA *ch, BOARD_DATA *board);
void personal_message (const char *sender, const char *to, const char *subject,
    const int expire_days, const char *text);
void make_note (const char* board_name, const char *sender, const char *to,
    const char *subject, const int expire_days, const char *text);
bool next_board (CHAR_DATA *ch);

/* for nanny */
NANNY_FUN handle_con_note_to;
NANNY_FUN handle_con_note_subject;
NANNY_FUN handle_con_note_expire;
NANNY_FUN handle_con_note_text;
NANNY_FUN handle_con_note_finish;

/* DERP */

void do_nwrite (CHAR_DATA *ch, char *argument);
void do_nread_next (CHAR_DATA *ch, char *argument, time_t *last_note);
void do_nread_number (CHAR_DATA *ch, char *argument, time_t *last_note,
    int number);
void do_nread (CHAR_DATA *ch, char *argument);
void do_nremove (CHAR_DATA *ch, char *argument);
void do_nlist (CHAR_DATA *ch, char *argument);
void do_ncatchup (CHAR_DATA *ch, char *argument);

#endif
