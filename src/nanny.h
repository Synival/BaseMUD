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
*    ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
*    ROM has been brought to you by the ROM consortium                      *
*        Russ Taylor (rtaylor@hypercube.org)                                *
*        Gabrielle Taylor (gtaylor@hypercube.org)                           *
*        Brian Moore (zump@rom.org)                                         *
*    By using this code, you have agreed to follow the terms of the         *
*    ROM license, in the file Rom24/doc/rom.license                         *
****************************************************************************/

#ifndef __ROM_NANNY_H
#define __ROM_NANNY_H

#include "merc.h"

/* Function prototypes. */
void nanny (DESCRIPTOR_DATA * d, char *argument);

NANNY_FUN nanny_ansi;
NANNY_FUN nanny_get_player_name;
NANNY_FUN nanny_get_old_password;
NANNY_FUN nanny_break_connect;
NANNY_FUN nanny_break_connect_confirm;
NANNY_FUN nanny_confirm_new_name;
NANNY_FUN nanny_get_new_password;
NANNY_FUN nanny_confirm_new_password;
NANNY_FUN nanny_get_new_race;
NANNY_FUN nanny_get_new_sex;
NANNY_FUN nanny_get_new_class;
NANNY_FUN nanny_get_alignment;
NANNY_FUN nanny_default_choice;
NANNY_FUN nanny_pick_weapon;
NANNY_FUN nanny_gen_groups;
NANNY_FUN nanny_gen_groups_done;
NANNY_FUN nanny_read_imotd;
NANNY_FUN nanny_read_motd;

#endif
