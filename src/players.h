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

#ifndef __ROM_PLAYERS_H
#define __ROM_PLAYERS_H

#include "merc.h"

/* Function prototypes. */
bool player_has_clan (const CHAR_T *ch);
bool player_is_independent (const CHAR_T *ch);
bool player_in_same_clan (const CHAR_T *ch, const CHAR_T *victim);
void player_reset (CHAR_T *ch);
void player_reset_colour (CHAR_T *ch);
void player_set_title (CHAR_T *ch, char *title);
void player_advance_level (CHAR_T *ch, bool hide);
void player_gain_exp (CHAR_T *ch, int gain);
int player_get_exp_to_next_level (const CHAR_T *ch);
int player_get_exp_per_level (const CHAR_T *ch);
int player_get_exp_per_level_with_points (const CHAR_T *ch, int points);

#endif
