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

#ifndef __ROM_ACT_BOARD_H
#define __ROM_ACT_BOARD_H

#include "merc.h"

/* Sub-routines and filters. */
void do_nread_next (CHAR_DATA *ch, char *argument, time_t *last_note);
void do_nread_number (CHAR_DATA *ch, char *argument, time_t *last_note,
    int number);

/* Commands. */
DECLARE_DO_FUN (do_nwrite);
DECLARE_DO_FUN (do_nread);
DECLARE_DO_FUN (do_nremove);
DECLARE_DO_FUN (do_nlist);
DECLARE_DO_FUN (do_ncatchup);
DECLARE_DO_FUN (do_note);
DECLARE_DO_FUN (do_board);

#endif
