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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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
****************************************************************************/

#ifndef __ROM_NANNY_H
#define __ROM_NANNY_H

#include "merc.h"

/* Function prototypes. */
bool new_player_name_is_valid (char *name);
void nanny (DESCRIPTOR_T *d, char *argument);

DECLARE_NANNY_FUN (nanny_ansi);
DECLARE_NANNY_FUN (nanny_get_player_name);
DECLARE_NANNY_FUN (nanny_get_old_password);
DECLARE_NANNY_FUN (nanny_break_connect);
DECLARE_NANNY_FUN (nanny_break_connect_confirm);
DECLARE_NANNY_FUN (nanny_confirm_new_name);
DECLARE_NANNY_FUN (nanny_get_new_password);
DECLARE_NANNY_FUN (nanny_confirm_new_password);
DECLARE_NANNY_FUN (nanny_get_new_race);
DECLARE_NANNY_FUN (nanny_get_new_sex);
DECLARE_NANNY_FUN (nanny_get_new_class);
DECLARE_NANNY_FUN (nanny_get_alignment);
DECLARE_NANNY_FUN (nanny_default_choice);
DECLARE_NANNY_FUN (nanny_pick_weapon);
DECLARE_NANNY_FUN (nanny_gen_groups);
DECLARE_NANNY_FUN (nanny_gen_groups_done);
DECLARE_NANNY_FUN (nanny_read_imotd);
DECLARE_NANNY_FUN (nanny_read_motd);

/* helper functions. */
bool nanny_parse_gen_groups (CHAR_T *ch, char *argument);

#endif
