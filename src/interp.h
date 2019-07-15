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
*    ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
*    ROM has been brought to you by the ROM consortium                      *
*        Russ Taylor (rtaylor@hypercube.org)                                *
*        Gabrielle Taylor (gtaylor@hypercube.org)                           *
*        Brian Moore (zump@rom.org)                                         *
*    By using this code, you have agreed to follow the terms of the         *
*    ROM license, in the file Rom24/doc/rom.license                         *
****************************************************************************/

#ifndef __ROM_INTERP_H
#define __ROM_INTERP_H

#include "merc.h"

/* Structure for a command in the command lookup table. */
struct cmd_type {
    char *const name;
    DO_FUN *do_fun;
    sh_int position;
    sh_int level;
    sh_int log;
    sh_int show;
};

/* Globals. */
extern const struct cmd_type cmd_table[];
extern bool fLogAll;

/* Function prototpyes. */
void interpret (CHAR_DATA *ch, char *argument);
void do_function (CHAR_DATA *ch, DO_FUN *do_fun, char *argument);
bool check_social (CHAR_DATA *ch, char *command, char *argument);
bool is_number (char *arg);
int number_argument (char *argument, char *arg);
int mult_argument (char *argument, char *arg);
char *one_argument (const char *argument, char *arg_first);

#endif
