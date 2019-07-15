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

#ifndef __ROM_ACT_CONF_H
#define __ROM_ACT_CONF_H

#include "merc.h"

/* Sub-routines and filters. */
void do_colour_one (CHAR_DATA * ch, const COLOUR_SETTING_TYPE * setting,
    const COLOUR_TYPE * colour, bool use_default, char *buf);
void do_colour_codes (CHAR_DATA * ch, char *argument);

/* Commands. */
DECLARE_DO_FUN (do_scroll);
DECLARE_DO_FUN (do_colour);
DECLARE_DO_FUN (do_autolist);
DECLARE_DO_FUN (do_autoassist);
DECLARE_DO_FUN (do_autoexit);
DECLARE_DO_FUN (do_autogold);
DECLARE_DO_FUN (do_autoloot);
DECLARE_DO_FUN (do_autosac);
DECLARE_DO_FUN (do_autosplit);
DECLARE_DO_FUN (do_noloot);
DECLARE_DO_FUN (do_nofollow);
DECLARE_DO_FUN (do_telnetga);
DECLARE_DO_FUN (do_brief);
DECLARE_DO_FUN (do_compact);
DECLARE_DO_FUN (do_show_affects);
DECLARE_DO_FUN (do_combine);
DECLARE_DO_FUN (do_nosummon);
DECLARE_DO_FUN (do_autoall);
DECLARE_DO_FUN (do_prompt);
DECLARE_DO_FUN (do_alia);
DECLARE_DO_FUN (do_alias);
DECLARE_DO_FUN (do_unalias);

#endif
