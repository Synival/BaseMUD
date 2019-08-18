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

#ifndef __ROM_WIZ_L4_H
#define __ROM_WIZ_L4_H

#include "merc.h"

/* Sub-routines and filters. */
void do_restore_single (CHAR_DATA *ch, CHAR_DATA *vch);

/* Commands. */
DECLARE_DO_FUN (do_guild);
DECLARE_DO_FUN (do_sockets);
DECLARE_DO_FUN (do_flag);
DECLARE_DO_FUN (do_freeze);
DECLARE_DO_FUN (do_load);
DECLARE_DO_FUN (do_mload);
DECLARE_DO_FUN (do_oload);
DECLARE_DO_FUN (do_pecho);
DECLARE_DO_FUN (do_purge);
DECLARE_DO_FUN (do_restore);
DECLARE_DO_FUN (do_echo);
DECLARE_DO_FUN (do_vnum);
DECLARE_DO_FUN (do_slookup);
DECLARE_DO_FUN (do_mfind);
DECLARE_DO_FUN (do_ofind);
DECLARE_DO_FUN (do_zecho);

#endif
