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

/***************************************************************************
 *                                                                         *
 *  MOBprograms for ROM 2.4 v0.98g (C) M.Nylander 1996                     *
 *  Based on MERC 2.2 MOBprograms concept by N'Atas-ha.                    *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 *  This code may be copied and distributed as per the ROM license.        *
 *                                                                         *
 ***************************************************************************/

#ifndef __ROG_MOB_PROG_H
#define __ROG_MOB_PROG_H

#include "merc.h"

int keyword_lookup (const char **table, char *keyword);
int num_eval (int lval, int oper, int rval);
CHAR_DATA *get_random_char (CHAR_DATA * mob);
bool count_people_room_check (CHAR_DATA * mob, CHAR_DATA * vch, int iFlag);
int count_people_room (CHAR_DATA * mob, int iFlag);
int get_order (CHAR_DATA * ch);
bool has_item (CHAR_DATA * ch, sh_int vnum, sh_int item_type, bool fWear);
bool get_mob_vnum_room (CHAR_DATA * ch, sh_int vnum);
bool get_obj_vnum_room (CHAR_DATA * ch, sh_int vnum);
int cmd_eval (sh_int vnum, char *line, int check,
              CHAR_DATA * mob, CHAR_DATA * ch,
              const void *arg1, const void *arg2, CHAR_DATA * rch);
void expand_arg (char *buf,
                 const char *format,
                 CHAR_DATA * mob, CHAR_DATA * ch,
                 const void *arg1, const void *arg2, CHAR_DATA * rch);
void program_flow (sh_int pvnum,    /* For diagnostic purposes */
                   char *source,    /* the actual MOBprog code */
                   CHAR_DATA * mob, CHAR_DATA * ch, const void *arg1,
                   const void *arg2);
bool mp_act_trigger (char *argument, CHAR_DATA * mob, CHAR_DATA * ch,
                     const void *arg1, const void *arg2, int type);
bool mp_percent_trigger (CHAR_DATA * mob, CHAR_DATA * ch,
                         const void *arg1, const void *arg2, int type);
bool mp_bribe_trigger (CHAR_DATA * mob, CHAR_DATA * ch, int amount);
bool mp_exit_trigger (CHAR_DATA * ch, int dir);
bool mp_give_trigger (CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj);
bool mp_greet_trigger (CHAR_DATA * ch);
bool mp_hprct_trigger (CHAR_DATA * mob, CHAR_DATA * ch);

#endif
